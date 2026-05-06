/**
 * @file TagRecaptureByAge.cpp
 * @author C.Marsh
 * @github https://github.com/Zaita
 * @date 23/10/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "TagRecaptureByAge.h"

#include <algorithm>

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
TagRecaptureByAge::TagRecaptureByAge(shared_ptr<Model> model) : Observation(model) {
  recaptures_table_ = parameters_.BindTable(PARAM_RECAPTURED, "The table of observed recaptured individuals in each age class");
  recaptures_table_->set_requires_columns(false);
  scanned_table_ = parameters_.BindTable(PARAM_SCANNED, "The table of observed scanned individuals in each age class");
  scanned_table_->set_requires_columns(false);

  // clang-format off
  parameters_.Bind<unsigned>(PARAM_MIN_AGE, &min_age_, "The minimum age");
  parameters_.Bind<unsigned>(PARAM_MAX_AGE, &max_age_, "The maximum age");
  parameters_.Bind<bool>(PARAM_PLUS_GROUP, &plus_group_, "Is the maximum age the age plus group?")->set_default_value(true);
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years for which there are observations")->set_is_optional(true);
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_labels_, "The labels of the selectivities of the scanned categories");
  parameters_.Bind<string>(PARAM_TAGGED_CATEGORIES, &tagged_category_labels_, "The categories of tagged individuals")->flag_is_category(true);
  parameters_.Bind<string>(PARAM_TAGGED_SELECTIVITIES, &tagged_selectivity_labels_, "The labels of the selectivities of the tagged categories");
  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "The label of the time step that the observation occurs in");
  parameters_.Bind<Double>(PARAM_DETECTION_PARAMETER, &detection_, "The probability of detecting a recaptured individual");
  parameters_.Bind<Double>(PARAM_DISPERSION, &dispersion_, "The overdispersion parameter (phi)")->set_is_optional(true);
  parameters_.Bind<Double>(PARAM_TIME_STEP_PROPORTION, &time_step_proportion_, "The proportion through the mortality block of the time step when the observation is evaluated")
    ->set_default_value(0.5);
  parameters_.Bind<Double>(PARAM_OVERLAP_SCALAR, &overlap_scalar_, "Scalar for the relative overlap of the tag recaptures ", "", 1.0)->set_lower_bound(0.0, true);
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
void TagRecaptureByAge::DoValidate() {
  parameters_.Validate(PARAM_MIN_AGE)->IsAge()->LessThanOrEqualToParameter(PARAM_MAX_AGE);
  parameters_.Validate(PARAM_MAX_AGE)->IsAge();
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
  parameters_.ValidateVector(PARAM_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES);
  parameters_.ValidateVector(PARAM_TAGGED_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_TAGGED_CATEGORIES)->SameNumberOfElementsAs(PARAM_TAGGED_CATEGORIES);
  parameters_.Validate(PARAM_DETECTION_PARAMETER)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);
  parameters_.ValidateVector(PARAM_DISPERSION)
      ->GreaterThanOrEqualTo(0.0)
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->DefaultValue(1.0, years_.size());
  parameters_.Validate(PARAM_TIME_STEP_PROPORTION)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);
  parameters_.ValidateVector(PARAM_OVERLAP_SCALAR)
      ->GreaterThanOrEqualTo(0.0)
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->DefaultValue(1.0, years_.size());

  dispersion_by_year_     = utilities::Map::create(years_, dispersion_);
  overlap_scalar_by_year_ = utilities::Map::create(years_, overlap_scalar_);

  age_spread_                    = (max_age_ - min_age_) + 1;
  unsigned expected_column_count = age_spread_ * category_labels_.size() + 1;

  parameters_.ValidateTable(PARAM_RECAPTURED)
      ->Rows(years_.size(), "Number of rows in the recaptured table must match the number of years provided")
      ->Columns(expected_column_count, "Expected year and recaptured values columns in the recaptured table")
      ->ColumnIsYear(0, "First column of the recaptured table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All recaptured except the first must be a double value for the observation")
      ->GreaterThan(expected_column_count - 1, 0.0);

  parameters_.ValidateTable(PARAM_SCANNED)
      ->Rows(years_.size(), "Number of rows in the scanned table must match the number of years provided")
      ->ExpandColumnsTo(expected_column_count - 1, 1u)
      ->Columns(expected_column_count, "Expected year and scanned value columns in the scanned table")
      ->ColumnIsYear(0, "First column of the scanned table must be a model year")
      ->DoubleDataRange(1, expected_column_count - 1, "All columns except the first must be a double value for the observation")
      ->GreaterThanForRange(1, expected_column_count - 1, 0.0);

  recaptures_ = recaptures_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, expected_column_count - 1);
  scanned_    = scanned_table_->MapColumnsToYearAndCategory(category_labels_, 0u, 1u, expected_column_count - 1);
}

/**
 * Build any runtime relationships and ensure that the labels for other objects are valid.
 */
void TagRecaptureByAge::DoBuild() {
  partition_               = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model(), category_labels_));
  cached_partition_        = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model(), category_labels_));
  tagged_partition_        = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model(), tagged_category_labels_));
  tagged_cached_partition_ = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model(), tagged_category_labels_));

  if (ageing_error_label_ != "") {
    LOG_CODE_ERROR() << "ageing error has not been implemented for the tag recapture at age observation";
  }

  age_results_.resize(age_spread_ * category_labels_.size(), 0.0);

  // Build Selectivity pointers
  for (string label : selectivity_labels_) {
    Selectivity* selectivity = model()->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_SELECTIVITIES) << ": Selectivity label " << label << " does not exist.";
    selectivities_.push_back(selectivity);
  }

  if (selectivities_.size() == 1 && category_labels_.size() != 1) {
    auto val_sel = selectivities_[0];
    selectivities_.assign(category_labels_.size(), val_sel);
  }

  for (string label : tagged_selectivity_labels_) {
    auto selectivity = model()->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity) {
      LOG_ERROR_P(PARAM_TAGGED_SELECTIVITIES) << ": Selectivity label " << label << " does not exist.";
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
}

/**
 * This method is called at the start of the targeted time step for this observation.
 *
 * Build the cache for the partition
 * structure to use with any interpolation
 */
void TagRecaptureByAge::PreExecute() {
  cached_partition_->BuildCache();
  tagged_cached_partition_->BuildCache();

  if (cached_partition_->Size() != scanned_[model()->current_year()].size()) {
    LOG_CODE_ERROR() << "cached_partition_->Size() != scanned_[model->current_year()].size()";
  }
  if (partition_->Size() != scanned_[model()->current_year()].size()) {
    LOG_CODE_ERROR() << "partition_->Size() != scanned_[model->current_year()].size()";
  }
}

/**
 * Execute
 */
void TagRecaptureByAge::Execute() {
  LOG_TRACE();
  LOG_FINEST() << "Entering observation " << label_;

  /**
   * Verify our cached partition and partition sizes are correct
   */
  auto cached_partition_iter        = cached_partition_->Begin();
  auto partition_iter               = partition_->Begin();  // vector<vector<partition::Category> >
  auto tagged_cached_partition_iter = tagged_cached_partition_->Begin();
  auto tagged_partition_iter        = tagged_partition_->Begin();  // vector<vector<partition::Category> >

  vector<Double> age_results(age_spread_);
  vector<Double> tagged_age_results(age_spread_);

  /**
   * Loop through the provided categories. Each provided category (combination) will have a list of observations
   * with it. We need to build a vector of proportions for each age using that combination and then
   * compare it to the observations.
   */
  unsigned selectivity_index        = 0;
  unsigned tagged_selectivity_index = 0;
  for (unsigned category_offset = 0; category_offset < category_labels_.size();
       ++category_offset, ++partition_iter, ++cached_partition_iter, ++tagged_partition_iter, ++tagged_cached_partition_iter) {
    Double selectivity_result = 0.0;
    Double start_value        = 0.0;
    Double end_value          = 0.0;
    Double final_value        = 0.0;

    std::fill(age_results.begin(), age_results.end(), 0.0);
    std::fill(tagged_age_results.begin(), tagged_age_results.end(), 0.0);

    std::set<string> selectivity_labels_set;

    /**
     * Loop through the 2 combined categories if they are supplied, building up the
     * age results proportions values.
     */
    auto category_iter        = partition_iter->begin();
    auto cached_category_iter = cached_partition_iter->begin();
    for (; category_iter != partition_iter->end(); ++cached_category_iter, ++category_iter, ++selectivity_index) {
      assert(selectivity_index < selectivities_.size());
      // std::cout << "category_labels_[category_offset]: " << (*category_iter)->name_ << " is using selectivity: " << selectivities_[selectivity_index]->label() << std::endl;

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
        selectivity_labels_set.insert(selectivities_[selectivity_index]->label());
        selectivity_result = selectivities_[selectivity_index]->GetAgeResult(age, (*category_iter)->age_length_);
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

        // Numbers at age from the population
        age_results[age_offset] += final_value * selectivity_result;
        LOG_FINE() << "category1 becomes: " << age_results[age_offset];
      }
    }

    /**
     * Loop through the tagged combined categories building up the
     * tagged age results
     */
    auto tagged_category_iter        = tagged_partition_iter->begin();
    auto tagged_cached_category_iter = tagged_cached_partition_iter->begin();
    // std::cout << "targged_partition_iter->size(): " << tagged_partition_iter->size() << std::endl;
    // std::cout << "tagged_cached_partition_iter->size(): " << tagged_cached_partition_iter->size() << std::endl;
    for (; tagged_category_iter != tagged_partition_iter->end(); ++tagged_cached_category_iter, ++tagged_category_iter, ++tagged_selectivity_index) {
      assert(tagged_selectivity_index < tagged_selectivities_.size());
      // std::cout << "Processing tagged category: " << (*tagged_category_iter)->name_ << std::endl;

      for (unsigned data_offset = 0; data_offset < (*tagged_category_iter)->data_.size(); ++data_offset) {
        // Check and skip ages we don't care about.
        if ((*tagged_category_iter)->min_age_ + data_offset < min_age_)
          continue;
        if ((*tagged_category_iter)->min_age_ + data_offset > max_age_ && !plus_group_)
          break;

        unsigned age_offset = ((*tagged_category_iter)->min_age_ + data_offset) - min_age_;
        unsigned age        = ((*tagged_category_iter)->min_age_ + data_offset);
        if (min_age_ + age_offset > max_age_)
          age_offset = age_spread_ - 1;
        if (age < min_age_)
          continue;
        if (age > max_age_)
          break;

        // std::cout << "tagged_category_labels_[category_offset]: " << (*tagged_category_iter)->name_
        //           << " is using selectivity: " << tagged_selectivities_[tagged_selectivity_index]->label() << std::endl;
        selectivity_labels_set.insert(tagged_selectivities_[tagged_selectivity_index]->label());
        selectivity_result = tagged_selectivities_[tagged_selectivity_index]->GetAgeResult(age, (*tagged_category_iter)->age_length_);
        start_value        = (*tagged_cached_category_iter)->cached_data_[data_offset];
        end_value          = (*tagged_category_iter)->data_[data_offset];
        final_value        = 0.0;

        if (mean_proportion_method_)
          final_value = start_value + ((end_value - start_value) * proportion_of_time_);
        else
          final_value = fabs(start_value - end_value) * proportion_of_time_;

        LOG_FINE() << "----------";
        LOG_FINE() << "Category2:" << (*tagged_category_iter)->name_;
        LOG_FINE() << "age: " << age;
        LOG_FINE() << "selectivity_result: " << selectivity_result;
        LOG_FINE() << "start_value: " << start_value << " / end_value: " << end_value;
        LOG_FINE() << "final_value: " << final_value;
        LOG_FINE() << "final_value * selectivity: " << (Double)(final_value * selectivity_result);
        tagged_age_results[age_offset] += final_value * selectivity_result;
        LOG_FINE() << "category2 becomes: " << tagged_age_results[age_offset];
      }
    }  // for (tagged_category_iter)

    if (age_results.size() != scanned_[model()->current_year()][category_labels_[category_offset]].size())
      LOG_CODE_ERROR() << "expected_values.size(" << age_results.size() << ") != scanned_[category_offset].size("
                       << scanned_[model()->current_year()][category_labels_[category_offset]].size() << ")";

    // save our comparisons so we can use them to generate the score from the likelihoods later
    for (unsigned i = 0; i < age_results.size(); ++i) {
      Double expected = 0.0;
      double observed = 0.0;
      if (age_results[i] != 0.0)
        expected = detection_ * tagged_age_results[i] / (age_results[i] * overlap_scalar_by_year_[model()->current_year()]);
      if (scanned_[model()->current_year()][category_labels_[category_offset]][i] == 0.0)
        observed = 0.0;
      else
        observed = (recaptures_[model()->current_year()][category_labels_[category_offset]][i]) / scanned_[model()->current_year()][category_labels_[category_offset]][i];

      LOG_MEDIUM() << "Comparison for age " << min_age_ + i << ": expected = " << expected << " observed = " << observed
                   << " error = " << scanned_[model()->current_year()][category_labels_[category_offset]][i]
                   << " recaptures = " << recaptures_[model()->current_year()][category_labels_[category_offset]][i];

      // process_error is not used here, and the dispersion is applied to the final likelihood value below
      SaveComparison(tagged_category_labels_[category_offset], selectivity_labels_set, min_age_ + i, 0, expected, observed, 0.0,
                     scanned_[model()->current_year()][category_labels_[category_offset]][i], 0.0, delta_, 0.0);
    }  // for (i)
  }  // for (category_offset)
}

/**
 * This method is called at the end of a model iteration
 * to calculate the score for the observation.
 */
void TagRecaptureByAge::CalculateScore() {
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
