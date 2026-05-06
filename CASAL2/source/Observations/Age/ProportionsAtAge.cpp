/**
 * @file ProportionsAtAge.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 8/04/2013
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "ProportionsAtAge.h"

#include <algorithm>

#include "../../Partition/Accessors/Cached/CombinedCategories.h"
#include "AgeingErrors/AgeingError.h"
#include "AgeingErrors/Manager.h"
#include "Model/Model.h"
#include "Partition/Accessors/All.h"
#include "Selectivities/Manager.h"
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
ProportionsAtAge::ProportionsAtAge(shared_ptr<Model> model) : Observation(model) {
  obs_table_ = parameters_.BindTable(PARAM_OBS, "The table of observed values");
  obs_table_->set_requires_columns(false);
  error_values_table_ = parameters_.BindTable(PARAM_ERROR_VALUES, "");
  error_values_table_->set_requires_columns(false);

  // clang-format off
  parameters_.Bind<unsigned>(PARAM_MIN_AGE, &min_age_, "The minimum age");
  parameters_.Bind<unsigned>(PARAM_MAX_AGE, &max_age_, "The maximum age");
  parameters_.Bind<bool>(PARAM_PLUS_GROUP, &plus_group_, "Is the maximum age the age plus group?")->set_default_value(true);
  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "The label of the time step that the observation occurs in");
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years of the observed values")->set_is_optional(true);
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_labels_, "The labels of the selectivities")->set_is_optional(true);
  parameters_.Bind<Double>(PARAM_PROCESS_ERRORS, &process_error_values_, "The process error")->set_is_optional(true);
  parameters_.Bind<string>(PARAM_AGEING_ERROR, &ageing_error_label_, "The label of ageing error to use")->set_default_value("");
  parameters_.Bind<bool>(PARAM_SIMULATED_DATA_SUM_TO_ONE, &simulated_data_sum_to_one_, "Whether simulated data is discrete or scaled by totals to be proportions for each year")
    ->set_default_value(true);
  parameters_.Bind<bool>(PARAM_SUM_TO_ONE, &sum_to_one_, "Scale year (row) observed values by the total, so they sum = 1")->set_default_value(false);
  // clang-format on

  allowed_likelihood_types_ = {PARAM_LOGNORMAL, PARAM_MULTINOMIAL, PARAM_DIRICHLET, PARAM_DIRICHLET_MULTINOMIAL, PARAM_LOGISTIC_NORMAL};
}

/**
 * Validate configuration file parameters
 */
void ProportionsAtAge::DoValidate() {
  LOG_TRACE();
  // set up some variables we'll need
  age_spread_                    = (max_age_ - min_age_) + 1;
  unsigned expected_column_count = age_spread_ * category_labels_.size() + 1;  // +1 for the year column

  parameters_.Validate(PARAM_MIN_AGE)->IsAge()->LessThanOrEqualToParameter(PARAM_MAX_AGE);
  parameters_.Validate(PARAM_MAX_AGE)->IsAge();
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
  parameters_.ValidateVector(PARAM_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES);
  parameters_.ValidateVector(PARAM_PROCESS_ERRORS)
      ->GreaterThanOrEqualTo(0.0)
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->DefaultValue(0.0, years_.size());

  parameters_.ValidateTable(PARAM_OBS)
      ->Rows(years_.size(), "Number of rows in the observation table must match the number of years provided")
      ->Columns(expected_column_count, "Expected year, observation values, and error value columns in the observation table")
      ->ColumnIsYear(0, "First column of the observation table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All columns except the first must be a double value (data + error value) for the observation")
      ->GreaterThanOrEqualToForRange(1u, expected_column_count - 1, 0.0);

  parameters_.ValidateTable(PARAM_ERROR_VALUES)
      ->Rows(years_.size(), "Number of rows in the error values table must match the number of years provided")
      ->ExpandColumnsTo(expected_column_count - 1, 1u)
      ->Columns(expected_column_count, "Expected year and error value columns in the error values table")
      ->ColumnIsYear(0, "First column of the error values table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All columns except the first must be a double value (error values) for the observation")
      ->GreaterThanForRange(1, expected_column_count - 1, 0.0);

  process_errors_by_year_ = utilities::Map::create(years_, process_error_values_);

  proportions_  = obs_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, expected_column_count - 1);
  error_values_ = error_values_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, expected_column_count - 1);
  if (sum_to_one_) {
    for (auto& year_pair : proportions_) {
      niwa::utilities::map::scale_to_one<string>(year_pair.second);
    }
  }
}

/**
 * Build any runtime relationships and ensure that the labels for other objects are valid.
 */
void ProportionsAtAge::DoBuild() {
  LOG_TRACE();
  partition_        = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model(), category_labels_));
  cached_partition_ = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model(), category_labels_));

  // Build Selectivity pointers
  for (string label : selectivity_labels_) {
    LOG_FINEST() << "label = " << label;
    Selectivity* selectivity = model()->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_SELECTIVITIES) << ": Selectivity label " << label << " was not found.";
    selectivities_.push_back(selectivity);
  }

  if (selectivities_.size() == 1 && category_labels_.size() != 1) {
    auto val_sel = selectivities_[0];
    selectivities_.assign(category_labels_.size(), val_sel);
  }

  // Create a pointer to Ageing error misclassification matrix
  if (ageing_error_label_ != "") {
    ageing_error_ = model()->managers()->ageing_error()->GetAgeingError(ageing_error_label_);
    if (!ageing_error_)
      LOG_ERROR_P(PARAM_AGEING_ERROR) << "Ageing error label (" << ageing_error_label_ << ") was not found.";
  }
  age_results_.resize(age_spread_ * category_labels_.size(), 0.0);

  // Find out how many selectivities there are, we can have selectivities for combined categories. this is what I am trying to solve for
  // We can either have 1 selectivity
  // A selectivity for each category_label
  // or a selectivity for each combined category in each category_label (total categories) These are defined by business rules in the DoValidate.

  if (selectivity_labels_.size() > category_labels_.size()) {
    selectivity_for_combined_categories_ = true;
  }

  expected_values_.resize(age_spread_, 0.0);
  numbers_age_.resize((model()->age_spread() + 1), 0.0);
  numbers_at_age_with_error_.resize((model()->age_spread() + 1), 0.0);
}

/**
 * This method is called at the start of the targeted
 * time step for this observation.
 *
 * Build the cache for the partition
 * structure to use with any interpolation
 */
void ProportionsAtAge::PreExecute() {
  LOG_TRACE();
  cached_partition_->BuildCache();

  if (cached_partition_->Size() != proportions_[model()->current_year()].size())
    LOG_CODE_ERROR() << "cached_partition_->Size() != proportions_[model->current_year()].size()";
  if (partition_->Size() != proportions_[model()->current_year()].size())
    LOG_CODE_ERROR() << "partition_->Size() != proportions_[model->current_year()].size()";
}

/**
 * Execute
 */
void ProportionsAtAge::Execute() {
  LOG_TRACE();
  LOG_FINEST() << "Entering observation " << label_;
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
  unsigned selectivity_iter = 0;

  LOG_FINEST() << "Number of categories " << category_labels_.size();
  for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset, ++partition_iter, ++cached_partition_iter) {
    Double selectivity_result = 0.0;
    Double start_value        = 0.0;
    Double end_value          = 0.0;
    Double final_value        = 0.0;

    fill(numbers_age_.begin(), numbers_age_.end(), 0.0);
    fill(expected_values_.begin(), expected_values_.end(), 0.0);
    /**
     * Loop through the 2 combined categories building up the
     * expected proportions values.
     */
    auto category_iter        = partition_iter->begin();
    auto cached_category_iter = cached_partition_iter->begin();
    for (; category_iter != partition_iter->end(); ++cached_category_iter, ++category_iter, ++selectivity_iter) {
      if (selectivity_iter >= selectivities_.size())
        LOG_CODE_ERROR() << "selectivity_iter ( " << selectivity_iter << ") >= selectivities_.size() (" << selectivities_.size() << ")";

      LOG_FINEST() << "using selectivity = " << selectivities_[selectivity_iter]->label();
      for (unsigned data_offset = 0; data_offset < (*category_iter)->data_.size(); ++data_offset) {
        // We now need to loop through all ages to apply ageing misclassification matrix to account
        // for ages older than max_age_ that could be classified as an individual within the observation range
        unsigned age       = ((*category_iter)->min_age_ + data_offset);
        selectivity_result = selectivities_[selectivity_iter]->GetAgeResult(age, (*category_iter)->age_length_);
        start_value        = (*cached_category_iter)->cached_data_[data_offset];
        end_value          = (*category_iter)->data_[data_offset];
        final_value        = 0.0;

        if (mean_proportion_method_) {
          final_value = start_value + ((end_value - start_value) * proportion_of_time_);
          numbers_age_[data_offset] += final_value * selectivity_result;
        } else {
          final_value = (1 - proportion_of_time_) * start_value + proportion_of_time_ * end_value;
          numbers_age_[data_offset] += final_value * selectivity_result;
        }

        LOG_FINE() << "----------";
        LOG_FINE() << "Category: " << (*category_iter)->name_ << " at age " << age;
        LOG_FINE() << "Selectivity: " << selectivities_[selectivity_iter]->label() << ": " << selectivity_result;
        LOG_FINE() << "start_value: " << start_value << "; end_value: " << end_value << "; final_value: " << final_value;
        LOG_FINE() << "Numbers at age before ageing error is applied: " << numbers_age_[data_offset];
      }
      // if (selectivity_for_combined_categories_) {
      //   ++selectivity_iter;
      // }
    }

    /*
     *  Apply Ageing error on numbers at age
     */
    if (ageing_error_label_ != "") {
      vector<vector<Double>>& mis_matrix = ageing_error_->mis_matrix();
      fill(numbers_at_age_with_error_.begin(), numbers_at_age_with_error_.end(), 0.0);
      for (unsigned i = 0; i < mis_matrix.size(); ++i) {
        for (unsigned j = 0; j < mis_matrix[i].size(); ++j) {
          numbers_at_age_with_error_[j] += numbers_age_[i] * mis_matrix[i][j];
        }
      }
      numbers_age_ = numbers_at_age_with_error_;
    }

    /*
     *  Now collapse the number_age into out expected values
     */
    for (unsigned k = 0; k < numbers_age_.size(); ++k) {
      // this is the difference between the
      unsigned age_offset = min_age_ - model()->min_age();
      if (k >= age_offset && (k - age_offset + min_age_) <= max_age_) {
        expected_values_[k - age_offset] = numbers_age_[k];
      }
      if (((k - age_offset + min_age_) > max_age_) && plus_group_) {
        expected_values_[age_spread_ - 1] += numbers_age_[k];
      }
    }

    if (expected_values_.size() != proportions_[model()->current_year()][category_labels_[category_offset]].size())
      LOG_CODE_ERROR() << "expected_values_.size(" << expected_values_.size() << ") != proportions_[category_offset].size("
                       << proportions_[model()->current_year()][category_labels_[category_offset]].size() << ")";

    for (unsigned i = 0; i < expected_values_.size(); ++i) {
      LOG_FINEST() << "-----";
      LOG_FINEST() << "Numbers at age for all categories in age " << min_age_ + i << " = " << expected_values_[i];

      SaveComparison(category_labels_[category_offset], min_age_ + i, 0.0, expected_values_[i], proportions_[model()->current_year()][category_labels_[category_offset]][i],
                     process_errors_by_year_[model()->current_year()], error_values_[model()->current_year()][category_labels_[category_offset]][i], 0.0, delta_, 0.0);
    }
  }
}

/**
 * This method is called at the end of a model iteration
 * to calculate the score for the observation.
 */
void ProportionsAtAge::CalculateScore() {
  /**
   * Simulate or generate results
   * During simulation mode we'll simulate results for this observation
   */
  LOG_FINEST() << "Calculating neglogLikelihood for observation = " << label_;

  if (model()->run_mode() == RunMode::kSimulation) {
    for (auto& iter : comparisons_) {
      Double total_expec = 0.0;
      for (auto& comparison : iter.second) total_expec += comparison.expected_;
      for (auto& comparison : iter.second) comparison.expected_ /= total_expec;
    }
    likelihood_->SimulateObserved(comparisons_);
    for (auto& iter : comparisons_) {
      double total = 0.0;
      for (auto& comparison : iter.second) total += comparison.observed_;
      if (simulated_data_sum_to_one_) {
        for (auto& comparison : iter.second) comparison.observed_ /= total;
      }
    }
  } else {
    /**
     * Convert the expected_values in to a proportion
     */
    for (unsigned year : years_) {
      Double running_total = 0.0;
      for (obs::Comparison comparison : comparisons_[year]) {
        running_total += comparison.expected_;
      }
      for (obs::Comparison& comparison : comparisons_[year]) {
        if (running_total != 0.0)
          comparison.expected_ = comparison.expected_ / running_total;
        else
          comparison.expected_ = 0.0;
      }
    }
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

} /* namespace age */
} /* namespace observations */
} /* namespace niwa */
