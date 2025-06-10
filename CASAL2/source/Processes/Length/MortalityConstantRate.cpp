/**
 * @file MortalityConstantRate.cpp
 * @author  C.Marsh
 * @version 1.0
 * @date 1/8/2017
 * @section LICENSE
 * @description
 * This class applies an exponential mortality rate to all partition members defined by the user.
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 *
 */

// Headers
#include "MortalityConstantRate.h"

#include <numeric>

#include "../../Categories/Categories.h"
#include "../../TimeSteps/Manager.h"
#include "Selectivities/Manager.h"
#include "Utilities/Map.h"

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

  parameters_.Bind<string>(PARAM_CATEGORIES, &category_labels_, "The list of categories labels")->flag_is_category();
  parameters_.Bind<Double>(PARAM_M, &m_input_, "The mortality rates", "")->set_lower_bound(0.0);
  parameters_.Bind<Double>(PARAM_TIME_STEP_PROPORTIONS, &ratios_, "The time step proportions for the mortality rates", "", false)->set_range(0.0, 1.0);
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_names_, "The M-by-length bin ogives to apply to each category for the mortality")
      ->set_alias_labels({PARAM_RELATIVE_M_BY_LENGTH});

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
  parameters_.ValidateVector(PARAM_M)->GreaterThanOrEqualTo(0.0)->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES);
  parameters_.ValidateVector(PARAM_TIME_STEP_PROPORTIONS)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0)->SumToOne();
  parameters_.ValidateVector(PARAM_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES);

  m_ = utilities::OrderedMap<string, Double>::create(category_labels_, m_input_);
}

/**
 * Build any runtime relationships
 *
 * - Build the partition accessor
 * - Build the list of selectivities
 * - Build the ratios for the number of time steps
 */
void MortalityConstantRate::DoBuild() {
  partition_.Init(category_labels_);

  /**
   * Organise our time step ratios. Each time step can
   * apply a different ratio of M so here we want to verify
   * we have enough and re-scale them to 1.0
   */
  vector<TimeStep*> time_steps = model_->managers()->time_step()->ordered_time_steps();
  LOG_FINEST() << "time_steps.size(): " << time_steps.size();
  vector<unsigned> active_time_steps;
  for (unsigned i = 0; i < time_steps.size(); ++i) {
    if (time_steps[i]->HasProcess(label_))
      active_time_steps.push_back(i);
  }

  if (ratios_.size() == 0) {
    for (unsigned i : active_time_steps) time_step_ratios_[i] = 1.0;
  } else {
    if (ratios_.size() != active_time_steps.size())
      LOG_ERROR_P(PARAM_TIME_STEP_PROPORTIONS) << " length (" << ratios_.size() << ") does not match the number of time steps this process has been assigned to ("
                                               << active_time_steps.size() << ")";
    for (unsigned i = 0; i < ratios_.size(); ++i) time_step_ratios_[active_time_steps[i]] = ratios_[i];
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

  // get the ratio to apply first
  unsigned time_step = model_->managers()->time_step()->current_time_step();

  LOG_FINEST() << "Ratios.size() " << time_step_ratios_.size() << " : time_step: " << time_step << "; ratio: " << time_step_ratios_[time_step];
  Double ratio = time_step_ratios_[time_step];

  // StoreForReport("year", model_->current_year());

  unsigned i = 0;
  Double   amount;
  Double   total_amount = 0.0;
  for (auto category : partition_) {
    Double   m = m_[category->name_];
    unsigned j = 0;
    LOG_FINEST() << "category " << category->name_ << "; ratio: " << ratio;
    // StoreForReport(category->name_ + " ratio", ratio);
    for (Double& data : category->data_) {
      amount = data * (1 - exp(-selectivities_[i]->GetLengthResult(j) * (m * ratio)));
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
