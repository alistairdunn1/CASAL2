/**
 * @file DoubleNormal.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 14/01/2013
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "DoubleNormal.h"

#include <cmath>

#include "../../Model/Model.h"

// Namespaces
namespace niwa {
namespace selectivities {

/**
 * Default constructor
 */
DoubleNormal::DoubleNormal(shared_ptr<Model> model) : Selectivity(model) {
  parameters_.Bind<Double>(PARAM_MU, &mu_, "The mean (mu)");
  parameters_.Bind<Double>(PARAM_SIGMA_L, &sigma_l_, "The sigma L parameter");
  parameters_.Bind<Double>(PARAM_SIGMA_R, &sigma_r_, "The sigma R parameter");
  parameters_.Bind<Double>(PARAM_ALPHA, &alpha_, "The maximum value of the selectivity")->set_default_value(1.0);
  parameters_.Bind<Double>(PARAM_BETA, &beta_, "The minimum age for which the selectivity applies")->set_default_value(0.0);

  RegisterAsAddressable(PARAM_MU, &mu_);
  RegisterAsAddressable(PARAM_SIGMA_L, &sigma_l_);
  RegisterAsAddressable(PARAM_SIGMA_R, &sigma_r_);
  RegisterAsAddressable(PARAM_ALPHA, &alpha_);
  allowed_length_based_in_age_based_model_ = true;
}

/**
 * Validate this selectivity. This will load the
 * values that were passed in from the configuration
 * file and assign them to the local variables.
 *
 * Then do some basic checks on the local
 * variables to ensure they are within the business
 * rules for the model.
 */
void DoubleNormal::DoValidate() {
  parameters_.Validate(PARAM_MU)->GreaterThanOrEqualTo(0.0);
  parameters_.Validate(PARAM_SIGMA_L)->GreaterThan(0.0);
  parameters_.Validate(PARAM_SIGMA_R)->GreaterThan(0.0);
  parameters_.Validate(PARAM_ALPHA)->GreaterThan(0.0);
  parameters_.Validate(PARAM_BETA)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualToModelMaxAge();
}
/**
 * The core function
 */
Double DoubleNormal::get_value(Double value) {
  if (value < beta_)
    return (0.0);

  if (value < mu_) {
    return pow(2.0, -((value - mu_) / sigma_l_ * (value - mu_) / sigma_l_)) * alpha_;
  } else {
    return pow(2.0, -((value - mu_) / sigma_r_ * (value - mu_) / sigma_r_)) * alpha_;
  }
  return alpha_;
}
/**
 * The core function
 */
Double DoubleNormal::get_value(unsigned value) {
  if (value < beta_)
    return (0.0);

  if (value < mu_) {
    return pow(2.0, -((value - mu_) / sigma_l_ * (value - mu_) / sigma_l_)) * alpha_;
  } else {
    return pow(2.0, -((value - mu_) / sigma_r_ * (value - mu_) / sigma_r_)) * alpha_;
  }
  return alpha_;
}
} /* namespace selectivities */
} /* namespace niwa */
