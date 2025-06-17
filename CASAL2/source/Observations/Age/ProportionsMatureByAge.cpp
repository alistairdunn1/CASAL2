/**
 * @file ProportionsMatureByAge.cpp
 * @author  C.Marsh
 * @version 1.0
 * @date 12/7/2017
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// Headers
#include "ProportionsMatureByAge.h"

#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim_all.hpp>

#include "../../Partition/Accessors/Cached/CombinedCategories.h"
#include "AgeingErrors/AgeingError.h"
#include "AgeingErrors/Manager.h"
#include "Categories/Categories.h"
#include "Model/Model.h"
#include "Partition/Accessors/All.h"
#include "TimeSteps/Manager.h"
#include "Utilities/Map.h"
#include "Utilities/Math.h"
#include "Utilities/To.h"

// Namespaces
namespace niwa {
namespace observations {
namespace age {

/**
 * Default constructor
 */
ProportionsMatureByAge::ProportionsMatureByAge(shared_ptr<Model> model) : Observation(model) {
  obs_table_ = parameters_.BindTable(PARAM_OBS, "The table of proportions at age mature");
  obs_table_->set_requires_columns(false);
  error_values_table_ = parameters_.BindTable(PARAM_ERROR_VALUES, "The table of error values of the observed values (note the units depend on the likelihood)");
  error_values_table_->set_requires_columns(false);

  // clang-format off
  parameters_.Bind<unsigned>(PARAM_MIN_AGE, &min_age_, "The minimum age");
  parameters_.Bind<unsigned>(PARAM_MAX_AGE, &max_age_, "The maximum age");
  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "The label of time-step that the observation occurs in");
  parameters_.Bind<bool>(PARAM_PLUS_GROUP, &plus_group_, "Use the age plus group?")->set_default_value(true);
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years for which there are observations");
  parameters_.Bind<string>(PARAM_AGEING_ERROR, &ageing_error_label_, "The label of ageing error to use")->set_default_value("");
  parameters_.Bind<string>(PARAM_TOTAL_CATEGORIES, &total_category_labels_,
      "All category labels that were vulnerable to sampling at the time of this observation (not including the categories already given)")
      ->flag_is_category(true);
  parameters_.Bind<Double>(PARAM_TIME_STEP_PROPORTION, &time_step_proportion_, "The proportion through the mortality block of the time step when the observation is evaluated")
      ->set_default_value(0.5);
  // clang-format on

  mean_proportion_method_ = false;

  allowed_likelihood_types_.push_back(PARAM_BINOMIAL);
}

/**
 * Validate total_categories command
 */
void ProportionsMatureByAge::DoValidate() {
  parameters_.Validate(PARAM_MIN_AGE)->IsAge()->LessThanOrEqualToParameter(PARAM_MAX_AGE);
  parameters_.Validate(PARAM_MAX_AGE)->IsAge();
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
  parameters_.ValidateVector(PARAM_TOTAL_CATEGORIES)->IsUniqueFrom(PARAM_CATEGORIES);
  parameters_.Validate(PARAM_TIME_STEP_PROPORTION)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);

  age_spread_                    = (max_age_ - min_age_) + 1;
  unsigned expected_column_count = age_spread_ * category_labels_.size() + 1;
  parameters_.ValidateTable(PARAM_OBS)
      ->Rows(years_.size(), "Number of rows in the observation table must match the number of years provided")
      ->Columns(expected_column_count, "Expected year, observation values, and error value columns in the observation table")
      ->ColumnIsYear(0, "First column of the observation table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All columns except the first must be a double value (data + error value) for the observation")
      ->GreaterThan(expected_column_count - 1, 0.0);

  parameters_.ValidateTable(PARAM_ERROR_VALUES)
      ->Rows(years_.size(), "Number of rows in the error values table must match the number of years provided")
      ->Columns(expected_column_count, "Expected year and error value columns in the error values table")
      ->ColumnIsYear(0, "First column of the error values table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All columns except the first must be a double value (error values) for the observation")
      ->GreaterThanForRange(1, expected_column_count - 1, 0.0);

  proportions_  = obs_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, expected_column_count - 1);
  error_values_ = error_values_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, expected_column_count - 1);
}

/**
 * Build any runtime relationships and ensure that
 * the labels for other objects are valid.
 */
void ProportionsMatureByAge::DoBuild() {
  LOG_TRACE();
  // Get all categories in the system.
  total_partition_        = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model_, total_category_labels_));
  total_cached_partition_ = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model_, total_category_labels_));

  // all categories
  partition_        = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model_, category_labels_));
  cached_partition_ = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model_, category_labels_));

  // Create a pointer to misclassification matrix
  if (ageing_error_label_ != "") {
    ageing_error_ = model_->managers()->ageing_error()->GetAgeingError(ageing_error_label_);
    if (!ageing_error_)
      LOG_ERROR_P(PARAM_AGEING_ERROR) << "Ageing error label (" << ageing_error_label_ << ") was not found.";
  }

  age_results_.resize(age_spread_ * category_labels_.size(), 0.0);

  TimeStep* time_step = model_->managers()->time_step()->GetTimeStep(time_step_label_);
  if (!time_step) {
    LOG_FATAL_P(PARAM_TIME_STEP) << "Time step label " << time_step_label_ << " was not found.";
  } else {
    for (unsigned year : years_) time_step->SubscribeToBlock(this, year);
  }
}

/**
 * This method is called at the start of the time step for this observation.
 *
 * Build the cache for the partition structure to use with any interpolation
 */
void ProportionsMatureByAge::PreExecute() {
  LOG_TRACE();
  cached_partition_->BuildCache();
  total_cached_partition_->BuildCache();
  LOG_FINEST() << "Entering observation " << label_;

  if (cached_partition_->Size() != proportions_[model_->current_year()].size()) {
    LOG_CODE_ERROR() << "cached_partition_->Size() != proportions_[model->current_year()].size()";
  }
  if (total_cached_partition_->Size() != proportions_[model_->current_year()].size()) {
    LOG_CODE_ERROR() << "total_cached_partition_->Size() != proportions_[model->current_year()].size()";
  }

  if (partition_->Size() != proportions_[model_->current_year()].size())
    LOG_CODE_ERROR() << "partition_->Size() != proportions_[model->current_year()].size()";
  if (total_partition_->Size() != proportions_[model_->current_year()].size())
    LOG_CODE_ERROR() << "total_partition_->Size() != proportions_[model->current_year()].size()";
}

/**
 * Execute
 */
void ProportionsMatureByAge::Execute() {
  LOG_TRACE();

  /**
   * Verify our cached partition and partition sizes are correct
   */
  auto cached_partition_iter       = cached_partition_->Begin();
  auto partition_iter              = partition_->Begin();
  auto total_cached_partition_iter = total_cached_partition_->Begin();
  auto total_partition_iter        = total_partition_->Begin();

  vector<Double> expected_values(age_spread_, 0.0);
  vector<Double> numbers_age((model_->age_spread() + 1), 0.0);
  vector<Double> total_numbers_age((model_->age_spread() + 1), 0.0);

  /**
   * Loop through the provided categories. Each provided category (combination) will have a list of observations
   * with it. We need to build a vector of proportions for each age using that combination and then
   * compare it to the observations.
   */
  LOG_FINEST() << "Number of categories " << category_labels_.size();
  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset, ++partition_iter, ++cached_partition_iter) {
    Double start_value = 0.0;
    Double end_value   = 0.0;

    std::fill(expected_values.begin(), expected_values.end(), 0.0);
    std::fill(numbers_age.begin(), numbers_age.end(), 0.0);
    std::fill(total_numbers_age.begin(), total_numbers_age.end(), 0.0);

    /**
     * Loop through the total categories building up numbers at age.
     */
    auto total_category_iter        = total_partition_iter->begin();
    auto total_cached_category_iter = total_cached_partition_iter->begin();
    for (; total_category_iter != total_partition_iter->end(); ++total_cached_category_iter, ++total_category_iter) {
      for (unsigned data_offset = 0; data_offset < (*total_category_iter)->data_.size(); ++data_offset) {
        // We now need to loop through all ages to apply ageing misclassification matrix to account
        // for ages older than max_age_ that could be classified as an individual within the observation range
        unsigned age = ((*total_category_iter)->min_age_ + data_offset);

        start_value = (*total_cached_category_iter)->cached_data_[data_offset];
        end_value   = (*total_category_iter)->data_[data_offset];

        total_numbers_age[data_offset] += start_value + (end_value - start_value) * time_step_proportion_;

        LOG_FINE() << "----------";
        LOG_FINE() << "Category: " << (*total_category_iter)->name_ << " at age " << age;
        LOG_FINE() << "start_value: " << start_value << "; end_value: " << end_value;
      }
    }

    /**
     * Loop through the categories building up numbers at age for the mature, but also adding them onto the total .
     */
    auto category_iter        = partition_iter->begin();
    auto cached_category_iter = cached_partition_iter->begin();
    for (; category_iter != partition_iter->end(); ++cached_category_iter, ++category_iter) {
      for (unsigned data_offset = 0; data_offset < (*category_iter)->data_.size(); ++data_offset) {
        // We now need to loop through all ages to apply ageing misclassification matrix to account
        // for ages older than max_age_ that could be classified as an individual within the observation range
        unsigned age = ((*category_iter)->min_age_ + data_offset);

        start_value = (*cached_category_iter)->cached_data_[data_offset];
        end_value   = (*category_iter)->data_[data_offset];

        numbers_age[data_offset] += start_value + (end_value - start_value) * time_step_proportion_;
        total_numbers_age[data_offset] += start_value + (end_value - start_value) * time_step_proportion_;

        LOG_FINE() << "----------";
        LOG_FINE() << "Category: " << (*category_iter)->name_ << " at age " << age;
        LOG_FINE() << "start_value: " << start_value << "; end_value: " << end_value;
      }
    }

    /*
     *  Apply Ageing error on numbers at age before and after
     */
    if (ageing_error_label_ != "") {
      vector<vector<Double>>& mis_matrix = ageing_error_->mis_matrix();
      vector<Double>          temp(numbers_age.size(), 0.0);
      vector<Double>          total_temp(total_numbers_age.size(), 0.0);

      for (unsigned i = 0; i < mis_matrix.size(); ++i) {
        for (unsigned j = 0; j < mis_matrix[i].size(); ++j) {
          temp[j] += numbers_age[i] * mis_matrix[i][j];
          total_temp[j] += total_numbers_age[i] * mis_matrix[i][j];
        }
      }
      numbers_age       = temp;
      total_numbers_age = total_temp;
    }

    /*
     *  Now collapse the number_age into expected values by age
     */
    Double plus = 0, total_plus = 0;
    for (unsigned k = 0; k < numbers_age.size(); ++k) {
      // this is the difference between the
      unsigned age_offset = min_age_ - model_->min_age();
      if (k >= age_offset && (k - age_offset + min_age_) <= max_age_) {
        // not plus group
        if (total_numbers_age[k] > 0.0) {
          expected_values[k - age_offset] = numbers_age[k] / total_numbers_age[k];
          LOG_FINEST() << "age = " << min_age_ + k << " total = " << total_numbers_age[k] << " mature = " << numbers_age[k];
        } else {
          expected_values[k - age_offset] = 0.0;
          LOG_FINEST() << "age = " << k + age_offset << " total = " << total_numbers_age[k] << " mature = " << numbers_age[k];
        }
      }

      if (((k - age_offset + min_age_) > max_age_) && plus_group_) {
        // plus group
        plus += numbers_age[k];
        total_plus += total_numbers_age[k];
      }
    }

    LOG_FINEST() << "Plus group before migration = " << plus << " Plus group after migration = " << total_plus;
    if (plus_group_) {
      if (total_plus > 0.0)
        expected_values[age_spread_ - 1] = plus / total_plus;
      else
        expected_values[age_spread_ - 1] = 0.0;
    }

    if (expected_values.size() != proportions_[model_->current_year()][category_labels_[category_offset]].size())
      LOG_CODE_ERROR() << "expected_values.size(" << expected_values.size() << ") != proportions_[category_offset].size("
                       << proportions_[model_->current_year()][category_labels_[category_offset]].size() << ")";

    /**
     * save our comparisons so we can use them to generate the score from the likelihoods later
     */
    for (unsigned i = 0; i < expected_values.size(); ++i) {
      LOG_FINEST() << "proportions mature at age " << min_age_ + i << " = " << expected_values[i];
      SaveComparison(category_labels_[category_offset], min_age_ + i, 0.0, expected_values[i], proportions_[model_->current_year()][category_labels_[category_offset]][i],
                     process_errors_by_year_[model_->current_year()], error_values_[model_->current_year()][category_labels_[category_offset]][i], 0.0, delta_, 0.0);
    }
  }
}

/**
 * This method is called at the end of a model iteration
 * to calculate the score for the observation.
 */
void ProportionsMatureByAge::CalculateScore() {
  /**
   * Simulate or generate results
   * During simulation mode we'll simulate results for this observation
   */
  LOG_FINEST() << "Calculating neglogLikelihood for observation = " << label_;

  if (model_->run_mode() == RunMode::kSimulation) {
    likelihood_->SimulateObserved(comparisons_);
  } else {
    /**
     * The comparisons are already proportions so the can be sent straight to the likelihood
     */
    likelihood_->GetScores(comparisons_);

    for (unsigned year : years_) {
      scores_[year] = likelihood_->GetInitialScore(comparisons_, year);
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
