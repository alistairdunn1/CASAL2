/**
 * @file ProportionsMigrating.cpp
 * @author  C.Marsh
 * @version 1.0
 * @date 8/10/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "ProportionsMigrating.h"

#include <algorithm>

#include "../../Partition/Accessors/Cached/CombinedCategories.h"
#include "AgeingErrors/AgeingError.h"
#include "AgeingErrors/Manager.h"
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
ProportionsMigrating::ProportionsMigrating(shared_ptr<Model> model) : Observation(model) {
  obs_table_ = parameters_.BindTable(PARAM_OBS, "The table of observed values");
  obs_table_->set_requires_columns(false);
  error_values_table_ = parameters_.BindTable(PARAM_ERROR_VALUES, "The table of error values of the observed values (note that the units depend on the likelihood)");
  error_values_table_->set_requires_columns(false);

  parameters_.Bind<unsigned>(PARAM_MIN_AGE, &min_age_, "The minimum age");
  parameters_.Bind<unsigned>(PARAM_MAX_AGE, &max_age_, "The maximum age");
  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "The label of the time step that the observation occurs in");
  parameters_.Bind<bool>(PARAM_PLUS_GROUP, &plus_group_, "Is the maximum age the age plus group?")->set_default_value(true);
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years for which there are observations");
  parameters_.Bind<Double>(PARAM_PROCESS_ERRORS, &process_error_values_, "The process error")->set_is_optional(true);
  parameters_.Bind<string>(PARAM_AGEING_ERROR, &ageing_error_label_, "The label of the ageing error to use")->set_default_value("");
  parameters_.Bind<string>(PARAM_PROCESS, &process_label_, "The process label");

  mean_proportion_method_ = false;

  allowed_likelihood_types_.push_back(PARAM_LOGNORMAL);
  allowed_likelihood_types_.push_back(PARAM_MULTINOMIAL);
  allowed_likelihood_types_.push_back(PARAM_DIRICHLET);
}

/**
 * Validate configuration file parameters
 */
void ProportionsMigrating::DoValidate() {
  parameters_.Validate(PARAM_MIN_AGE)->IsAge()->LessThanOrEqualToParameter(PARAM_MAX_AGE);
  parameters_.Validate(PARAM_MAX_AGE)->IsAge();
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
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
      ->GreaterThan(expected_column_count - 1, 0.0);

  parameters_.ValidateTable(PARAM_ERROR_VALUES)
      ->Rows(years_.size(), "Number of rows in the error values table must match the number of years provided")
      ->ExpandColumnsTo(expected_column_count - 1, 1u)
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
void ProportionsMigrating::DoBuild() {
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
  } else
    time_step->SubscribeToProcess(this, years_, process_label_);
}

/**
 * This method is called at the start of the targeted
 * time step for this observation.
 *
 * Build the cache for the partition
 * structure to use with any interpolation
 */
void ProportionsMigrating::PreExecute() {
  cached_partition_->BuildCache();
  LOG_FINEST() << "Entering observation " << label_;

  if (cached_partition_->Size() != proportions_[model_->current_year()].size()) {
    LOG_MEDIUM() << "Cached size " << cached_partition_->Size() << " proportions size = " << proportions_[model_->current_year()].size();
    LOG_CODE_ERROR() << "cached_partition_->Size() != proportions_[model->current_year()].size()";
  }
  if (partition_->Size() != proportions_[model_->current_year()].size())
    LOG_CODE_ERROR() << "partition_->Size() != proportions_[model->current_year()].size()";

  expected_values_.resize(age_spread_, 0.0);
  numbers_age_before_.resize((model_->age_spread() + 1), 0.0);
  numbers_age_after_.resize((model_->age_spread() + 1), 0.0);
  numbers_age_before_with_ageing_error_.resize(numbers_age_before_.size(), 0.0);
  numbers_age_after_with_ageing_error_.resize(numbers_age_after_.size(), 0.0);
}

/**
 * Execute
 */
void ProportionsMigrating::Execute() {
  LOG_TRACE();

  /**
   * Verify our cached partition and partition sizes are correct
   */
  auto cached_partition_iter = cached_partition_->Begin();
  auto partition_iter        = partition_->Begin();  // vector<vector<partition::Category> >

  /**
   * Loop through the provided categories. Each provided category (combination) will have a list of observations
   * with it. We need to build a vector of proportions for each age using that combination and then
   * compare it to the observations.
   */
  LOG_FINEST() << "Number of categories " << category_labels_.size();
  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset, ++partition_iter, ++cached_partition_iter) {
    Double start_value = 0.0;
    Double end_value   = 0.0;

    fill(numbers_age_before_.begin(), numbers_age_before_.end(), 0.0);
    fill(numbers_age_after_.begin(), numbers_age_after_.end(), 0.0);
    fill(expected_values_.begin(), expected_values_.end(), 0.0);

    /**
     * Loop through the 2 combined categories building up the
     * expected proportions values.
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

        numbers_age_before_[data_offset] += start_value;
        numbers_age_after_[data_offset] += end_value;

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
      fill(numbers_age_before_with_ageing_error_.begin(), numbers_age_before_with_ageing_error_.end(), 0.0);
      fill(numbers_age_after_with_ageing_error_.begin(), numbers_age_after_with_ageing_error_.end(), 0.0);

      for (unsigned i = 0; i < mis_matrix.size(); ++i) {
        for (unsigned j = 0; j < mis_matrix[i].size(); ++j) {
          numbers_age_before_with_ageing_error_[j] += numbers_age_before_[i] * mis_matrix[i][j];
          numbers_age_after_with_ageing_error_[j] += numbers_age_after_[i] * mis_matrix[i][j];
        }
      }

      numbers_age_before_ = numbers_age_before_with_ageing_error_;
      numbers_age_after_  = numbers_age_after_with_ageing_error_;
    }

    /*
     *  Now collapse the number_age into out expected values
     */
    Double plus_before = 0, plus_after = 0;
    for (unsigned k = 0; k < numbers_age_before_.size(); ++k) {
      // this is the difference between the
      unsigned age_offset = min_age_ - model_->min_age();
      if (numbers_age_before_[k] > 0) {
        if (k >= age_offset && (k - age_offset + min_age_) <= max_age_) {
          expected_values_[k - age_offset] = (numbers_age_before_[k] - numbers_age_after_[k]) / numbers_age_before_[k];
          LOG_FINEST() << "Numbers before migration = " << numbers_age_before_[k] << " numbers after migration = " << numbers_age_after_[k]
                       << " proportion migrated = " << expected_values_[k - age_offset];
        }
        if (((k - age_offset + min_age_) > max_age_) && plus_group_) {
          plus_before += numbers_age_before_[k];
          plus_after += numbers_age_after_[k];
        }
      } else {
        if (k >= age_offset && (k - age_offset + min_age_) <= max_age_)
          expected_values_[k] = 0;
        if (((k - age_offset + min_age_) > max_age_) && plus_group_) {
          plus_before += 0;
          plus_after += 0;
        }
      }
    }

    LOG_FINEST() << "Plus group before migration = " << plus_before << " Plus group after migration = " << plus_after;
    if (plus_group_)
      expected_values_[age_spread_ - 1] = (plus_before - plus_after) / plus_before;

    if (expected_values_.size() != proportions_[model_->current_year()][category_labels_[category_offset]].size())
      LOG_CODE_ERROR() << "expected_values.size(" << expected_values_.size() << ") != proportions_[category_offset].size("
                       << proportions_[model_->current_year()][category_labels_[category_offset]].size() << ")";

    /**
     * save our comparisons so we can use them to generate the score from the likelihoods later
     */
    for (unsigned i = 0; i < expected_values_.size(); ++i) {
      LOG_FINEST() << " Numbers at age " << min_age_ + i << " = " << expected_values_[i];
      SaveComparison(category_labels_[category_offset], min_age_ + i, 0.0, expected_values_[i], proportions_[model_->current_year()][category_labels_[category_offset]][i],
                     process_errors_by_year_[model_->current_year()], error_values_[model_->current_year()][category_labels_[category_offset]][i], 0.0, delta_, 0.0);
    }
  }
}

/**
 * This method is called at the end of a model iteration
 * to calculate the score for the observation.
 */
void ProportionsMigrating::CalculateScore() {
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
