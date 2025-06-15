/**
 * @file TagRecaptureByLengthForGrowthForGrowth.cpp
 * @author C.Marsh
 * @github https://github.com/Craig44
 * @date 2022
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "TagRecaptureByLengthForGrowth.h"

#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim_all.hpp>

#include "Categories/Categories.h"
#include "Model/Model.h"
#include "Partition/Accessors/All.h"
#include "Selectivities/Manager.h"
#include "TimeSteps/Manager.h"
#include "Utilities/Map.h"
#include "Utilities/Math.h"
#include "Utilities/To.h"
// namespaces
namespace niwa {
namespace observations {
namespace length {

/**
 * Default constructor
 */
TagRecaptureByLengthForGrowth::TagRecaptureByLengthForGrowth(shared_ptr<Model> model) : Observation(model) {
  recaptures_table_ = new parameters::Table(PARAM_RECAPTURED);
  parameters_.BindTable(PARAM_RECAPTURED, recaptures_table_, "The table of observed recaptured individuals in each age class", "", false);

  // clang-format off
  parameters_.Bind<double>(PARAM_LENGTH_BINS, &length_bins_, "The length bins")->set_is_optional(true);
  parameters_.Bind<bool>(PARAM_PLUS_GROUP, &length_plus_, "Is the last length bin a plus group?")->set_is_optional(true);
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years for which there are observations")->set_is_optional(true);
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_labels_, "The labels of the selectivities");
  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "The label of the time step that the observation occurs in");
  parameters_.Bind<Double>(PARAM_PROCESS_ERRORS, &process_error_values_, "The process error")->set_is_optional(true);
  parameters_.Bind<Double>(PARAM_TIME_STEP_PROPORTION, &time_step_proportion_, "The proportion through the mortality block of the time step when the observation is evaluated")
    ->set_default_value(0.5);
  // clang-format on

  mean_proportion_method_ = true;

  allowed_likelihood_types_.push_back(PARAM_MULTINOMIAL);
}

/**
 * Validate configuration file parameters
 */
void TagRecaptureByLengthForGrowth::DoValidate() {
  parameters_.ValidateVector(PARAM_LENGTH_BINS)->IsLengthBin()->IsInIncreasingOrder()->DefaultToAllModelLengthBins();
  parameters_.Validate(PARAM_PLUS_GROUP)->DefaultValue(model_->length_plus());
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
  parameters_.ValidateVector(PARAM_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES);
  parameters_.ValidateVector(PARAM_PROCESS_ERRORS)
      ->GreaterThanOrEqualTo(0.0)
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->DefaultValue(1.0, years_.size());
  parameters_.Validate(PARAM_TIME_STEP_PROPORTION)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);

  if (length_plus_ & !model_->length_plus())
    LOG_ERROR_P(PARAM_LENGTH_PLUS)
        << "you have specified a plus group on this observation, but the global length bins don't have a plus group. This is an inconsistency that must be fixed. Try changing the model plus group to false or this plus group to true";

  process_errors_by_year_ = utilities::Map::create(years_, process_error_values_);

  // Do some checks if we're not using all of the model length bins
  using_model_length_bins = length_bins_.size() == model_->length_bins().size();
  if (!using_model_length_bins)
    map_local_length_bins_to_global_length_bins_ = model_->get_map_for_bespoke_length_bins_to_global_length_bins(length_bins_, length_plus_);

  if (length_plus_ & !model_->length_plus())
    LOG_ERROR_P(PARAM_LENGTH_PLUS)
        << "you have specified a plus group on this observation, but the global length bins don't have a plus group. This is an inconsistency that must be fixed. Try changing the model plus group to false or this plus group to true";

  number_bins_ = length_plus_ ? length_bins_.size() : length_bins_.size() - 1;

  category_split_labels_ = model_->categories()->total_categories(category_labels_);

  /**
   * Validate the number of recaptures provided matches age spread * category_labels * years
   * This is because we'll have 1 set of recaptures per category collection provided.
   * categories male+female male = 2 collections
   */
  map<unsigned, vector<double>> recaptures_by_year;
  unsigned                      obs_expected = number_bins_ * category_labels_.size() + 1;
  LOG_FINE() << "expected obs = " << obs_expected << " number of bins = " << number_bins_ << " tagged categories = " << category_labels_.size();
  vector<vector<string>>& recaptures_data = recaptures_table_->data();
  if (recaptures_data.size() != years_.size()) {
    LOG_ERROR_P(PARAM_RECAPTURED) << "has" << recaptures_data.size() << " rows defined, but " << years_.size() << " should match the number of years provided";
  }

  for (vector<string>& recaptures_data_line : recaptures_data) {
    unsigned year = 0;

    if (recaptures_data_line.size() != obs_expected) {
      LOG_ERROR_P(PARAM_RECAPTURED) << "has " << recaptures_data_line.size() << " values defined, but " << obs_expected
                                    << " should match the age spread * categories + 1 (for year)";
      return;
    }

    if (!utilities::To<unsigned>(recaptures_data_line[0], year)) {
      LOG_ERROR_P(PARAM_RECAPTURED) << "value " << recaptures_data_line[0] << " could not be converted to an unsigned integer. It should be the year for this line";
      return;
    }

    if (std::find(years_.begin(), years_.end(), year) == years_.end()) {
      LOG_ERROR_P(PARAM_RECAPTURED) << "value " << year << " is not a valid year for this observation";
      return;
    }

    for (unsigned i = 1; i < recaptures_data_line.size(); ++i) {
      double value = 0.0;
      if (!utilities::To<double>(recaptures_data_line[i], value))
        LOG_ERROR_P(PARAM_RECAPTURED) << "value (" << recaptures_data_line[i] << ") could not be converted to a Double";
      recaptures_by_year[year].push_back(value);
    }
    if (recaptures_by_year[year].size() != obs_expected - 1)
      LOG_FATAL_P(PARAM_RECAPTURED) << " " << recaptures_by_year[year].size() << " columns but " << obs_expected - 1 << " required. Check that the table is specified correctly";
  }
  /**
   * Build our Recaptured  maps for use in the DoExecute() section
   */
  double value = 0.0;
  for (auto iter = recaptures_by_year.begin(); iter != recaptures_by_year.end(); ++iter) {
    double total = 0.0;

    for (unsigned i = 0; i < category_labels_.size(); ++i) {
      for (unsigned j = 0; j < number_bins_; ++j) {
        unsigned obs_index = i * number_bins_ + j;
        if (!utilities::To<double>(iter->second[obs_index], value)) {
          LOG_ERROR_P(PARAM_OBS) << ": obs_ value (" << iter->second[obs_index] << ") at index " << obs_index + 1 << " in the definition could not be converted to a Double";
        }
        recaptures_[iter->first].push_back(value);
        total += value;
      }
    }
    n_[iter->first] = total;
    LOG_FINE() << "total_recaptures in year = " << iter->first << " = " << n_[iter->first];
  }
}

/**
 * Build any runtime relationships and ensure that the labels for other objects are valid.
 */
void TagRecaptureByLengthForGrowth::DoBuild() {
  partition_        = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model_, category_labels_));
  cached_partition_ = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model_, category_labels_));

  // Build Selectivity pointers
  for (string label : selectivity_labels_) {
    Selectivity* selectivity = model_->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_SELECTIVITIES) << ": Selectivity " << label << " does not exist.";
    selectivities_.push_back(selectivity);
  }

  if (selectivities_.size() == 1 && category_labels_.size() != 1) {
    auto val_sel = selectivities_[0];
    selectivities_.assign(category_labels_.size(), val_sel);
  }

  proportion_of_time_ = time_step_proportion_;

  auto time_step = model_->managers()->time_step()->GetTimeStep(time_step_label_);
  if (!time_step) {
    LOG_ERROR_P(PARAM_TIME_STEP) << "Time step label " << time_step_label_ << " was not found.";
  } else {
    for (unsigned year : years_) time_step->SubscribeToBlock(this, year);
  }

  // reserve memory
  numbers_at_length_.resize(category_labels_.size());
  cached_numbers_at_length_.resize(category_labels_.size());

  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset) {
    numbers_at_length_[category_offset].resize(number_bins_, 0.0);
    cached_numbers_at_length_[category_offset].resize(number_bins_, 0.0);
  }

  length_results_.resize(number_bins_ * category_labels_.size(), 0.0);
  categories_for_comparison_.resize(number_bins_ * category_labels_.size());
  length_bins_for_comparison_.resize(number_bins_ * category_labels_.size(), 0.0);
}

/**
 * This method is called at the start of the taggeded
 * time step for this observation.
 *
 * Build the cache for the partition
 * structure to use with any interpolation
 */
void TagRecaptureByLengthForGrowth::PreExecute() {
  cached_partition_->BuildCache();
}

/**
 * Execute
 */
void TagRecaptureByLengthForGrowth::Execute() {
  LOG_TRACE();
  LOG_FINEST() << "Entering observation " << label_;

  /**
   * Verify our cached partition and partition sizes are correct
   */
  auto     partition_iter         = partition_->Begin();  // vector<vector<partition::Category> >
  unsigned category_length_offset = 0;
  Double   total_expected         = 0.0;
  /**
   * Loop through the provided categories. Each provided category (combination) will have a list of observations
   * with it. We need to build a vector of proportions for each age using that combination and then
   * compare it to the observations.
   */
  std::fill(length_results_.begin(), length_results_.end(), 0.0);

  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset, ++partition_iter) {
    category_length_offset = category_offset * number_bins_;
    LOG_FINEST() << "Observing collection of categories " << category_labels_[category_offset];
    Double start_value = 0.0;
    Double end_value   = 0.0;
    Double final_value = 0.0;
    // reset some category specific containers
    std::fill(numbers_at_length_[category_offset].begin(), numbers_at_length_[category_offset].end(), 0.0);
    std::fill(cached_numbers_at_length_[category_offset].begin(), cached_numbers_at_length_[category_offset].end(), 0.0);
    /**
     * Loop through the  combined categories if they are supplied, building up the
     * numbers at length
     */
    auto category_iter = partition_iter->begin();
    for (; category_iter != partition_iter->end(); ++category_iter) {
      LOG_FINE() << "this category = " << (*category_iter)->name_;
      LOG_FINEST() << "Selectivity for " << category_labels_[category_offset] << " selectivity " << selectivities_[category_offset]->label();
      // Now convert numbers at age to numbers at length using the categories age-length transition matrix
      if (using_model_length_bins) {
        LOG_FINE() << "using model length bins";
        for (unsigned model_length_offset = 0; model_length_offset < model_->get_number_of_length_bins(); ++model_length_offset) {
          // now for each column (length bin) in age_length_matrix sum up all the rows (ages) for both cached and current matricies
          cached_numbers_at_length_[category_offset][model_length_offset]
              += (*category_iter)->cached_data_[model_length_offset] * selectivities_[category_offset]->GetLengthResult(model_length_offset);
          numbers_at_length_[category_offset][model_length_offset]
              += (*category_iter)->data_[model_length_offset] * selectivities_[category_offset]->GetLengthResult(model_length_offset);
        }
      } else {
        LOG_FINE() << "using bespoke length bins";
        for (unsigned model_length_offset = 0; model_length_offset < model_->get_number_of_length_bins(); ++model_length_offset) {
          if (map_local_length_bins_to_global_length_bins_[model_length_offset] >= 0) {
            // now for each column (length bin) in age_length_matrix sum up all the rows (ages) for both cached and current matricies
            cached_numbers_at_length_[category_offset][map_local_length_bins_to_global_length_bins_[model_length_offset]]
                += (*category_iter)->cached_data_[model_length_offset] * selectivities_[category_offset]->GetLengthResult(model_length_offset);
            numbers_at_length_[category_offset][map_local_length_bins_to_global_length_bins_[model_length_offset]]
                += (*category_iter)->data_[model_length_offset] * selectivities_[category_offset]->GetLengthResult(model_length_offset);
          }
        }
      }
    }
    LOG_FINEST() << "category = " << category_labels_[category_offset];
    LOG_FINEST() << "----------";
    // Interpolate between the cached and current values for bothe tagged and untagged
    for (unsigned length_offset = 0; length_offset < number_bins_; ++length_offset) {
      categories_for_comparison_[category_length_offset + length_offset] = category_labels_[category_offset];
      start_value                                                        = cached_numbers_at_length_[category_offset][length_offset];
      end_value                                                          = numbers_at_length_[category_offset][length_offset];
      final_value                                                        = 0.0;

      if (mean_proportion_method_)
        final_value = start_value + ((end_value - start_value) * proportion_of_time_);
      else
        final_value = (1 - proportion_of_time_) * start_value + proportion_of_time_ * end_value;
      length_results_[category_length_offset + length_offset] += final_value;
      length_bins_for_comparison_[category_length_offset + length_offset] = length_bins_[length_offset];
      LOG_FINEST() << "length bin = " << length_bins_[length_offset] << " expected = " << final_value;
      total_expected += final_value;
    }
  }
  Double expected = 0.0;
  double observed = 0.0;
  // save our comparisons so we can use them to generate the score from the likelihoods later
  for (unsigned i = 0; i < length_results_.size(); ++i) {
    // expected = detection_ * tagged_length_results_[i] / length_results_[i];
    expected = length_results_[i] / total_expected;
    LOG_FINEST() << " total numbers at length " << length_bins_for_comparison_[i] << " = " << length_results_[i] << ", denominator = " << total_expected
                 << " category = " << categories_for_comparison_[i];
    observed = recaptures_[model_->current_year()][i] / n_[model_->current_year()];

    SaveComparison(categories_for_comparison_[i], 0, length_bins_for_comparison_[i], expected, observed, process_errors_by_year_[model_->current_year()],
                   n_[model_->current_year()], 0.0, delta_, 0.0);
  }
}

/**
 * This method is called at the end of a model iteration
 * to calculate the score for the observation.
 */
void TagRecaptureByLengthForGrowth::CalculateScore() {
  /**
   * Simulate or generate results
   * During simulation mode we'll simulate results for this observation
   */
  LOG_FINEST() << "Calculating neglogLikelihood for observation = " << label_;

  if (model_->run_mode() == RunMode::kSimulation) {
    likelihood_->SimulateObserved(comparisons_);
  } else {
    likelihood_->GetScores(comparisons_);

    for (unsigned year : years_) {
      scores_[year] = likelihood_->GetInitialScore(comparisons_, year);
      LOG_FINEST() << "-- Observation neglogLikelihood calculation " << label_;
      LOG_FINEST() << "[" << year << "] Initial neglogLikelihood:" << scores_[year];
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
