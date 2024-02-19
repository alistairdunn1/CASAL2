/**
 * @file MortalityConstantRate.cpp
 * @author  C.Marsh
 * @version 1.0
 * @date 1/8/2017
 * @section LICENSE
 * @description
 * This class applies an exponential mortality rate to all partition members defined by the user.
 * Copyright NIWA Science �2017 - www.niwa.co.nz
 *
 *
 */

// Headers
#include <numeric>

#include "../../Categories/Categories.h"
#include "../../TimeSteps/Manager.h"
#include "MortalityConstantRate.h"
#include "Selectivities/Manager.h"

// Namespaces
namespace niwa {
namespace processes {
namespace length {

/**
 * Default constructor
 */
MortalityConstantRate::MortalityConstantRate(shared_ptr<Model> model) : Process(model), partition_(model) {
  LOG_TRACE();
  process_type_        = ProcessType::kMortality;
  partition_structure_ = PartitionType::kLength;

  parameters_.Bind<string>(PARAM_CATEGORIES, &category_labels_, "The list of categories labels", "");
  parameters_.Bind<Double>(PARAM_M, &m_input_, "The mortality rates", "")->set_lower_bound(0.0);
  parameters_.Bind<Double>(PARAM_TIME_STEP_PROPORTIONS, &proportions_, "The time step proportions for the mortality rates", "", false)->set_range(0.0, 1.0);
  parameters_.Bind<string>(PARAM_RELATIVE_M_BY_LENGTH, &selectivity_names_, "The M-by-length bin ogives to apply to each category for the mortality", "");

  RegisterAsAddressable(PARAM_M, &m_);
}

/**
 * Validate the Mortality Constant Rate process
 *
 * - Validate the required parameters
 * - Assign the label from the parameters
 * - Assign and validate remaining parameters
 * - Duplicate 'm' and 'selectivities' if only one value specified
 * - Check m is between 0.0 and 1.0
 * - Check the categories are real
 */
void MortalityConstantRate::DoValidate() {
  if (m_input_.size() == 1) {
    auto val_m = m_input_[0];
    m_input_.assign(category_labels_.size(), val_m);
  }

  if (selectivity_names_.size() == 1) {
    auto val_sel = selectivity_names_[0];
    selectivity_names_.assign(category_labels_.size(), val_sel);
  }

  if (m_input_.size() != category_labels_.size()) {
    LOG_ERROR_P(PARAM_M) << ": The number of Ms provided (" << m_input_.size() << ") does not match the number of categories provided (" << category_labels_.size() << ").";
  }

  if (selectivity_names_.size() != category_labels_.size()) {
    LOG_ERROR_P(PARAM_RELATIVE_M_BY_LENGTH) << ": The number of M-by-length ogives provided (" << selectivity_names_.size()
                                            << ") does not match the number of categories provided (" << category_labels_.size() << ").";
  }

  for (unsigned i = 0; i < m_input_.size(); ++i) m_[category_labels_[i]] = m_input_[i];

  // check proportions sum to one
  Double running_total = 0.0;
  for (Double value : proportions_) {  // ADOLC prevents std::accum
    if (value <= 0)
      LOG_INFO_P(PARAM_PROPORTIONS) << "is zero for one of the categories";
    running_total += value;
  }
  if (!utilities::math::IsOne(running_total)) {
    LOG_WARNING_P(PARAM_PROPORTIONS) << "the sum of the proportions is " << running_total << ", but should be 1.0. These have been rescaled to sum to 1.0";
    for (Double& proportion : proportions_) proportion /= running_total;
  }
}

/**
 * Build any runtime relationships
 *
 * - Build the partition accessor
 * - Build the list of selectivities
 * - Build the proportions for the number of time steps
 */
void MortalityConstantRate::DoBuild() {
  partition_.Init(category_labels_);

  /**
   * Organise our time step proportions. Each time step can
   * apply a different proportion of M so here we want to verify
   * we have enough and re-scale them to 1.0
   */
  vector<TimeStep*> time_steps = model_->managers()->time_step()->ordered_time_steps();
  LOG_FINEST() << "time_steps.size(): " << time_steps.size();
  vector<unsigned> active_time_steps;
  for (unsigned i = 0; i < time_steps.size(); ++i) {
    if (time_steps[i]->HasProcess(label_))
      active_time_steps.push_back(i);
  }

  if (proportions_.size() == 0) {
    for (unsigned i : active_time_steps) time_step_proportions_[i] = 1.0;
  } else {
    if (proportions_.size() != active_time_steps.size())
      LOG_ERROR_P(PARAM_TIME_STEP_PROPORTIONS) << " length (" << proportions_.size() << ") does not match the number of time steps this process has been assigned to ("
                                               << active_time_steps.size() << ")";
    for (unsigned i = 0; i < proportions_.size(); ++i) time_step_proportions_[active_time_steps[i]] = proportions_[i];
  }
  for (string label : selectivity_names_) {
    Selectivity* selectivity = model_->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_RELATIVE_M_BY_LENGTH) << ": M-by-length ogive label " << label << " was not found.";
    selectivities_.push_back(selectivity);
  }
}

/**
 * Execute the process
 */
void MortalityConstantRate::DoExecute() {
  LOG_FINEST() << "year: " << model_->current_year();

  // get the proportion to apply first
  unsigned time_step = model_->managers()->time_step()->current_time_step();

  LOG_FINEST() << "Ratios.size() " << time_step_proportions_.size() << " : time_step: " << time_step << "; proportion: " << time_step_proportions_[time_step];
  Double proportion = time_step_proportions_[time_step];

  // StoreForReport("year", model_->current_year());

  unsigned i = 0;
  Double   amount;
  Double   total_amount = 0.0;
  for (auto category : partition_) {
    Double   m = m_[category->name_];
    unsigned j = 0;
    LOG_FINEST() << "category " << category->name_ << "; proportion: " << proportion;
    // StoreForReport(category->name_ + " proportion", proportion);
    for (Double& data : category->data_) {
      amount = data * (1 - exp(-selectivities_[i]->GetLengthResult(j) * (m * proportion)));
      data -= amount;
      total_amount += amount;
      ++j;
    }
    ++i;
  }
}

/**
 * Reset the Mortality Process
 */
void MortalityConstantRate::DoReset() {
  mortality_rates_.clear();
}

} /* namespace length */
} /* namespace processes */
} /* namespace niwa */
