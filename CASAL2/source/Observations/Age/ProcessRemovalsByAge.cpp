/**
 * @file ProcessRemovalsByAge.cpp
 * @author  C Marsh
 * @version 1.0
 * @date 25/08/15
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 */

// Headers
#include "ProcessRemovalsByAge.h"

#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim_all.hpp>

#include "AgeingErrors/Manager.h"
#include "Categories/Categories.h"
#include "Model/Model.h"
#include "Partition/Accessors/All.h"
#include "Processes/Manager.h"
#include "TimeSteps/Manager.h"
#include "Utilities/Map.h"
#include "Utilities/Math.h"
#include "Utilities/To.h"
#include "Utilities/Vector.h"

// Namespaces
namespace niwa::observations::age {

/**
 * Default constructor
 */
ProcessRemovalsByAge::ProcessRemovalsByAge(shared_ptr<Model> model) : Observation(model) {
  obs_table_          = new parameters::Table(PARAM_OBS);
  error_values_table_ = new parameters::Table(PARAM_ERROR_VALUES);
  parameters_.BindTable(PARAM_OBS, obs_table_, "The table of observed values", "", false);
  parameters_.BindTable(PARAM_ERROR_VALUES, error_values_table_, "The table of error values of the observed values (note that the units depend on the likelihood)", "", false);

  parameters_.Bind<unsigned>(PARAM_MIN_AGE, &min_age_, "The minimum age");
  parameters_.Bind<unsigned>(PARAM_MAX_AGE, &max_age_, "The maximum age");
  parameters_.Bind<bool>(PARAM_PLUS_GROUP, &plus_group_, "Is the maximum age the age plus group")->set_default_value(true);
  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "The label of time-steps that the observation occurs in");
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years for which there are observations")->set_is_optional(true);
  parameters_.Bind<Double>(PARAM_PROCESS_ERRORS, &process_error_values_, "The label of process error to use")->set_is_optional(true);
  parameters_.Bind<string>(PARAM_AGEING_ERROR, &ageing_error_label_, "The label of the ageing error to use")->set_default_value("");
  parameters_.Bind<string>(PARAM_METHOD_OF_REMOVAL, &method_, "The label of the observed method of removals")->set_default_value("");
  parameters_.Bind<string>(PARAM_MORTALITY_PROCESS, &process_label_, "The label of the mortality process for the observation");
  parameters_.Bind<bool>(PARAM_SIMULATED_DATA_SUM_TO_ONE, &simulated_data_sum_to_one_, "Whether simulated data is discrete or scaled by totals to be proportions for each year")
      ->set_default_value(true);
  parameters_.Bind<bool>(PARAM_SUM_TO_ONE, &sum_to_one_, "Scale year (row) observed values by the total, so they sum = 1")->set_default_value(false);

  mean_proportion_method_ = false;

  RegisterAsAddressable(PARAM_PROCESS_ERRORS, &process_error_values_);

  allowed_likelihood_types_ = {PARAM_LOGNORMAL, PARAM_MULTINOMIAL, PARAM_DIRICHLET, PARAM_DIRICHLET_MULTINOMIAL, PARAM_LOGISTIC_NORMAL};
  allowed_mortality_types_  = {PARAM_MORTALITY_INSTANTANEOUS, PARAM_MORTALITY_HYBRID};
}

/**
 * Destructor
 */
ProcessRemovalsByAge::~ProcessRemovalsByAge() {
  delete obs_table_;
  delete error_values_table_;
}

/**
 * Validate configuration file parameters
 */
void ProcessRemovalsByAge::DoValidate() {
  parameters_.Validate(PARAM_MIN_AGE)->IsAge();
  parameters_.Validate(PARAM_MAX_AGE)->IsAge();
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
  parameters_.ValidateVector(PARAM_PROCESS_ERRORS)->GreaterThanOrEqualTo(0.0)->ExpandToSameNumberOfElementsAs(PARAM_YEARS)->SameNumberOfElementsAs(PARAM_YEARS);
  parameters_.ValidateVector(PARAM_METHOD_OF_REMOVAL)->SameNumberOfElementsAs(PARAM_TIME_STEP);

  if (process_error_values_.size() == 0)
    process_error_values_.assign(years_.size(), 0.0);
  process_errors_by_year_ = utilities::Map::create(years_, process_error_values_);

  age_spread_ = (max_age_ - min_age_) + 1;

  map<unsigned, vector<double>> error_values_by_year;
  map<unsigned, vector<double>> obs_by_year;

  /**
   * Do some simple checks
   */

  // if only one value supplied then assume its the same for all years
  if (process_error_values_.size() == 1) {
    Double temp = process_error_values_[0];
    process_error_values_.resize(years_.size(), temp);
  }

  /**
   * Validate the number of obs provided matches age spread * category_labels * years
   * This is because we'll have 1 set of obs per category collection provided.
   * categories male+female male = 2 collections
   */
  unsigned                obs_expected = age_spread_ * category_labels_.size() + 1;
  vector<vector<string>>& obs_data     = obs_table_->data();
  if (obs_data.size() != years_.size()) {
    LOG_ERROR_P(PARAM_OBS) << " has " << obs_data.size() << " rows defined, but " << years_.size() << " should match the number of years provided";
  }

  for (vector<string>& obs_data_line : obs_data) {
    if (obs_data_line.size() != obs_expected) {
      LOG_ERROR_P(PARAM_OBS) << " has " << obs_data_line.size() << " values defined, but " << obs_expected << " should match the age spread * categories + 1 (for year)";
    }

    unsigned year = 0;
    if (!utilities::To<unsigned>(obs_data_line[0], year))
      LOG_ERROR_P(PARAM_OBS) << " value " << obs_data_line[0] << " could not be converted to an unsigned integer. It should be the year for this line";
    if (std::find(years_.begin(), years_.end(), year) == years_.end())
      LOG_ERROR_P(PARAM_OBS) << " value " << year << " is not a valid year for this observation";

    for (unsigned i = 1; i < obs_data_line.size(); ++i) {
      double value = 0.0;
      if (!utilities::To<double>(obs_data_line[i], value))
        LOG_ERROR_P(PARAM_OBS) << " value (" << obs_data_line[i] << ") could not be converted to a Double";
      obs_by_year[year].push_back(value);
    }
    if (obs_by_year[year].size() != obs_expected - 1)
      LOG_FATAL_P(PARAM_OBS) << " " << obs_by_year[year].size() << "ages were supplied, but " << obs_expected - 1 << " ages are required";
  }

  /**
   * Build our error value map
   */
  vector<vector<string>>& error_values_data = error_values_table_->data();
  if (error_values_data.size() != years_.size()) {
    LOG_ERROR_P(PARAM_ERROR_VALUES) << " has " << error_values_data.size() << " rows defined, but " << years_.size() << " should match the number of years provided";
  }

  for (vector<string>& error_values_data_line : error_values_data) {
    if (error_values_data_line.size() != 2 && error_values_data_line.size() != obs_expected) {
      LOG_FATAL_P(PARAM_VALUES) << " has " << error_values_data_line.size() << " values defined, but " << obs_expected
                                << " should match the age spread * categories + 1 (for year)";
    }

    unsigned year = 0;
    if (!utilities::To<unsigned>(error_values_data_line[0], year))
      LOG_ERROR_P(PARAM_ERROR_VALUES) << " value " << error_values_data_line[0] << " could not be converted to an unsigned integer. It should be the year for this line";
    if (std::find(years_.begin(), years_.end(), year) == years_.end())
      LOG_ERROR_P(PARAM_ERROR_VALUES) << " value " << year << " is not a valid year for this observation";
    for (unsigned i = 1; i < error_values_data_line.size(); ++i) {
      double value = 0;
      if (!utilities::To<double>(error_values_data_line[i], value))
        LOG_FATAL_P(PARAM_ERROR_VALUES) << "value (" << error_values_data_line[i] << ") could not be converted to a Double";
      if (likelihood_type_ == PARAM_LOGNORMAL && value <= 0.0) {
        LOG_ERROR_P(PARAM_ERROR_VALUES) << ": error_value (" << value << ") cannot be equal to or less than 0.0";
      } else if (likelihood_type_ == PARAM_MULTINOMIAL && value < 0.0) {
        LOG_ERROR_P(PARAM_ERROR_VALUES) << ": error_value (" << value << ") cannot be less than 0.0";
      }

      error_values_by_year[year].push_back(value);
    }

    if (error_values_by_year[year].size() == 1) {
      auto val_e = error_values_by_year[year][0];
      error_values_by_year[year].assign(obs_expected - 1, val_e);
    }

    if (error_values_by_year[year].size() != obs_expected - 1)
      LOG_FATAL_P(PARAM_ERROR_VALUES) << " " << error_values_by_year[year].size() << " error values by year were supplied, but " << obs_expected - 1
                                      << " values are required based on the obs table";
  }

  /**
   * Build our proportions and error values for use in the observation
   * If the proportions for a given observation do not sum to 1.0
   * and is off by more than the tolerance rescale them.
   */
  double value = 0.0;
  for (auto iter = obs_by_year.begin(); iter != obs_by_year.end(); ++iter) {
    double total = 0.0;
    for (unsigned i = 0; i < category_labels_.size(); ++i) {
      for (unsigned j = 0; j < age_spread_; ++j) {
        auto e_f = error_values_by_year.find(iter->first);
        if (e_f != error_values_by_year.end()) {
          unsigned obs_index = i * age_spread_ + j;
          value              = iter->second[obs_index];
          error_values_[iter->first][category_labels_[i]].push_back(e_f->second[obs_index]);
          // if not rescaling add the data
          if (!sum_to_one_)
            proportions_[iter->first][category_labels_[i]].push_back(value);
          total += value;
        }
      }
    }
    // rescale the year obs so sum = 1
    if (sum_to_one_) {
      for (unsigned i = 0; i < category_labels_.size(); ++i) {
        for (unsigned j = 0; j < age_spread_; ++j) {
          unsigned obs_index = i * age_spread_ + j;
          value              = iter->second[obs_index];
          proportions_[iter->first][category_labels_[i]].push_back(value / total);
        }
      }
    } else {
      if (!utilities::math::IsOne(total)) {
        LOG_WARNING_P(PARAM_OBS) << ": The sum of the values for year " << iter->first << " was " << total << " and do not sum to 1.0";
      }
    }
  }
}

/**
 * Build any runtime relationships and ensure that the labels for other objects are valid.
 */
void ProcessRemovalsByAge::DoBuild() {
  partition_ = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model_, category_labels_));

  // Create a pointer to misclassification matrix
  if (ageing_error_label_ != "") {
    ageing_error_ = model_->managers()->ageing_error()->GetAgeingError(ageing_error_label_);
    if (!ageing_error_)
      LOG_ERROR_P(PARAM_AGEING_ERROR) << "Ageing error label (" << ageing_error_label_ << ") was not found.";
  } else {
    LOG_ERROR() << "An age-based observation with no ageing error type was provided";
  }

  age_results_.resize(age_spread_ * category_labels_.size(), 0.0);

  for (string time_label : time_step_label_) {
    auto time_step = model_->managers()->time_step()->GetTimeStep(time_label);
    if (!time_step) {
      LOG_FATAL_P(PARAM_TIME_STEP) << "Time step label " << time_label << " was not found.";
    } else {
      auto process = time_step->SubscribeToProcess(this, years_, process_label_);
      if (!process)
        LOG_FATAL_P(PARAM_MORTALITY_PROCESS) << "could not find process " << process_label_;
      mortality_process_ = model_->managers()->process()->GetAgeBasedMortalityProcess(process_label_);
    }
  }

  if (mortality_process_ == nullptr) {
    LOG_FATAL_P(PARAM_MORTALITY_PROCESS) << "Could not find process " << process_label_ << ".";
  } else {
    if (find(allowed_mortality_types_.begin(), allowed_mortality_types_.end(), mortality_process_->type()) == allowed_mortality_types_.end())
      LOG_FATAL_P(PARAM_MORTALITY_PROCESS) << "The mortality process is of type " << mortality_process_->type() << " is not allowed for this observation.";
  }
  // Need to split the categories if any are combined for checking
  vector<string> temp_split_category_labels, split_category_labels;

  for (const string& category_label : category_labels_) {
    boost::split(temp_split_category_labels, category_label, boost::is_any_of("+"));
    for (const string& split_category_label : temp_split_category_labels) {
      split_category_labels.push_back(split_category_label);
    }
  }
  for (auto category : split_category_labels) LOG_FINEST() << category;

  // Do some checks so that the observation and process are compatible
  if (!mortality_process_->check_methods_for_removal_obs(method_))
    LOG_FATAL_P(PARAM_METHOD_OF_REMOVAL) << "could not find all these methods in the instantaneous_mortality process labeled " << process_label_
                                         << ". Check that the methods are compatible with this process";
  if (!mortality_process_->check_categories_in_methods_for_removal_obs(method_, split_category_labels))
    LOG_FATAL_P(PARAM_CATEGORIES) << "could not find all these categories in the instantaneous_mortality process labeled " << process_label_
                                  << ". Check that the categories are compatible with this process";
  if (!mortality_process_->check_years_in_methods_for_removal_obs(years_, method_))
    LOG_FATAL_P(PARAM_YEARS) << "could not find catches with catch in all years in the instantaneous_mortality process labeled " << process_label_
                             << ". Check that the years are compatible with this process";
  fishery_ndx_to_get_catch_at_info_ = mortality_process_->get_fishery_ndx_for_catch_at(method_);
  LOG_FINE() << "fishery ndx = ";
  for (auto fishndx : fishery_ndx_to_get_catch_at_info_) LOG_FINE() << fishndx;
  vector<unsigned> category_ndxs = mortality_process_->get_category_ndx_for_catch_at(split_category_labels);
  for (unsigned category_ndx = 0; category_ndx < split_category_labels.size(); ++category_ndx) {
    category_lookup_for_ndx_to_get_catch_at_info_[split_category_labels[category_ndx]] = category_ndxs[category_ndx];
    LOG_FINE() << "cat " << split_category_labels[category_ndx] << " ndx = " << category_ndxs[category_ndx];
  }
  year_ndx_to_get_catch_at_info_ = mortality_process_->get_year_ndx_for_catch_at(years_);
  LOG_FINE() << "year ndx = ";
  for (auto yearndx : year_ndx_to_get_catch_at_info_) LOG_FINE() << yearndx;

  // If this observation is made up of multiple methods lets find out the last one, because that is when we execute the process
  vector<unsigned> time_step_index;
  for (string label : time_step_label_) time_step_index.push_back(model_->managers()->time_step()->GetTimeStepIndex(label));

  unsigned last_method_time_step = 9999;
  if (time_step_index.size() > 1) {
    for (unsigned i = 0; i < (time_step_index.size() - 1); ++i) {
      if (time_step_index[i] > time_step_index[i + 1])
        last_method_time_step = time_step_index[i];
      else
        last_method_time_step = time_step_index[i + 1];
    }
    time_step_to_execute_ = last_method_time_step;
  } else {
    time_step_to_execute_ = time_step_index[0];
  }

  expected_values_.resize(age_spread_, 0.0);
  accumulated_expected_values_.resize(age_spread_, 0.0);
  numbers_at_age_with_ageing_error_.resize(model_->age_spread(), 0.0);
}

/**
 * This method is called at the start of the targeted
 * time step for this observation.
 *
 * Build the cache for the partition
 * structure to use with any interpolation
 */
void ProcessRemovalsByAge::PreExecute() {}

/**
 * Execute
 */
void ProcessRemovalsByAge::Execute() {
  LOG_FINEST() << "Entering observation " << label_;
  unsigned             year           = model_->current_year();
  std::pair<bool, int> this_year_iter = utilities::findInVector(years_, year);
  if (!this_year_iter.first)
    LOG_CODE_ERROR() << "if(!this_year_iter.first)";
  LOG_FINEST() << "current year " << year;
  // Check if we are in the final time_step so we have all the relevent information from the Mortaltiy process
  unsigned current_time_step = model_->managers()->time_step()->current_time_step();
  if (time_step_to_execute_ == current_time_step) {
    auto partition_iter = partition_->Begin();  // vector<vector<partition::Category> >
    for (unsigned category_offset = 0; category_offset < category_labels_.size(); ++category_offset, ++partition_iter) {
      fill(accumulated_expected_values_.begin(), accumulated_expected_values_.end(), 0.0);
      fill(expected_values_.begin(), expected_values_.end(), 0.0);
      LOG_FINEST() << "Category = " << category_labels_[category_offset];
      auto category_iter = partition_iter->begin();
      for (; category_iter != partition_iter->end(); ++category_iter) {
        // Go through all the fisheries and accumulate the expectation whilst also applying ageing error
        unsigned method_offset = 0;
        for (string fishery : method_) {
          fill(numbers_at_age_with_ageing_error_.begin(), numbers_at_age_with_ageing_error_.end(), 0.0);
          LOG_FINEST() << "year ndx = " << year_ndx_to_get_catch_at_info_[this_year_iter.second] << " fishery ndx = " << fishery_ndx_to_get_catch_at_info_[method_offset]
                       << " category ndx = " << category_lookup_for_ndx_to_get_catch_at_info_[(*category_iter)->name_];
          vector<Double>& Removals_at_age
              = mortality_process_->get_catch_at_by_year_fishery_category(year_ndx_to_get_catch_at_info_[this_year_iter.second], fishery_ndx_to_get_catch_at_info_[method_offset],
                                                                          category_lookup_for_ndx_to_get_catch_at_info_[(*category_iter)->name_]);
          LOG_FINE() << "Nages = " << Removals_at_age.size();
          /*
           *  Apply Ageing error on Removals at age vector
           */
          if (ageing_error_label_ != "") {
            vector<vector<Double>>& mis_matrix = ageing_error_->mis_matrix();
            for (unsigned i = 0; i < mis_matrix.size(); ++i) {
              for (unsigned j = 0; j < mis_matrix[i].size(); ++j) {
                numbers_at_age_with_ageing_error_[j] += Removals_at_age[i] * mis_matrix[i][j];
              }
            }
          } else {
            for (unsigned i = 0; i < numbers_at_age_with_ageing_error_.size(); ++i) numbers_at_age_with_ageing_error_[i] = Removals_at_age[i];
          }
          LOG_TRACE();

          /*
           *  Now collapse the number_age into the expected_values for the observation
           */
          for (unsigned k = 0; k < numbers_at_age_with_ageing_error_.size(); ++k) {
            LOG_FINE() << "----------";
            LOG_FINE() << "Fishery: " << fishery;
            LOG_FINE() << "Numbers At Age After Ageing error: " << (*category_iter)->min_age_ + k << " for category " << (*category_iter)->name_ << " "
                       << numbers_at_age_with_ageing_error_[k];

            unsigned age_offset = min_age_ - model_->min_age();
            if (k >= age_offset && (k - age_offset + min_age_) <= max_age_)
              expected_values_[k - age_offset] = numbers_at_age_with_ageing_error_[k];
            // Deal with the plus group
            if (((k - age_offset + min_age_) > max_age_) && plus_group_)
              expected_values_[age_spread_ - 1] += numbers_at_age_with_ageing_error_[k];
          }

          if (expected_values_.size() != proportions_[model_->current_year()][category_labels_[category_offset]].size())
            LOG_CODE_ERROR() << "expected_values_.size(" << expected_values_.size() << ") != proportions_[category_offset].size("
                             << proportions_[model_->current_year()][category_labels_[category_offset]].size() << ")";

          // Accumulate the expectations if they come form multiple fisheries
          for (unsigned i = 0; i < expected_values_.size(); ++i) accumulated_expected_values_[i] += expected_values_[i];

          method_offset++;
        }
      }

      /**
       * save our comparisons so we can use them to generate the score from the likelihoods later
       */
      for (unsigned i = 0; i < accumulated_expected_values_.size(); ++i) {
        LOG_FINEST() << "-----";
        LOG_FINEST() << "Numbers at age for category: " << category_labels_[category_offset] << " for age " << min_age_ + i << " = " << accumulated_expected_values_[i];
        SaveComparison(category_labels_[category_offset], min_age_ + i, 0.0, accumulated_expected_values_[i],
                       proportions_[model_->current_year()][category_labels_[category_offset]][i], process_errors_by_year_[model_->current_year()],
                       error_values_[model_->current_year()][category_labels_[category_offset]][i], 0.0, delta_, 0.0);
      }
    }
  }
}

/**
 * This method is called at the end of a model iteration
 * to calculate the score for the observation.
 */
void ProcessRemovalsByAge::CalculateScore() {
  /**
   * Simulate or generate results
   * During simulation mode we'll simulate results for this observation
   */
  LOG_FINEST() << "Calculating neglogLikelihood for observation = " << label_;

  if (model_->run_mode() == RunMode::kSimulation) {
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

}  // namespace niwa::observations::age
