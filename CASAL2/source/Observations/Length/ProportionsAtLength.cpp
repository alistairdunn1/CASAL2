/**
 * @file ProportionsAtLength.cpp
 * @author  C Marsh
 * @version 1.0
 * @date 2022
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 */

// Headers
#include "ProportionsAtLength.h"

#include <algorithm>

#include "../../Partition/Accessors/Cached/CombinedCategories.h"
#include "Model/Model.h"
#include "Partition/Accessors/All.h"
#include "Selectivities/Manager.h"
#include "Utilities/Map.h"
#include "Utilities/Math.h"
#include "Utilities/To.h"

// Namespaces
namespace niwa {
namespace observations {
namespace length {

/**
 * Default constructor
 */
ProportionsAtLength::ProportionsAtLength(shared_ptr<Model> model) : Observation(model) {
  obs_table_ = parameters_.BindTable(PARAM_OBS, "The table of observed values");
  obs_table_->set_requires_columns(false);
  error_values_table_ = parameters_.BindTable(PARAM_ERROR_VALUES, "The table of error values of the observed values (note that the units depend on the likelihood)");
  error_values_table_->set_requires_columns(false);

  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "The label of the time step that the observation occurs in");
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years for which there are observations");
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_labels_, "The labels of the selectivities");
  parameters_.Bind<Double>(PARAM_PROCESS_ERRORS, &process_error_values_, "The process error")->set_is_optional(true);
  parameters_.Bind<double>(PARAM_LENGTH_BINS, &length_bins_, "The length bins")->set_is_optional(true);
  parameters_.Bind<bool>(PARAM_PLUS_GROUP, &length_plus_, "Is the last length bin a plus group? (defaults to @model value)")->set_is_optional(true);
  parameters_.Bind<bool>(PARAM_SIMULATED_DATA_SUM_TO_ONE, &simulated_data_sum_to_one_, "Whether simulated data is discrete or scaled by totals to be proportions for each year")
      ->set_default_value(true);
  parameters_.Bind<bool>(PARAM_SUM_TO_ONE, &sum_to_one_, "Scale year (row) observed values by the total, so they sum = 1")->set_default_value(true);

  allowed_likelihood_types_ = {PARAM_LOGNORMAL, PARAM_MULTINOMIAL, PARAM_DIRICHLET, PARAM_DIRICHLET_MULTINOMIAL, PARAM_LOGISTIC_NORMAL};
}

/**
 * Validate configuration file parameters
 */
void ProportionsAtLength::DoValidate() {
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
  parameters_.ValidateVector(PARAM_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES);
  parameters_.ValidateVector(PARAM_PROCESS_ERRORS)
      ->GreaterThanOrEqualTo(0.0)
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->DefaultValue(0.0, years_.size());
  parameters_.Validate(PARAM_PLUS_GROUP)->DefaultValue(model_->length_plus());
  parameters_.ValidateVector(PARAM_LENGTH_BINS)->IsLengthBin()->IsInIncreasingOrder()->DefaultToAllModelLengthBins();

  number_bins_                   = length_plus_ ? length_bins_.size() : length_bins_.size() - 1;
  unsigned expected_column_count = number_bins_ * category_labels_.size() + 1;  // +1 for the year column

  parameters_.ValidateTable(PARAM_OBS)
      ->Rows(years_.size(), "Number of rows in the observation table must match the number of years provided")
      ->Columns(expected_column_count, "Expected year, observation values, and error value columns in the observation table")
      ->ColumnIsYear(0, "First column of the observation table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All columns except the first must be a double value (data + error value) for the observation")
      ->GreaterThanOrEqualToForRange(1u, expected_column_count - 1, 0.0)
      ->LessThanOrEqualToForRange(1u, expected_column_count - 1, 1.0);

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
 * Build any runtime relationships and ensure that the labels for other objects are valid.
 */
void ProportionsAtLength::DoBuild() {
  partition_        = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model_, category_labels_));
  cached_partition_ = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model_, category_labels_));

  // Build Selectivity pointers
  for (string label : selectivity_labels_) {
    Selectivity* selectivity = model_->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_SELECTIVITIES) << ": Selectivity label " << label << " was not found.";
    selectivities_.push_back(selectivity);
  }

  LOG_FINE() << "number_bins_ = " << number_bins_ << " model length bins = " << model_->get_number_of_length_bins();
  expected_values_.resize(number_bins_, 0.0);
  numbers_at_length_.resize(number_bins_, 0.0);
  cached_numbers_at_length_.resize(number_bins_, 0.0);
  denominator_.resize(years_.size(), 0.0);
  cached_denominator_.resize(years_.size(), 0.0);
  final_denominator_.resize(years_.size(), 0.0);
}

/**
 * This method is called at the start of the targeted
 * time step for this observation.
 *
 * Build the cache for the partition
 * structure to use with any interpolation
 */
void ProportionsAtLength::PreExecute() {
  cached_partition_->BuildCache();
  if (cached_partition_->Size() != proportions_[model_->current_year()].size())
    LOG_CODE_ERROR() << "cached_partition_->Size() != proportions_[model->current_year()].size()";
  if (partition_->Size() != proportions_[model_->current_year()].size())
    LOG_CODE_ERROR() << "partition_->Size() != proportions_[model->current_year()].size()";
}

void ProportionsAtLength::DoReset() {
  // reset some containers
  fill(final_denominator_.begin(), final_denominator_.end(), 0.0);
  fill(denominator_.begin(), denominator_.end(), 0.0);
  fill(cached_denominator_.begin(), cached_denominator_.end(), 0.0);
}

/**
 * Execute the ProportionsAtLength expected values calculations
 */
void ProportionsAtLength::Execute() {
  LOG_TRACE();
  LOG_FINEST() << "Entering observation " << label_;

  auto     it       = std::find(years_.begin(), years_.end(), model_->current_year());
  unsigned year_ndx = distance(years_.begin(), it);
  LOG_FINE() << "Year = " << model_->current_year() << " year ndx = " << year_ndx;
  /**
   * Verify our cached partition and partition sizes are correct
   */
  auto partition_iter = partition_->Begin();  // vector<vector<partition::Category>>
  /**
   * Loop through the provided categories. Each provided category (combination) will have a list of observations
   * with it. We need to build a vector of proportions for each length using that combination and then
   * compare it to the observations.
   */
  unsigned selectivity_offset = 0;
  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset, ++partition_iter) {
    LOG_FINEST() << "category: " << category_labels_[category_offset];
    std::fill(expected_values_.begin(), expected_values_.end(), 0.0);
    Double start_value = 0.0;
    Double end_value   = 0.0;
    Double final_value = 0.0;

    std::set<string> selectivities_labels;
    /**
     * Loop through the 2 combined categories building up the
     * expected proportions values.
     */
    auto category_iter = partition_iter->begin();
    // clear these temporay vectors
    std::fill(cached_numbers_at_length_.begin(), cached_numbers_at_length_.end(), 0.0);
    std::fill(numbers_at_length_.begin(), numbers_at_length_.end(), 0.0);
    for (; category_iter != partition_iter->end(); ++category_iter, ++selectivity_offset) {
      LOG_FINE() << "this category = " << (*category_iter)->name_;
      LOG_FINEST() << "Selectivity for " << category_labels_[category_offset] << " selectivity " << selectivities_[selectivity_offset]->label();
      selectivities_labels.insert(selectivities_[selectivity_offset]->label());

      // Now convert numbers at age to numbers at length using the categories age-length transition matrix
      if (using_model_length_bins) {
        LOG_FINE() << "using model length bins";
        for (unsigned model_length_offset = 0; model_length_offset < model_->get_number_of_length_bins(); ++model_length_offset) {
          // now for each column (length bin) in age_length_matrix sum up all the rows (ages) for both cached and current matricies
          cached_numbers_at_length_[model_length_offset]
              += (*category_iter)->cached_data_[model_length_offset] * selectivities_[selectivity_offset]->GetLengthResult(model_length_offset);
          numbers_at_length_[model_length_offset] += (*category_iter)->data_[model_length_offset] * selectivities_[selectivity_offset]->GetLengthResult(model_length_offset);
          denominator_[year_ndx] += (*category_iter)->data_[model_length_offset] * selectivities_[selectivity_offset]->GetLengthResult(model_length_offset);
          cached_denominator_[year_ndx] += (*category_iter)->cached_data_[model_length_offset] * selectivities_[selectivity_offset]->GetLengthResult(model_length_offset);
        }
      } else {
        LOG_FINE() << "using bespoke length bins";
        for (unsigned model_length_offset = 0; model_length_offset < model_->get_number_of_length_bins(); ++model_length_offset) {
          if (!sum_to_one_) {
            // denominator is over entire population if not sum = 1
            denominator_[year_ndx] += (*category_iter)->data_[model_length_offset] * selectivities_[selectivity_offset]->GetLengthResult(model_length_offset);
            cached_denominator_[year_ndx] += (*category_iter)->cached_data_[model_length_offset] * selectivities_[selectivity_offset]->GetLengthResult(model_length_offset);
          }
          if (map_local_length_bins_to_global_length_bins_[model_length_offset] >= 0) {
            // now for each column (length bin) in age_length_matrix sum up all the rows (ages) for both cached and current matricies
            cached_numbers_at_length_[map_local_length_bins_to_global_length_bins_[model_length_offset]]
                += (*category_iter)->cached_data_[model_length_offset] * selectivities_[selectivity_offset]->GetLengthResult(model_length_offset);
            numbers_at_length_[map_local_length_bins_to_global_length_bins_[model_length_offset]]
                += (*category_iter)->data_[model_length_offset] * selectivities_[selectivity_offset]->GetLengthResult(model_length_offset);
            if (sum_to_one_) {
              // denominator just over observation range
              denominator_[year_ndx] += (*category_iter)->data_[model_length_offset] * selectivities_[selectivity_offset]->GetLengthResult(model_length_offset);
              cached_denominator_[year_ndx] += (*category_iter)->cached_data_[model_length_offset] * selectivities_[selectivity_offset]->GetLengthResult(model_length_offset);
            }
          }
        }
      }
    }
    LOG_FINE() << "year " << year_ndx << " denominator = " << denominator_[year_ndx];

    for (unsigned length_offset = 0; length_offset < number_bins_; ++length_offset) {
      start_value = cached_numbers_at_length_[length_offset];
      end_value   = numbers_at_length_[length_offset];
      if (mean_proportion_method_) {
        final_value = start_value + ((end_value - start_value) * proportion_of_time_);
      } else {
        final_value = (1 - proportion_of_time_) * start_value + proportion_of_time_ * end_value;
      }
      expected_values_[length_offset] += final_value;

      LOG_FINE() << "----------";
      LOG_FINE() << "start_value: " << start_value << "; end_value: " << end_value << "; final_value: " << final_value;
      LOG_FINE() << "expected_value becomes: " << expected_values_[length_offset];
    }
    if (expected_values_.size() != proportions_[model_->current_year()][category_labels_[category_offset]].size())
      LOG_CODE_ERROR() << "expected_values.size(" << expected_values_.size() << ") != proportions_[category_offset].size("
                       << proportions_[model_->current_year()][category_labels_[category_offset]].size() << ")";

    /**
     * save our comparisons so we can use them to generate the score from the likelihoods later
     */
    for (unsigned i = 0; i < expected_values_.size(); ++i) {
      SaveComparison(category_labels_[category_offset], selectivities_labels, 0, length_bins_[i], expected_values_[i],
                     proportions_[model_->current_year()][category_labels_[category_offset]][i], process_errors_by_year_[model_->current_year()],
                     error_values_[model_->current_year()][category_labels_[category_offset]][0], 0.0, delta_, 0.0);
    }
  }
  if (mean_proportion_method_)
    final_denominator_[year_ndx] = cached_denominator_[year_ndx] + ((denominator_[year_ndx] - cached_denominator_[year_ndx]) * proportion_of_time_);
  else
    final_denominator_[year_ndx] = (1 - proportion_of_time_) * cached_denominator_[year_ndx] + proportion_of_time_ * denominator_[year_ndx];
  LOG_FINE() << "denominator before " << cached_denominator_[year_ndx] << " after = " << denominator_[year_ndx] << " result = " << final_denominator_[year_ndx];
}

/**
 * This method is called at the end of a model iteration
 * to calculate the score for the observation.
 */
void ProportionsAtLength::CalculateScore() {
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
    // simulated values based on error_value so hard to deal with sum_to_one = F
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
      LOG_FINE() << "year = " << iter.first;
      auto     it       = std::find(years_.begin(), years_.end(), iter.first);
      unsigned year_ndx = distance(years_.begin(), it);
      LOG_FINE() << "year ndx " << year_ndx << " denominator = " << final_denominator_[year_ndx];
      for (auto& comparison : iter.second) comparison.expected_ /= final_denominator_[year_ndx];
    }
    likelihood_->GetScores(comparisons_);
    for (unsigned year : years_) {
      scores_[year] = likelihood_->GetInitialScore(comparisons_, year);
      LOG_FINEST() << "-- Observation neglogLikelihood calculation";
      LOG_FINEST() << "[" << year << "] Initial neglogLikelihood: " << scores_[year];
      for (obs::Comparison comparison : comparisons_[year]) {
        LOG_FINEST() << "[" << year << "] + neglogLikelihood: " << comparison.score_;
        scores_[year] += comparison.score_;
      }
    }

    LOG_FINEST() << "Finished calculating neglogLikelihood for = " << label_;
  }
}

}  // namespace length
} /* namespace observations */
} /* namespace niwa */
