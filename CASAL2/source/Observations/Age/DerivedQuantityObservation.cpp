/**
 * @file DerivedQuantityObservation.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 7/01/2014
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 */

// headers
#include "DerivedQuantityObservation.h"

#include "AgeWeights/Manager.h"
#include "InitialisationPhases/Manager.h"
#include "TimeSteps/Manager.h"
#include "Utilities/Map.h"
#include "Utilities/To.h"

// namespaces
namespace niwa {
namespace observations {
namespace age {

namespace utils = niwa::utilities;

/**
 * Default constructor
 */
DerivedQuantityObservation::DerivedQuantityObservation(shared_ptr<Model> model) : Observation(model) {
  obs_table_ = parameters_.BindTable(PARAM_OBS, "The table of observed values");
  obs_table_->set_requires_columns(false);

  // clang-format off
  parameters_.Bind<string>(PARAM_DERIVED_QUANTITY, &derived_quantity_label_, "The derived quantity that is the reference biomass (i.e., a derived quantity label)", "");
  parameters_.Bind<string>(PARAM_B0_PHASE, &initialisation_phase_label_, "The initialisation phase label that the initial biomass is from", "", "");
  parameters_.Bind<unsigned>(PARAM_YEAR_LAG, &year_lag_, "The lag (years) of the derived_quantity that is used for the calculation of the catch", "", 0)->set_lower_bound(0, true);
  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "The label of the time step that the observation occurs in", "");
  parameters_.Bind<Double>(PARAM_PROCESS_ERROR, &process_error_value_, "The process error", "", Double(0.0))->set_lower_bound(0.0);
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years of the observed values", "");
  // clang-format on

  RegisterAsAddressable(PARAM_PROCESS_ERROR, &process_error_value_);

  allowed_likelihood_types_.push_back(PARAM_NORMAL);
  allowed_likelihood_types_.push_back(PARAM_LOGNORMAL);
  allowed_likelihood_types_.push_back(PARAM_PSEUDO);
}

/**
 * Validate
 */
void DerivedQuantityObservation::DoValidate() {
  LOG_TRACE();

  if (parameters_.Get(PARAM_CATEGORIES)->has_been_defined()) {
    LOG_ERROR_P(PARAM_CATEGORIES) << ": The " << PARAM_CATEGORIES << " parameter is not valid for this observation type. Use the " << PARAM_BIOMASS_INDEX << " parameter instead.";
  }

  // Delta
  if (delta_ < 0.0)
    LOG_ERROR_P(PARAM_DELTA) << ": delta (" << delta_ << ") cannot be less than 0.0";
  if (process_error_value_ < 0.0)
    LOG_ERROR_P(PARAM_PROCESS_ERROR) << ": process_error (" << AS_DOUBLE(process_error_value_) << ") cannot be less than 0.0";

  // Obs
  unsigned                num_obs       = 1;
  unsigned                vals_expected = 1 + num_obs + 1;  // year, observation value(s), error value
  vector<vector<string>>& obs_data      = obs_table_->data();
  if (obs_data.size() != years_.size()) {
    LOG_ERROR_P(PARAM_OBS) << "has " << obs_data.size() << " rows defined, but does not match the number of years provided (number of years = " << years_.size() << ")";
  }

  LOG_MEDIUM() << "Number of categories: " << num_obs << ", number of years: " << years_.size() << ", number of observation columns: " << obs_data.size();

  unsigned         year      = 0;
  double           obs_value = 0;
  double           err_value = 0;
  vector<unsigned> table_years;
  for (vector<string>& obs_data_line : obs_data) {
    if (obs_data_line.size() != vals_expected) {
      LOG_ERROR_P(PARAM_OBS) << "has " << obs_data_line.size() << " values defined, but does not match " << vals_expected;
    }

    if (!utilities::To<unsigned>(obs_data_line.front(), year))
      LOG_ERROR_P(PARAM_OBS) << " value " << obs_data_line.front() << " could not be converted to an unsigned integer. It should be the year for this line";
    if (std::find(years_.begin(), years_.end(), year) == years_.end())
      LOG_ERROR_P(PARAM_OBS) << " year " << year << " is not a valid year for this observation";
    // Check to validate each row specifies a unique year
    if (std::find(table_years.begin(), table_years.end(), year) == table_years.end())
      table_years.push_back(year);
    else
      LOG_ERROR_P(PARAM_OBS) << "the year " << year << " occurs more than once in the table of data. Please check the input file";
    // Input and check table data
    for (unsigned i = 1; i <= num_obs; ++i) {
      if (!utilities::To<double>(obs_data_line[i], obs_value))
        LOG_ERROR_P(PARAM_OBS) << "value " << obs_data_line[i] << " could not be converted to a Double. It should be the observation value for this line";
      if (obs_value <= 0.0)
        LOG_ERROR_P(PARAM_OBS) << ": observation value " << obs_value << " for year " << year << " cannot be less than or equal to 0.0";
      obs_by_year_[year].push_back(obs_value);
    }

    if (!utilities::To<double>(obs_data_line.back(), err_value))
      LOG_ERROR_P(PARAM_OBS) << "value " << obs_data_line.back() << " could not be converted to a Double. It should be the error value for this line";
    if (err_value <= 0.0)
      LOG_ERROR_P(PARAM_OBS) << ": error value " << err_value << " for year " << year << " cannot be less than or equal to 0.0";
    error_values_by_year_[year] = err_value;
  }
}

/**
 * Build
 */
void DerivedQuantityObservation::DoBuild() {
  LOG_TRACE();
  derived_quantity_ = model()->managers()->derived_quantity()->GetDerivedQuantity(derived_quantity_label_);
  if (!derived_quantity_) {
    LOG_ERROR_P(PARAM_DERIVED_QUANTITY) << "The " << PARAM_DERIVED_QUANTITY << " derived_quantity (" << derived_quantity_label_ << ") was not found.";
  }
  if (initialisation_phase_label_ != "") {
    initialisation_phase_ = model()->managers()->initialisation_phase()->GetPhaseIndex(initialisation_phase_label_);
  } else {
    initialisation_phase_ = 0;
  }

  auto time_step = model()->managers()->time_step()->GetTimeStep(time_step_label_);
  if (!time_step) {
    LOG_ERROR_P(PARAM_TIME_STEP) << "Time step label " << time_step_label_ << " was not found.";
  } else {
    for (unsigned year : years_) time_step->SubscribeToBlock(this, year);
  }
  for (auto year : years_) {
    if ((year < model()->start_year()) || (year > model()->final_year()))
      LOG_ERROR_P(PARAM_YEARS) << "Years cannot be less than start_year (" << model()->start_year() << "), or greater than final_year (" << model()->final_year() << ").";
  }

  // check the time_step is after its creation timestep in order to check the year_lag
  if (year_lag_ < 1) {
    string dq_time_step_label = derived_quantity_->time_step();
    auto   dq_time_step       = model()->managers()->time_step()->GetTimeStepIndex(dq_time_step_label);
    auto   this_time_step     = model()->managers()->time_step()->GetTimeStepIndex(time_step_label_);
    if ((this_time_step < dq_time_step) && year_lag_ < 1) {
      LOG_ERROR_P(PARAM_TIME_STEP) << "The time step " << time_step_label_ << " occurs before the time step of the derived quantity (" << dq_time_step_label << ").";
    }
  }
}

/**
 * Reset
 */
void DerivedQuantityObservation::DoReset() {}
/**
 * Pre-execute
 */
void DerivedQuantityObservation::PreExecute() {}

/**
 * Execute
 */
void DerivedQuantityObservation::Execute() {
  LOG_FINEST() << "Entering observation " << label_;
  unsigned current_year = model()->current_year();
  unsigned year         = current_year - year_lag_;

  Double expected    = derived_quantity_->GetValue(year) / derived_quantity_->GetLastValueFromInitialisation(initialisation_phase_);
  double error_value = error_values_by_year_[current_year];

  // Store the values
  SaveComparison(derived_quantity_label_, expected, obs_by_year_[current_year][0], process_error_value_, error_value, 0.0, delta_, 0.0);
}

/**
 * Calculate the score
 */
void DerivedQuantityObservation::CalculateScore() {
  /**
   * Simulate or generate results
   * During simulation mode we'll simulate results for this observation
   */
  LOG_FINEST() << "Calculating neglogLikelihood for observation = " << label_;

  // Check if we have a nuisance q or a free q
  if (model()->run_mode() == RunMode::kSimulation) {
    // Send to be simulated
    likelihood_->SimulateObserved(comparisons_);
  } else {
    /**
     * Convert the expected_values
     */
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
