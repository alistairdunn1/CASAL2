/**
 * @file TagRecaptureByLength.cpp
 * @author C.Marsh
 * @github https://github.com/Zaita
 * @date 23/10/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "TagRecaptureByLength.h"

#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim_all.hpp>

#include "../../Partition/Accessors/Cached/CombinedCategories.h"
#include "AgeLengths/AgeLength.h"
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
namespace age {

/**
 * Default constructor
 */
TagRecaptureByLength::TagRecaptureByLength(shared_ptr<Model> model) : Observation(model) {
  recaptures_table_ = parameters_.BindTable(PARAM_RECAPTURED, "The table of observed recaptured individuals in each length bin");
  recaptures_table_->set_requires_columns(false);
  scanned_table_ = parameters_.BindTable(PARAM_SCANNED, "The table of observed scanned individuals in each length bin");
  scanned_table_->set_requires_columns(false);

  // clang-format off
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years for which there are observations")->set_is_optional(true);
  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "The label of the time step that the observation occurs in");
  parameters_.Bind<double>(PARAM_LENGTH_BINS, &length_bins_, "The length bins")->set_is_optional(true);
  parameters_.Bind<bool>(PARAM_PLUS_GROUP, &length_plus_, "Is the last length bin a plus group? (defaults to @model value)")->set_is_optional(true);
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_labels_, "The labels of the selectivities of the scanned categories");
  parameters_.Bind<string>(PARAM_TAGGED_CATEGORIES, &tagged_category_labels_, "The categories of tagged individuals")->flag_is_category(true);
  parameters_.Bind<string>(PARAM_TAGGED_SELECTIVITIES, &tagged_selectivity_labels_, "The labels of the selectivities of the tagged categories");
  parameters_.Bind<Double>(PARAM_DETECTION_PARAMETER, &detection_, "The probability of detecting a recaptured individual");
  parameters_.Bind<Double>(PARAM_DISPERSION, &dispersion_, "The overdispersion parameter (phi)")->set_is_optional(true);
  parameters_.Bind<Double>(PARAM_TIME_STEP_PROPORTION, &time_step_proportion_, "The proportion through the mortality block of the time step when the observation is evaluated")
    ->set_default_value(0.5);
  parameters_.Bind<Double>(PARAM_OVERLAP_SCALAR, &overlap_scalar_, "Scalar for the relative overlap of the tag recaptures")->set_is_optional(true);
  // clang-format on

  RegisterAsAddressable(PARAM_DETECTION_PARAMETER, &detection_);
  RegisterAsAddressable(PARAM_DISPERSION, &dispersion_);
  RegisterAsAddressable(PARAM_OVERLAP_SCALAR, &overlap_scalar_);

  mean_proportion_method_ = true;

  allowed_likelihood_types_.push_back(PARAM_BINOMIAL);
}

/**
 * Validate configuration file parameters
 */
void TagRecaptureByLength::DoValidate() {
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
  parameters_.ValidateVector(PARAM_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES);
  parameters_.ValidateVector(PARAM_TAGGED_CATEGORIES);
  parameters_.ValidateVector(PARAM_TAGGED_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_TAGGED_CATEGORIES)->SameNumberOfElementsAs(PARAM_TAGGED_CATEGORIES);
  parameters_.Validate(PARAM_DETECTION_PARAMETER)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);
  parameters_.ValidateVector(PARAM_DISPERSION)
      ->GreaterThanOrEqualTo(0.0)
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->DefaultValue(1.0, years_.size());
  parameters_.ValidateVector(PARAM_OVERLAP_SCALAR)
      ->GreaterThanOrEqualTo(0.0)
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->DefaultValue(1.0, years_.size());
  parameters_.Validate(PARAM_TIME_STEP_PROPORTION)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);
  parameters_.Validate(PARAM_PLUS_GROUP)->DefaultValue(model()->length_plus());
  parameters_.ValidateVector(PARAM_LENGTH_BINS)->IsLengthBin()->IsInIncreasingOrder()->DefaultToAllModelLengthBins();

  number_bins_                   = length_plus_ ? length_bins_.size() : length_bins_.size() - 1;
  unsigned expected_column_count = number_bins_ * tagged_category_labels_.size() + 1;

  parameters_.ValidateTable(PARAM_RECAPTURED)
      ->Rows(years_.size(), "Number of rows in the recaptured table must match the number of years provided")
      ->Columns(expected_column_count, "Expected year and recaptured values columns in the recaptured table")
      ->ColumnIsYear(0, "First column of the recaptured table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All recaptured except the first must be a double value for the observation")
      ->GreaterThanOrEqualToForRange(1u, expected_column_count - 1, 0.0);

  parameters_.ValidateTable(PARAM_SCANNED)
      ->Rows(years_.size(), "Number of rows in the scanned table must match the number of years provided")
      ->ExpandColumnsTo(expected_column_count - 1, 1u)
      ->Columns(expected_column_count, "Expected year and scanned value columns in the scanned table")
      ->ColumnIsYear(0, "First column of the scanned table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All columns except the first must be a double value for the observation")
      ->GreaterThanOrEqualToForRange(1u, expected_column_count - 1, 0.0);

  recaptures_ = recaptures_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, expected_column_count - 1);
  scanned_    = scanned_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, expected_column_count - 1);

  // Do some checks if we're not using all of the model length bins
  using_model_length_bins = length_bins_.size() == model()->length_bins().size();
  if (!using_model_length_bins)
    map_local_length_bins_to_global_length_bins_ = model()->get_map_for_bespoke_length_bins_to_global_length_bins(length_bins_, length_plus_);

  if (length_plus_ & !model()->length_plus())
    LOG_ERROR_P(PARAM_LENGTH_PLUS)
        << "you have specified a plus group on this observation, but the global length bins don't have a plus group. This is an inconsistency that must be fixed. Try changing the model plus group to false or this plus group to true";

  dispersion_by_year_     = utilities::Map::create(years_, dispersion_);
  overlap_scalar_by_year_ = utilities::Map::create(years_, overlap_scalar_);

  category_split_labels_        = model()->categories()->total_categories(category_labels_);
  tagged_category_split_labels_ = model()->categories()->total_categories(tagged_category_labels_);

  // Do a check so that every Tagged category must be in the categories as well
  for (unsigned i = 0; i < tagged_category_split_labels_.size(); ++i) {
    for (unsigned j = 0; j < tagged_category_split_labels_[i].size(); ++j) {
      if (find(category_split_labels_[i].begin(), category_split_labels_[i].end(), tagged_category_split_labels_[i][j]) == category_split_labels_[i].end()) {
        LOG_ERROR_P(PARAM_TAGGED_CATEGORIES) << "The category " << tagged_category_split_labels_[i][j] << " is not in the " << PARAM_CATEGORIES << " at element " << i + 1
                                             << ". All tagged individuals should be in the categories";
      }
    }
  }
}

/**
 * Build any runtime relationships and ensure that the labels for other objects are valid.
 */
void TagRecaptureByLength::DoBuild() {
  partition_               = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model(), category_labels_));
  cached_partition_        = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model(), category_labels_));
  tagged_partition_        = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model(), tagged_category_labels_));
  tagged_cached_partition_ = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model(), tagged_category_labels_));

  auto partition_iter = partition_->Begin();
  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset, ++partition_iter) {
    auto category_iter = partition_iter->begin();
    for (; category_iter != partition_iter->end(); ++category_iter) (*category_iter)->age_length_->BuildAgeLengthMatrixForTheseYears(years_);
  }

  partition_iter = tagged_partition_->Begin();
  for (unsigned category_offset = 0; category_offset < tagged_category_labels_.size(); ++category_offset, ++partition_iter) {
    auto category_iter = partition_iter->begin();
    for (; category_iter != partition_iter->end(); ++category_iter) (*category_iter)->age_length_->BuildAgeLengthMatrixForTheseYears(years_);
  }

  // Build Selectivity pointers
  for (string label : selectivity_labels_) {
    Selectivity* selectivity = model()->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_SELECTIVITIES) << ": Selectivity " << label << " does not exist.";
    selectivities_.push_back(selectivity);
  }

  if (selectivities_.size() == 1 && category_labels_.size() != 1) {
    auto val_sel = selectivities_[0];
    selectivities_.assign(category_labels_.size(), val_sel);
  }

  for (string label : tagged_selectivity_labels_) {
    auto selectivity = model()->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity) {
      LOG_ERROR_P(PARAM_TAGGED_SELECTIVITIES) << ": Selectivity " << label << " does not exist.";
    } else
      tagged_selectivities_.push_back(selectivity);
  }

  if (tagged_selectivities_.size() == 1 && category_labels_.size() != 1) {
    auto val_t = tagged_selectivities_[0];
    tagged_selectivities_.assign(category_labels_.size(), val_t);
  }

  if (time_step_proportion_ < 0.0 || time_step_proportion_ > 1.0)
    LOG_ERROR_P(PARAM_TIME_STEP_PROPORTION) << ": time_step_proportion (" << time_step_proportion_ << ") must be between 0.0 and 1.0 inclusive";
  proportion_of_time_ = time_step_proportion_;

  auto time_step = model()->managers()->time_step()->GetTimeStep(time_step_label_);
  if (!time_step) {
    LOG_ERROR_P(PARAM_TIME_STEP) << "Time step label " << time_step_label_ << " was not found.";
  } else {
    for (unsigned year : years_) time_step->SubscribeToBlock(this, year);
  }

  // reserve memory
  numbers_at_length_.resize(category_labels_.size());
  cached_numbers_at_length_.resize(category_labels_.size());
  tagged_numbers_at_length_.resize(tagged_category_labels_.size());
  tagged_cached_numbers_at_length_.resize(tagged_category_labels_.size());

  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset) {
    numbers_at_length_[category_offset].resize(number_bins_, 0.0);
    cached_numbers_at_length_[category_offset].resize(number_bins_, 0.0);
    tagged_numbers_at_length_[category_offset].resize(number_bins_, 0.0);
    tagged_cached_numbers_at_length_[category_offset].resize(number_bins_, 0.0);
  }

  length_results_.resize(number_bins_, 0.0);
  tagged_length_results_.resize(number_bins_, 0.0);
}

/**
 * This method is called at the start of the tagged
 * time step for this observation.
 *
 * Build the cache for the partition
 * structure to use with any interpolation
 */
void TagRecaptureByLength::PreExecute() {
  cached_partition_->BuildCache();
  tagged_cached_partition_->BuildCache();

  if (cached_partition_->Size() != scanned_[model()->current_year()].size()) {
    LOG_CODE_ERROR() << "cached_partition_->Size() != scanned_[model->current_year()].size() for " << label_ << " " << cached_partition_->Size()
                     << " != " << scanned_[model()->current_year()].size();
  }
  if (partition_->Size() != scanned_[model()->current_year()].size()) {
    LOG_CODE_ERROR() << "partition_->Size() != scanned_[model->current_year()].size() for " << label_ << " " << partition_->Size()
                     << " != " << scanned_[model()->current_year()].size();
  }
}

/**
 * This method is called at the start of the tagged
 * time step for this observation.
 */
void TagRecaptureByLength::Execute() {
  LOG_TRACE();
  LOG_FINEST() << "Entering observation " << label_;

  /**
   * Verify our cached partition and partition sizes are correct
   */
  auto partition_iter        = partition_->Begin();         // vector<vector<partition::Category> >
  auto tagged_partition_iter = tagged_partition_->Begin();  // vector<vector<partition::Category> >

  unsigned selectivity_index        = 0;
  unsigned tagged_selectivity_index = 0;
  /**
   * Loop through the provided categories. Each provided category (combination) will have a list of observations
   * with it.
   * This is equal between the category_labels_ == tagged_category_labels_
   */
  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset, ++partition_iter, ++tagged_partition_iter) {
    LOG_FINEST() << "Observing first collection of categories " << category_labels_[category_offset];
    Double start_value = 0.0;
    Double end_value   = 0.0;
    Double final_value = 0.0;
    // reset some category specific containers
    std::fill(numbers_at_length_[category_offset].begin(), numbers_at_length_[category_offset].end(), 0.0);
    std::fill(cached_numbers_at_length_[category_offset].begin(), cached_numbers_at_length_[category_offset].end(), 0.0);
    std::fill(tagged_cached_numbers_at_length_[category_offset].begin(), tagged_cached_numbers_at_length_[category_offset].end(), 0.0);
    std::fill(tagged_numbers_at_length_[category_offset].begin(), tagged_numbers_at_length_[category_offset].end(), 0.0);
    std::fill(length_results_.begin(), length_results_.end(), 0.0);
    std::fill(tagged_length_results_.begin(), tagged_length_results_.end(), 0.0);

    std::set<string> selectivity_labels_set;
    /**
     * Loop through the  combined categories if they are supplied, building up the
     * numbers at length
     */
    auto category_iter = partition_iter->begin();
    for (; category_iter != partition_iter->end(); ++category_iter, ++selectivity_index) {
      assert(selectivity_index < selectivities_.size());

      selectivity_labels_set.insert(selectivities_[selectivity_index]->label());
      // get numbers at length
      if (using_model_length_bins) {
        (*category_iter)->age_length_->populate_numbers_at_length((*category_iter)->data_, numbers_at_length_[category_offset], selectivities_[selectivity_index]);
        (*category_iter)->age_length_->populate_numbers_at_length((*category_iter)->cached_data_, cached_numbers_at_length_[category_offset], selectivities_[selectivity_index]);
      } else {
        (*category_iter)
            ->age_length_->populate_numbers_at_length((*category_iter)->data_, numbers_at_length_[category_offset], selectivities_[selectivity_index],
                                                      map_local_length_bins_to_global_length_bins_);
        (*category_iter)
            ->age_length_->populate_numbers_at_length((*category_iter)->cached_data_, cached_numbers_at_length_[category_offset], selectivities_[selectivity_index],
                                                      map_local_length_bins_to_global_length_bins_);
      }
    }
    /**
     * Loop through the  combined categories if they are supplied, building up the
     * numbers at length
     */
    auto tagged_category_iter = tagged_partition_iter->begin();
    for (; tagged_category_iter != tagged_partition_iter->end(); ++tagged_category_iter, ++tagged_selectivity_index) {
      assert(tagged_selectivity_index < tagged_selectivities_.size());
      selectivity_labels_set.insert(tagged_selectivities_[tagged_selectivity_index]->label());

      // get numbers at length
      if (using_model_length_bins) {
        (*tagged_category_iter)
            ->age_length_->populate_numbers_at_length((*tagged_category_iter)->data_, tagged_numbers_at_length_[category_offset], tagged_selectivities_[tagged_selectivity_index]);
        (*tagged_category_iter)
            ->age_length_->populate_numbers_at_length((*tagged_category_iter)->cached_data_, tagged_cached_numbers_at_length_[category_offset],
                                                      tagged_selectivities_[tagged_selectivity_index]);
      } else {
        (*tagged_category_iter)
            ->age_length_->populate_numbers_at_length((*tagged_category_iter)->data_, tagged_numbers_at_length_[category_offset], tagged_selectivities_[tagged_selectivity_index],
                                                      map_local_length_bins_to_global_length_bins_);
        (*tagged_category_iter)
            ->age_length_->populate_numbers_at_length((*tagged_category_iter)->cached_data_, tagged_cached_numbers_at_length_[category_offset],
                                                      tagged_selectivities_[tagged_selectivity_index], map_local_length_bins_to_global_length_bins_);
      }
    }
    // Interpolate between the cached and current values for both the tagged and untagged
    for (unsigned length_offset = 0; length_offset < number_bins_; ++length_offset) {
      start_value = tagged_cached_numbers_at_length_[category_offset][length_offset];
      end_value   = tagged_numbers_at_length_[category_offset][length_offset];
      final_value = 0.0;

      if (mean_proportion_method_)
        final_value = start_value + ((end_value - start_value) * proportion_of_time_);
      else
        final_value = (1 - proportion_of_time_) * start_value + proportion_of_time_ * end_value;

      tagged_length_results_[length_offset] += final_value;

      // Tagged
      start_value = cached_numbers_at_length_[category_offset][length_offset];
      end_value   = numbers_at_length_[category_offset][length_offset];
      final_value = 0.0;

      if (mean_proportion_method_)
        final_value = start_value + ((end_value - start_value) * proportion_of_time_);
      else
        final_value = (1 - proportion_of_time_) * start_value + proportion_of_time_ * end_value;
      length_results_[length_offset] += final_value;
    }

    if (length_results_.size() != scanned_[model()->current_year()][category_labels_[category_offset]].size()) {
      LOG_CODE_ERROR() << "expected_values.size(" << length_results_.size() << ") != proportions_[category_offset].size("
                       << scanned_[model()->current_year()][category_labels_[category_offset]].size() << ")";
    }

    // save our comparisons so we can use them to generate the score from the likelihoods later
    for (unsigned i = 0; i < length_results_.size(); ++i) {
      Double expected = 0.0;
      double observed = 0.0;
      if (length_results_[i] != 0.0) {
        expected = detection_ * tagged_length_results_[i] / (length_results_[i] * overlap_scalar_by_year_[model()->current_year()]);
        LOG_FINEST() << "total numbers at length " << length_bins_[i] << " = " << tagged_length_results_[i] << ", denominator = " << length_results_[i];
      }

      if (scanned_[model()->current_year()][category_labels_[category_offset]][i] == 0.0)
        observed = 0.0;
      else
        observed = (recaptures_[model()->current_year()][category_labels_[category_offset]][i]) / scanned_[model()->current_year()][category_labels_[category_offset]][i];

      // process_error is not used here, and the dispersion is applied to the final likelihood value below
      SaveComparison(tagged_category_labels_[category_offset], selectivity_labels_set, 0, length_bins_[i], expected, observed, 0.0,
                     scanned_[model()->current_year()][category_labels_[category_offset]][i], 0.0, delta_, 0.0);
    }
  }
}

/**
 * This method is called at the end of a model iteration
 * to calculate the score for the observation.
 */
void TagRecaptureByLength::CalculateScore() {
  /**
   * Simulate or generate results
   * During simulation mode we'll simulate results for this observation
   */
  LOG_FINEST() << "Calculating neglogLikelihood for observation = " << label_;

  if (model()->run_mode() == RunMode::kSimulation) {
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
      // Add the dispersion factor to the likelihood score
      scores_[year] /= dispersion_by_year_[year];
    }

    LOG_FINEST() << "Finished calculating neglogLikelihood for = " << label_;
  }
}

} /* namespace age */
} /* namespace observations */
} /* namespace niwa */
