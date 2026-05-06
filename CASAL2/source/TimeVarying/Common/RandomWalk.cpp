/**
 * @file RandomWalk.cpp
 * @author Craig Marsh
 * @github https://github.com/Zaita
 * @date 2/02/2016
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "RandomWalk.h"

#include "../../Estimates/Manager.h"
#include "../../Model/Objects.h"
#include "../../Utilities/Map.h"
#include "../../Utilities/RandomNumberGenerator.h"

// namespaces
namespace niwa::timevarying {

/**
 * Default constructor
 */
RandomWalk::RandomWalk(shared_ptr<Model> model) : TimeVarying(model) {
  // clang-format off
  parameters_.Bind<Double>(PARAM_MEAN, &mu_, "The mean (mu) of the random walk distribution")
    ->set_default_value(0.0);
  parameters_.Bind<Double>(PARAM_SIGMA, &sigma_, "The standard deviation (sigma)  of the random walk distribution")
    ->set_default_value(1.0);
  parameters_.Bind<Double>(PARAM_LOWER_BOUND, &lower_bound_, "The lower bound for the random walk");
  parameters_.Bind<Double>(PARAM_UPPER_BOUND, &upper_bound_, "The upper bound for the random walk");
  parameters_.Bind<Double>(PARAM_RHO, &rho_, "The autocorrelation parameter (rho)  of the random walk distribution")
    ->set_default_value(1.0);
  parameters_.Bind<string>(PARAM_DISTRIBUTION, &distribution_, "The distribution type")
    ->set_default_value(PARAM_NORMAL);
  // clang-format on

  RegisterAsAddressable(PARAM_MEAN, &mu_);
  RegisterAsAddressable(PARAM_SIGMA, &sigma_);
  RegisterAsAddressable(PARAM_SIGMA, &rho_);
}

/**
 * Validate
 */
void RandomWalk::DoValidate() {
  LOG_CODE_ERROR() << "TimeVarying.Random_Walk has been disabled due to a software bug. Please contact the developers if you require this functionality";

  parameters_.Validate(PARAM_SIGMA)->GreaterThan(0.0);
  parameters_.Validate(PARAM_LOWER_BOUND)->LessThanParameter(PARAM_UPPER_BOUND);
  parameters_.Validate(PARAM_DISTRIBUTION)->IsInList({PARAM_NORMAL});
}

/**
 *
 */
void RandomWalk::DoBuild() {
  auto current_model = model();
  // Warn users that they have a time-varying parameter in estimation mode.
  if (current_model->run_mode() == RunMode::kEstimation) {
    LOG_WARNING() << "Time varying of type " << type_ << " was found during estimation. Note that this is NOT recommended as it is not a true random effect."
                  << " The purpose of this is for investigating model behaviour in simulations or projections";
  }
  if (current_model->objects().GetAddressableType(parameter_) != addressable::kSingle)
    LOG_ERROR_P(PARAM_TYPE) << "@time_varying blocks of type " << PARAM_RANDOMWALK << " can be used only with parameters that are scalars or single values";
}

/**
 * Update
 */
void RandomWalk::DoUpdate() {
  auto current_model = model();
  LOG_FINEST() << "value = " << *addressable_;
  utilities::RandomNumberGenerator& rng     = utilities::RandomNumberGenerator::Instance();
  Double                            value   = *addressable_;
  double                            deviate = rng.normal(AS_DOUBLE(mu_), AS_DOUBLE(sigma_));
  cout << "year: " << current_model->current_year() << "; value: " << value << "; deviate: " << deviate << endl;
  value += value * rho_ + deviate;

  if (value < lower_bound_) {
    LOG_FINEST() << "@estimate at lower bound, changing value from " << value << " to " << lower_bound_;
    value = lower_bound_;
  }
  if (value > upper_bound_) {
    LOG_FINEST() << "@estimate at upper bound, changing value from " << value << " to " << upper_bound_;
    value = upper_bound_;
  }

  LOG_FINEST() << "value after deviate of " << deviate << " = " << value << " for year " << current_model->current_year();
  cout << "setting value to " << value;

  LOG_FINE() << "Setting Value to: " << value;
  parameter_by_year_[current_model->current_year()] = value;
  (this->*update_function_)(value);
}

}  // namespace niwa::timevarying
