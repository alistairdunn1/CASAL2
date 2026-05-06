/**
 * @file TimeStepBiomass.cpp
 * @author C.Marsh
 * @github https://github.com/Craig44
 * @date 2022
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "TimeStep.h"

#include "TimeSteps/Manager.h"

// namespaces
namespace niwa {
namespace observations {
namespace length {

/**
 * Default constructor
 */
TimeStepBiomass::TimeStepBiomass(shared_ptr<Model> model) : observations::length::Biomass(model) {
  parameters_.Bind<Double>(PARAM_TIME_STEP_PROPORTION, &time_step_proportion_, "The proportion through the mortality block of the time step when the observation is evaluated")
      ->set_default_value(0.5);

  mean_proportion_method_ = true;
}

/**
 * This method is called to validate the observation
 * parameters and values.
 */
void TimeStepBiomass::DoValidate() {
  length::Biomass::DoValidate();
  parameters_.Validate(PARAM_TIME_STEP_PROPORTION)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);
}

/**
 * Build
 */
void TimeStepBiomass::DoBuild() {
  length::Biomass::DoBuild();

  if (time_step_proportion_ < 0.0 || time_step_proportion_ > 1.0)
    LOG_ERROR_P(PARAM_TIME_STEP_PROPORTION) << ": time_step_proportion (" << time_step_proportion_ << ") must be between 0.0 and 1.0 inclusive";
  proportion_of_time_ = time_step_proportion_;

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
}

} /* namespace length */
} /* namespace observations */
} /* namespace niwa */
