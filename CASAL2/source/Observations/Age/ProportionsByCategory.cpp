/**
 * @file ProportionsByCategory.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 17/02/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "ProportionsByCategory.h"

#include <algorithm>

#include "../../Partition/Accessors/Cached/CombinedCategories.h"
#include "Categories/Categories.h"
#include "Model/Model.h"
#include "Partition/Accessors/All.h"
#include "Selectivities/Manager.h"
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
ProportionsByCategory::ProportionsByCategory(shared_ptr<Model> model) : Observation(model) {
  obs_table_ = parameters_.BindTable(PARAM_OBS, "The table of observed values");
  obs_table_->set_requires_columns(false);
  error_values_table_ = parameters_.BindTable(PARAM_ERROR_VALUES, "The table of error values of the observed values (note that the units depend on the likelihood)");
  error_values_table_->set_requires_columns(false);

  parameters_.Bind<unsigned>(PARAM_MIN_AGE, &min_age_, "The minimum age");
  parameters_.Bind<unsigned>(PARAM_MAX_AGE, &max_age_, "The maximum age");
  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "The label of the time step that the observation occurs in");
  parameters_.Bind<bool>(PARAM_PLUS_GROUP, &plus_group_, "Use the age plus group?")->set_default_value(true);
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years for which there are observations");
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_labels_, "The labels of the selectivities");
  parameters_.Bind<string>(PARAM_TARGET_CATEGORIES, &target_category_labels_, "The target categories")->flag_is_category(true);
  parameters_.Bind<string>(PARAM_TARGET_SELECTIVITIES, &target_selectivity_labels_, "The target selectivities");
  parameters_.Bind<Double>(PARAM_PROCESS_ERRORS, &process_error_values_, "The process error")->set_is_optional(true);

  allowed_likelihood_types_ = {PARAM_BINOMIAL};
}

/**
 * Validate configuration file parameters
 */
void ProportionsByCategory::DoValidate() {
  parameters_.Validate(PARAM_MIN_AGE)->IsAge()->LessThanOrEqualToParameter(PARAM_MAX_AGE);
  parameters_.Validate(PARAM_MAX_AGE)->IsAge();
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
  parameters_.ValidateVector(PARAM_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES);
  parameters_.ValidateVector(PARAM_TARGET_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES, false)->IsUniqueFrom(PARAM_CATEGORIES);
  parameters_.ValidateVector(PARAM_TARGET_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_TARGET_CATEGORIES)->SameNumberOfElementsAs(PARAM_TARGET_CATEGORIES);
  parameters_.ValidateVector(PARAM_PROCESS_ERRORS)
      ->GreaterThanOrEqualTo(0.0)
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->DefaultValue(0.0, years_.size());

  age_spread_                    = (max_age_ - min_age_) + 1;
  unsigned expected_column_count = age_spread_ * category_labels_.size() + 1;
  parameters_.ValidateTable(PARAM_OBS)
      ->Rows(years_.size(), "Number of rows in the observation table must match the number of years provided")
      ->Columns(expected_column_count, "Expected year, observation values, and error value columns in the observation table")
      ->ColumnIsYear(0, "First column of the observation table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All columns except the first must be a double value (data + error value) for the observation")
      ->GreaterThanOrEqualToForRange(1, expected_column_count - 1, 0.0);

  parameters_.ValidateTable(PARAM_ERROR_VALUES)
      ->Rows(years_.size(), "Number of rows in the error values table must match the number of years provided")
      ->Columns(expected_column_count, "Expected year and error value columns in the error values table")
      ->ColumnIsYear(0, "First column of the error values table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All columns except the first must be a double value (error values) for the observation")
      ->GreaterThanForRange(1, expected_column_count - 1, 0.0);

  process_errors_by_year_ = utilities::Map::create(years_, process_error_values_);
  proportions_            = obs_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, expected_column_count - 1);
  error_values_           = error_values_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, expected_column_count - 1);
}

/**
 * Build any runtime relationships and ensure that the labels for other objects are valid.
 */
void ProportionsByCategory::DoBuild() {
  partition_               = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model(), category_labels_));
  cached_partition_        = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model(), category_labels_));
  target_partition_        = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model(), target_category_labels_));
  target_cached_partition_ = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model(), target_category_labels_));

  if (ageing_error_label_ != "")
    LOG_CODE_ERROR() << "ageing error has not been implemented for proportions at age observations";

  age_results_.resize(age_spread_ * category_labels_.size(), 0.0);

  for (string label : selectivity_labels_) {
    Selectivity* selectivity = model()->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_SELECTIVITIES) << ": Selectivity label " << label << " was not found.";
    selectivities_.push_back(selectivity);
  }

  for (string label : target_selectivity_labels_) {
    auto selectivity = model()->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity) {
      LOG_ERROR_P(PARAM_TARGET_SELECTIVITIES) << ": Selectivity label " << label << " was not found.";
    } else
      target_selectivities_.push_back(selectivity);
  }
}

/**
 * This method is called at the start of the targeted
 * time step for this observation.
 *
 * Build the cache for the partition
 * structure to use with any interpolation
 */
void ProportionsByCategory::PreExecute() {
  cached_partition_->BuildCache();
  target_cached_partition_->BuildCache();

  if (cached_partition_->Size() != proportions_[model()->current_year()].size())
    LOG_CODE_ERROR() << "cached_partition_->Size() != proportions_[model->current_year()].size()";
  if (partition_->Size() != proportions_[model()->current_year()].size())
    LOG_CODE_ERROR() << "partition_->Size() != proportions_[model->current_year()].size()";
}

/**
 * Execute
 */
void ProportionsByCategory::Execute() {
  LOG_TRACE();
  LOG_FINEST() << "Entering observation " << label_;

  /**
   * Verify our cached partition and partition sizes are correct
   */
  auto cached_partition_iter        = cached_partition_->Begin();
  auto partition_iter               = partition_->Begin();  // vector<vector<partition::Category> >
  auto target_cached_partition_iter = target_cached_partition_->Begin();
  auto target_partition_iter        = target_partition_->Begin();  // vector<vector<partition::Category> >

  /**
   * Loop through the provided categories. Each provided category (combination) will have a list of observations
   * with it. We need to build a vector of proportions for each age using that combination and then
   * compare it to the observations.
   */
  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset, ++partition_iter, ++cached_partition_iter) {
    Double selectivity_result = 0.0;
    Double start_value        = 0.0;
    Double end_value          = 0.0;
    Double final_value        = 0.0;

    vector<Double> age_results(age_spread_, 0.0);
    vector<Double> target_age_results(age_spread_, 0.0);

    /**
     * Loop through the 2 combined categories building up the
     * age results proportions values.
     */
    auto category_iter        = partition_iter->begin();
    auto cached_category_iter = cached_partition_iter->begin();
    for (; category_iter != partition_iter->end(); ++cached_category_iter, ++category_iter) {
      for (unsigned data_offset = 0; data_offset < (*category_iter)->data_.size(); ++data_offset) {
        // Check and skip ages we don't care about.
        if ((*category_iter)->min_age_ + data_offset < min_age_)
          continue;
        if ((*category_iter)->min_age_ + data_offset > max_age_ && !plus_group_)
          break;

        unsigned age_offset = ((*category_iter)->min_age_ + data_offset) - min_age_;
        unsigned age        = ((*category_iter)->min_age_ + data_offset);

        if (age < min_age_)
          continue;
        if (age > max_age_)
          break;

        if (min_age_ + age_offset > max_age_)
          age_offset = age_spread_ - 1;

        LOG_FINE() << "---------------";
        LOG_FINE() << "age: " << age;
        selectivity_result = selectivities_[category_offset]->GetAgeResult(age, (*category_iter)->age_length_);
        start_value        = (*cached_category_iter)->cached_data_[data_offset];
        end_value          = (*category_iter)->data_[data_offset];
        final_value        = 0.0;

        if (mean_proportion_method_)
          final_value = start_value + ((end_value - start_value) * proportion_of_time_);
        else
          final_value = fabs(start_value - end_value) * proportion_of_time_;

        LOG_FINE() << "Category1:" << (*category_iter)->name_;
        LOG_FINE() << "selectivity_result: " << selectivity_result;
        LOG_FINE() << "start_value: " << start_value << " / end_value: " << end_value;
        LOG_FINE() << "final_value: " << final_value;
        LOG_FINE() << "final_value * selectivity: " << (Double)(final_value * selectivity_result);

        age_results[age_offset] += final_value * selectivity_result;
        LOG_FINE() << "category1 becomes: " << age_results[age_offset];
      }
    }

    /**
     * Loop through the target combined categories building up the
     * target age results
     */
    auto target_category_iter        = target_partition_iter->begin();
    auto target_cached_category_iter = target_cached_partition_iter->begin();
    for (; target_category_iter != target_partition_iter->end(); ++target_cached_category_iter, ++target_category_iter) {
      for (unsigned data_offset = 0; data_offset < (*target_category_iter)->data_.size(); ++data_offset) {
        // Check and skip ages we don't care about.
        if ((*target_category_iter)->min_age_ + data_offset < min_age_)
          continue;
        if ((*target_category_iter)->min_age_ + data_offset > max_age_ && !plus_group_)
          break;

        unsigned age_offset = ((*target_category_iter)->min_age_ + data_offset) - min_age_;
        unsigned age        = ((*target_category_iter)->min_age_ + data_offset);
        if (min_age_ + age_offset > max_age_)
          age_offset = age_spread_ - 1;
        if (age < min_age_)
          continue;
        if (age > max_age_)
          break;

        selectivity_result = target_selectivities_[category_offset]->GetAgeResult(age, (*target_category_iter)->age_length_);
        start_value        = (*target_cached_category_iter)->cached_data_[data_offset];
        end_value          = (*target_category_iter)->data_[data_offset];
        final_value        = 0.0;

        if (mean_proportion_method_)
          final_value = start_value + ((end_value - start_value) * proportion_of_time_);
        else
          final_value = fabs(start_value - end_value) * proportion_of_time_;
        LOG_FINE() << "----------";
        LOG_FINE() << "Category2:" << (*target_category_iter)->name_;
        LOG_FINE() << "age: " << age;
        LOG_FINE() << "selectivity_result: " << selectivity_result;
        LOG_FINE() << "start_value: " << start_value << " / end_value: " << end_value;
        LOG_FINE() << "final_value: " << final_value;
        LOG_FINE() << "final_value * selectivity: " << (Double)(final_value * selectivity_result);
        target_age_results[age_offset] += final_value * selectivity_result;
        LOG_FINE() << "category2 becomes: " << target_age_results[age_offset];
      }
    }

    if (age_results.size() != proportions_[model()->current_year()][category_labels_[category_offset]].size())
      LOG_CODE_ERROR() << "expected_values.size(" << age_results.size() << ") != proportions_[category_offset].size("
                       << proportions_[model()->current_year()][category_labels_[category_offset]].size() << ")";

    /**
     * save our comparisons so we can use them to generate the score from the likelihoods later
     */
    for (unsigned i = 0; i < age_results.size(); ++i) {
      Double expected = 0.0;
      if (age_results[i] != 0.0)
        expected = target_age_results[i] / age_results[i];

      SaveComparison(category_labels_[category_offset], min_age_ + i, 0, expected, proportions_[model()->current_year()][category_labels_[category_offset]][i],
                     process_errors_by_year_[model()->current_year()], error_values_[model()->current_year()][category_labels_[category_offset]][i], 0.0, delta_, 0.0);
    }
  }
}

/**
 * This method is called at the end of a model iteration
 * to calculate the score for the observation.
 */
void ProportionsByCategory::CalculateScore() {
  /**
   * Simulate or generate results
   * During simulation mode we'll simulate results for this observation
   */
  LOG_FINEST() << "Calculating neglogLikelihood for observation = " << label_;

  if (model()->run_mode() == RunMode::kSimulation) {
    likelihood_->SimulateObserved(comparisons_);
  } else {
    /**
     * Convert the expected_values in to a proportion
     */
    for (unsigned year : years_) {
      //      Double running_total = 0.0;
      //      for (obs::Comparison comparison : comparisons_[year]) {
      //        running_total += comparison.expected_;
      //      }
      //      for (obs::Comparison& comparison : comparisons_[year]) {
      //        if (running_total != 0.0)
      //          comparison.expected_  = comparison.expected_ / running_total;
      //        else
      //          comparison.expected_  = 0.0;
      //      }

      scores_[year] = likelihood_->GetInitialScore(comparisons_, year);
      likelihood_->GetScores(comparisons_);
      for (obs::Comparison comparison : comparisons_[year]) {
        LOG_FINEST() << "[" << year << "] + neglogLikelihood: " << comparison.score_;
        scores_[year] += comparison.score_;
      }
    }
  }
}

} /* namespace age */
} /* namespace observations */
} /* namespace niwa */
