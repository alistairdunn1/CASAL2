/**
 * @file ProcessRemovalsByLength.cpp
 * @author  C Marsh
 * @version 1.0
 * @date 2022
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 */

// Headers
#include "ProcessRemovalsByLength.h"

#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <iterator>

#include "Categories/Categories.h"
#include "GrowthIncrements/GrowthIncrement.h"
#include "Model/Model.h"
#include "Partition/Accessors/All.h"
#include "Partition/Accessors/Cached/CombinedCategories.h"
#include "TimeSteps/Manager.h"
#include "Utilities/Map.h"
#include "Utilities/Math.h"
#include "Utilities/To.h"
#include "Utilities/Vector.h"

// Namespaces
namespace niwa::observations::length {

/**
 * Default constructor
 */
ProcessRemovalsByLength::ProcessRemovalsByLength(shared_ptr<Model> model) : Observation(model) {
  obs_table_ = parameters_.BindTable(PARAM_OBS, "Table of observed values");
  obs_table_->set_requires_columns(false);
  error_values_table_ = parameters_.BindTable(PARAM_ERROR_VALUES, "The table of error values of the observed values (note that the units depend on the likelihood)");
  error_values_table_->set_requires_columns(false);

  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "The time step to execute in");
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years for which there are observations");
  parameters_.Bind<Double>(PARAM_PROCESS_ERRORS, &process_error_values_, "The process error")->set_is_optional(true);
  parameters_.Bind<string>(PARAM_METHOD_OF_REMOVAL, &method_, "The label of observed method of removals")->set_default_value("");
  parameters_.Bind<double>(PARAM_LENGTH_BINS, &length_bins_, "The length bins")->set_is_optional(true);
  parameters_.Bind<bool>(PARAM_PLUS_GROUP, &length_plus_, "Is the last length bin a plus group?")->set_is_optional(true);
  parameters_.Bind<string>(PARAM_MORTALITY_PROCESS, &process_label_, "The label of the mortality instantaneous process for the observation");
  parameters_.Bind<bool>(PARAM_SIMULATED_DATA_SUM_TO_ONE, &simulated_data_sum_to_one_, "Whether simulated data is discrete or scaled by totals to be proportions for each year")
      ->set_default_value(true);
  parameters_.Bind<bool>(PARAM_SUM_TO_ONE, &sum_to_one_, "Scale year (row) observed values by the total, so they sum = 1")->set_default_value(false);

  mean_proportion_method_ = false;

  RegisterAsAddressable(PARAM_PROCESS_ERRORS, &process_error_values_);

  allowed_likelihood_types_ = {PARAM_LOGNORMAL, PARAM_MULTINOMIAL, PARAM_DIRICHLET, PARAM_DIRICHLET_MULTINOMIAL, PARAM_LOGISTIC_NORMAL};
}

/**
 * Validate configuration file parameters
 */
void ProcessRemovalsByLength::DoValidate() {
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
  parameters_.Validate(PARAM_PLUS_GROUP)->DefaultValue(model_->length_plus());
  parameters_.ValidateVector(PARAM_LENGTH_BINS)->IsLengthBin()->IsInIncreasingOrder()->DefaultToAllModelLengthBins();
  parameters_.ValidateVector(PARAM_PROCESS_ERRORS)
      ->GreaterThanOrEqualTo(0.0)
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->DefaultValue(0.0, years_.size());

  number_bins_                   = length_plus_ ? length_bins_.size() : length_bins_.size() - 1;
  unsigned expected_column_count = number_bins_ * category_labels_.size() + 1;

  parameters_.ValidateTable(PARAM_OBS)
      ->Rows(years_.size(), "Number of rows in the observation table must match the number of years provided")
      ->Columns(expected_column_count, "Expected year, observation values, and error value columns in the observation table")
      ->ColumnIsYear(0, "First column of the observation table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All columns except the first must be a double value (data + error value) for the observation")
      ->GreaterThanOrEqualToForRange(1u, expected_column_count - 1, 0.0);

  parameters_.ValidateTable(PARAM_ERROR_VALUES)
      ->Rows(years_.size(), "Number of rows in the error values table must match the number of years provided")
      ->ExpandColumnsTo(category_labels_.size(), 1u)
      ->Columns(category_labels_.size() + 1, "Expected year and error value columns in the error values table")
      ->ColumnIsYear(0, "First column of the error values table must be a model year")
      ->DoubleDataRange(1, category_labels_.size(), "All columns except the first must be a double value (error values) for the observation")
      ->GreaterThanForRange(1, category_labels_.size(), 0.0);

  // Do some checks if we're not using all of the model length bins
  using_model_length_bins = length_bins_.size() == model_->length_bins().size();
  if (!using_model_length_bins)
    map_local_length_bins_to_global_length_bins_ = model_->get_map_for_bespoke_length_bins_to_global_length_bins(length_bins_, length_plus_);

  if (length_plus_ & !model_->length_plus())
    LOG_ERROR_P(PARAM_LENGTH_PLUS)
        << "you have specified a plus group on this observation, but the global length bins don't have a plus group. This is an inconsistency that must be fixed. Try changing the model plus group to false or this plus group to true";

  process_errors_by_year_ = utilities::Map::create(years_, process_error_values_);

  proportions_  = obs_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, expected_column_count - 1);
  error_values_ = error_values_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, category_labels_.size());

  if (sum_to_one_) {
    for (auto& year_pair : proportions_) {
      niwa::utilities::map::scale_to_one<string>(year_pair.second);
    }
  }
}

/**
 * Build any runtime relationships we may have and ensure
 * the labels for other objects are valid.
 */
void ProcessRemovalsByLength::DoBuild() {
  partition_ = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model_, category_labels_));
  // flag age-length to Build age-length matrix

  auto time_step = model_->managers()->time_step()->GetTimeStep(time_step_label_);
  if (!time_step) {
    LOG_FATAL_P(PARAM_TIME_STEP) << "Time step label " << time_step_label_ << " was not found.";
  } else {
    auto process             = time_step->SubscribeToProcess(this, years_, process_label_);
    mortality_instantaneous_ = dynamic_cast<length::MortalityInstantaneous*>(process);
  }

  if (mortality_instantaneous_ == nullptr)
    LOG_FATAL() << "Observation " << label_ << " can be used with Process type " << PARAM_MORTALITY_INSTANTANEOUS << " only. Process " << process_label_
                << " was not found or has retained catch characteristics specified.";

  // Need to split the categories if any are combined for checking
  vector<string> temp_split_category_labels, split_category_labels;

  for (const string& category_label : category_labels_) {
    boost::split(temp_split_category_labels, category_label, boost::is_any_of("+"));
    for (const string& split_category_label : temp_split_category_labels) {
      split_category_labels.push_back(split_category_label);
    }
  }

  // Need to make this a vector so its compatible with the function couldn't be bothered templating sorry
  vector<string> methods;
  methods.push_back(method_);

  // Do some checks so that the observation and process are compatible
  if (!mortality_instantaneous_->check_methods_for_removal_obs(methods))
    LOG_FATAL_P(PARAM_METHOD_OF_REMOVAL) << "could not find all these methods in the instantaneous_mortality process labeled " << process_label_
                                         << ". Check that the methods are compatible with this process";
  if (!mortality_instantaneous_->check_categories_in_methods_for_removal_obs(methods, split_category_labels))
    LOG_FATAL_P(PARAM_CATEGORIES) << "could not find all these categories in the instantaneous_mortality process labeled " << process_label_
                                  << ". Check that the categories are compatible with this process";
  if (!mortality_instantaneous_->check_years_in_methods_for_removal_obs(years_, methods))
    LOG_FATAL_P(PARAM_YEARS) << "could not find catches in all years in the instantaneous_mortality process labeled " << process_label_
                             << ". Check that the years are compatible with this process";

  fishery_ndx_to_get_catch_at_info_ = mortality_instantaneous_->get_fishery_ndx_for_catch_at(methods);
  vector<unsigned> category_ndxs    = mortality_instantaneous_->get_category_ndx_for_catch_at(split_category_labels);
  for (unsigned category_ndx = 0; category_ndx < split_category_labels.size(); ++category_ndx)
    category_lookup_for_ndx_to_get_catch_at_info_[split_category_labels[category_ndx]] = category_ndxs[category_ndx];
  year_ndx_to_get_catch_at_info_ = mortality_instantaneous_->get_year_ndx_for_catch_at(years_);

  expected_values_.resize(number_bins_, 0.0);
  final_denominator_.resize(years_.size(), 0.0);
}

void ProcessRemovalsByLength::DoReset() {
  // reset some containers
  fill(final_denominator_.begin(), final_denominator_.end(), 0.0);
}
/**
 * This method is called at the start of the targeted
 * time step for this observation.
 *
 * At this point we need to build our cache for the partition
 * structure to use with any interpolation
 */
void ProcessRemovalsByLength::PreExecute() {
  LOG_FINEST() << "Entering observation " << label_;
  if (partition_->Size() != proportions_[model_->current_year()].size())
    LOG_CODE_ERROR() << "partition_->Size() != proportions_[model->current_year()].size()";
}

/**
 * Execute the ProcessRemovalsByLength expected values calculations
 */
void ProcessRemovalsByLength::Execute() {
  LOG_TRACE();
  /**
   * Verify our cached partition and partition sizes are correct
   */
  //  auto categories = model_->categories();
  unsigned year           = model_->current_year();
  auto     it             = std::find(years_.begin(), years_.end(), year);
  unsigned year_ndx       = distance(years_.begin(), it);
  auto     partition_iter = partition_->Begin();  // vector<vector<partition::Category> >
  /**
   * Loop through the provided categories. Each provided category (combination) will have a list of observations
   * with it. We need to build a vector of proportions for each length using that combination and then
   * compare it to the observations.
   */
  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset, ++partition_iter) {
    LOG_FINEST() << "category: " << category_labels_[category_offset];
    std::fill(expected_values_.begin(), expected_values_.end(), 0.0);
    /**
     * Loop through the 2 combined categories building up the
     * expected proportions values.
     */

    auto category_iter = partition_iter->begin();
    for (; category_iter != partition_iter->end(); ++category_iter) {
      vector<Double>& Removals_at_length = mortality_instantaneous_->get_catch_at_by_year_fishery_category(
          year_ndx_to_get_catch_at_info_[year_ndx], fishery_ndx_to_get_catch_at_info_[0], category_lookup_for_ndx_to_get_catch_at_info_[(*category_iter)->name_]);

      LOG_FINE() << "----------";
      LOG_FINE() << "Category: " << (*category_iter)->name_;
      ;
      // clear these temporay vectors
      if (using_model_length_bins) {
        LOG_FINE() << "using model length bins";
        // Model length bins
        for (unsigned model_length_offset = 0; model_length_offset < model_->get_number_of_length_bins(); ++model_length_offset) {
          // now for each column (length bin) in age_length_matrix sum up all the rows (ages) for both cached and current matricies
          expected_values_[model_length_offset] += Removals_at_length[model_length_offset];
          final_denominator_[year_ndx] += Removals_at_length[model_length_offset];
        }
      } else {
        LOG_FINE() << "using bespoke length bins";
        // Bespoke model bins
        for (unsigned model_length_offset = 0; model_length_offset < model_->get_number_of_length_bins(); ++model_length_offset) {
          // if not sum = 1 then denominator over all length bins.
          if (!sum_to_one_)
            final_denominator_[year_ndx] += Removals_at_length[model_length_offset];
          if (map_local_length_bins_to_global_length_bins_[model_length_offset] >= 0) {
            // now for each column (length bin) in age_length_matrix sum up all the rows (ages) for both cached and current matricies
            expected_values_[map_local_length_bins_to_global_length_bins_[model_length_offset]] += Removals_at_length[model_length_offset];
            if (sum_to_one_)
              final_denominator_[year_ndx] += Removals_at_length[model_length_offset];
          }
        }
      }
    }

    if (expected_values_.size() != proportions_[model_->current_year()][category_labels_[category_offset]].size())
      LOG_CODE_ERROR() << "expected_values.size(" << expected_values_.size() << ") != proportions_[category_offset].size("
                       << proportions_[model_->current_year()][category_labels_[category_offset]].size() << ")";

    /**
     * save our comparisons so we can use them to generate the score from the likelihoods later
     */
    for (unsigned i = 0; i < expected_values_.size(); ++i) {
      LOG_FINEST() << "category_labels_[category_offset] " << category_labels_[category_offset] << " i " << i << " length_bins_[i] " << length_bins_[i] << " proportions "
                   << proportions_[model_->current_year()][category_labels_[category_offset]][i] << " expected_values_[i] " << expected_values_[i];
      SaveComparison(category_labels_[category_offset], 0, length_bins_[i], expected_values_[i], proportions_[model_->current_year()][category_labels_[category_offset]][i],
                     process_errors_by_year_[model_->current_year()], error_values_[model_->current_year()][category_labels_[category_offset]][0], 0.0, delta_, 0.0);
    }
  }
}

/**
 * This method is called at the end of a model iteration
 * to calculate the score for the observation.
 */
void ProcessRemovalsByLength::CalculateScore() {
  /**
   * Simulate or generate results
   * During simulation mode we'll simulate results for this observation
   */
  LOG_FINEST() << "Calculating neglogLikelihood for observation = " << label_;

  if (model_->run_mode() == RunMode::kSimulation) {
    if (model_->get_simulation_iterator() == 0) {
      for (auto& iter : comparisons_) {
        auto     it       = std::find(years_.begin(), years_.end(), iter.first);
        unsigned year_ndx = distance(years_.begin(), it);
        for (auto& comparison : iter.second) comparison.expected_ /= final_denominator_[year_ndx];
      }
    }
    likelihood_->SimulateObserved(comparisons_);
    if (simulated_data_sum_to_one_) {
      for (auto& iter : comparisons_) {
        double total = 0.0;
        for (auto& comparison : iter.second) total += comparison.observed_;
        for (auto& comparison : iter.second) comparison.observed_ /= total;
      }
    }
  } else {
    /**
     * Convert the expected_values in to a proportion
     */
    for (auto& iter : comparisons_) {
      auto     it       = std::find(years_.begin(), years_.end(), iter.first);
      unsigned year_ndx = distance(years_.begin(), it);
      for (auto& comparison : iter.second) comparison.expected_ /= final_denominator_[year_ndx];
    }
    likelihood_->GetScores(comparisons_);
    for (unsigned year : years_) {
      scores_[year] = likelihood_->GetInitialScore(comparisons_, year);
      LOG_FINEST() << "-- Observation neglogLikelihood calculation";
      LOG_FINEST() << "[" << year << "] Initial neglogLikelihood:" << scores_[year];
      for (obs::Comparison comparison : comparisons_[year]) {
        LOG_FINEST() << "[" << year << "] + neglogLikelihood: " << comparison.score_;
        scores_[year] += comparison.score_;
      }
    }
    LOG_FINEST() << "Finished calculating neglogLikelihood for = " << label_;
  }
}

}  // namespace niwa::observations::length
