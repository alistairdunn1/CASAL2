/**
 * @file TagLossEmpirical.cpp
 * @author  A. Dunn
 * @version 1.0
 * @date 20/05/2023
 * @section LICENSE
 *
 */

// Headers
#include "TagLossEmpirical.h"

#include <numeric>

#include "Categories/Categories.h"
#include "Selectivities/Manager.h"
#include "Selectivities/Selectivity.h"
#include "TimeSteps/Manager.h"

// Namespaces
namespace niwa {
namespace processes {
namespace age {

/**
 * Default Constructor
 *
 * This process differs from TagLoss in that every category gets the same tag loss rate in each year
 */
TagLossEmpirical::TagLossEmpirical(shared_ptr<Model> model) : Process(model), partition_(model) {
  LOG_TRACE();
  process_type_ = ProcessType::kMortality;
  // Why this was changed from type transition to mortality. CASAL includes this in the 'mortality block' so when you are
  // deriving observations partway through a time-step Casal incorporates F and TagLoss. Casal2 wasn't including tag-loss
  // into mid time-step interpolations. This fix will have annoying ramifications because a mortality block is defined
  // as continuous processes. so if you have a time-step with the following processes: F, tag-release, tag-loss. Casal2
  // may complain. CASAL reference see population_section.cpp line: 1924-2006

  partition_structure_ = PartitionType::kAge;

  // clang-format off
  parameters_.Bind<string>(PARAM_CATEGORIES, &category_labels_, "The list of categories to apply")
    ->flag_is_category();
  parameters_.Bind<Double>(PARAM_TAG_LOSS_RATE, &tag_loss_input_, "The tag loss rates"); 
  parameters_.Bind<Double>(PARAM_TIME_STEP_PROPORTIONS, &ratios_, "The time step proportions for tag loss"); 
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_names_, "The selectivities");
  parameters_.Bind<unsigned>(PARAM_YEAR, &year_, "The year the first tagging release process was executed");
  parameters_.Bind<unsigned>(PARAM_YEARS_AT_LIBERTY, &years_at_liberty_, "Years at liberty to apply the annual tag loss rates of tags", "")->set_lower_bound(0, true);
  // clang-format on

  RegisterAsAddressable(PARAM_TAG_LOSS_RATE, &tag_loss_input_);
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
void TagLossEmpirical::DoValidate() {
  LOG_FINEST() << "the number of years = " << years_at_liberty_.size() << ", the number of proportions = " << tag_loss_input_.size();

  parameters_.ValidateVector(PARAM_TAG_LOSS_RATE)
      ->GreaterThanOrEqualTo(0.0)
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS_AT_LIBERTY)
      ->SameNumberOfElementsAs(PARAM_YEARS_AT_LIBERTY);
  parameters_.ValidateVector(PARAM_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES);
  parameters_.ValidateVector(PARAM_TIME_STEP_PROPORTIONS)->SumToOne();
  parameters_.Validate(PARAM_YEAR)->IsModelYear();
  parameters_.ValidateVector(PARAM_YEARS_AT_LIBERTY)->GreaterThanOrEqualTo(0u)->LessThanOrEqualTo(model_->final_year() - model_->start_year() + 1u);
}

/**
 * Build any runtime relationships
 * - Build the partition accessor
 * - Build the list of selectivities
 * - Build the ratios for the number of time steps
 */
void TagLossEmpirical::DoBuild() {
  partition_.Init(category_labels_);

  for (string label : selectivity_names_) {
    Selectivity* selectivity = model_->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_SELECTIVITIES) << ": Selectivity label " << label << " was not found.";

    selectivities_.push_back(selectivity);
  }

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
    LOG_FATAL_P(PARAM_TIME_STEP_PROPORTIONS) << "are required parameters for this process and must be specified";
  } else {
    if (ratios_.size() != active_time_steps.size())
      LOG_FATAL_P(PARAM_TIME_STEP_PROPORTIONS) << " length (" << ratios_.size() << ") does not match the number of time steps this process has been assigned to ("
                                               << active_time_steps.size() << ")";

    for (Double value : ratios_) {
      if (value < 0.0 || value > 1.0)
        LOG_ERROR_P(PARAM_TIME_STEP_PROPORTIONS) << " Time step proportions (" << value << ") must be between 0.0 and 1.0 inclusive";
    }

    for (unsigned i = 0; i < ratios_.size(); ++i) time_step_ratios_[active_time_steps[i]] = ratios_[i];
  }
}

/**
 * Execute the process
 */
void TagLossEmpirical::DoExecute() {
  // To reduce computation only execute in relevant years.
  if (model_->current_year() >= year_) {
    LOG_FINEST() << "year: " << model_->current_year();

    unsigned time = model_->current_year() - year_;
    unsigned i    = 0;
    // apply the correct rate for the specific time at liberty
    for (unsigned year = 0; year < years_at_liberty_.size(); ++year) {
      if (time == years_at_liberty_[year]) {
        // get the ratio to apply first
        unsigned time_step = model_->managers()->time_step()->current_time_step();
        LOG_FINEST() << "Ratios.size() " << time_step_ratios_.size() << " : time_step: " << time_step << "; ratio: " << time_step_ratios_[time_step];
        Double ratio = time_step_ratios_[time_step];

        // apply over each category and age
        for (auto category : partition_) {
          unsigned j = 0;
          LOG_FINEST() << "category " << category->name_ << "; min_age: " << category->min_age_ << "; ratio: " << ratio;
          for (Double& data : category->data_) {
            Double amount = data * (1.0 - exp(-selectivities_[i]->GetAgeResult(category->min_age_ + j, category->age_length_) * tag_loss_input_[year] * ratio));
            LOG_FINEST() << "Category " << category->name_ << " numbers at age: " << category->min_age_ + j << " = " << data << " removing " << amount;
            data -= amount;
            ++j;
          }
          ++i;
        }
      }
    }
  }
}

/**
 * Reset the tag loss process
 */
void TagLossEmpirical::DoReset() {}

} /* namespace age */
} /* namespace processes */
} /* namespace niwa */
