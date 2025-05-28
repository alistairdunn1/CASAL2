/**
 * @file DoubleExponential.cpp
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
#include "DoubleExponential.h"

#include "../../Model/Model.h"

// Namespaces
namespace niwa {
namespace selectivities {

/**
 * Explicit Constructor
 */
DoubleExponential::DoubleExponential(shared_ptr<Model> model) : Selectivity(model) {
  parameters_.Bind<Double>(PARAM_X0, &x0_, "The X0 parameter");
  parameters_.Bind<Double>(PARAM_X1, &x1_, "The X1 parameter");
  parameters_.Bind<Double>(PARAM_X2, &x2_, "The X2 parameter");
  parameters_.Bind<Double>(PARAM_Y0, &y0_, "The Y0 parameter", "")->set_lower_bound(0.0, false);
  parameters_.Bind<Double>(PARAM_Y1, &y1_, "The Y1 parameter", "")->set_lower_bound(0.0, false);
  parameters_.Bind<Double>(PARAM_Y2, &y2_, "The Y2 parameter", "")->set_lower_bound(0.0, false);
  parameters_.Bind<Double>(PARAM_ALPHA, &alpha_, "The maximum value of the selectivity", "", 1.0)->set_lower_bound(0.0, false);
  parameters_.Bind<Double>(PARAM_BETA, &beta_, "The minimum age for which the selectivity applies", "", 0.0)->set_lower_bound(0.0, true);

  RegisterAsAddressable(PARAM_X0, &x0_);
  RegisterAsAddressable(PARAM_Y0, &y0_);
  RegisterAsAddressable(PARAM_Y1, &y1_);
  RegisterAsAddressable(PARAM_Y2, &y2_);
  RegisterAsAddressable(PARAM_X2, &x2_);

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
void DoubleExponential::DoValidate() {
  parameters_.Validate(PARAM_X0)->GreaterThanOrEqualTo(0.0)->GreaterThanOrEqualToParameter(PARAM_X1)->LessThanOrEqualToParameter(PARAM_X2);
  parameters_.Validate(PARAM_X1)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualToParameter(PARAM_X2);
  parameters_.Validate(PARAM_X2)->GreaterThanOrEqualTo(0.0);
  parameters_.Validate(PARAM_Y0)->GreaterThanOrEqualTo(0.0);
  parameters_.Validate(PARAM_Y1)->GreaterThanOrEqualTo(0.0);
  parameters_.Validate(PARAM_Y2)->GreaterThanOrEqualTo(0.0);
  parameters_.Validate(PARAM_ALPHA)->GreaterThan(0.0);
  parameters_.Validate(PARAM_BETA)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualToModelMaxAge();
}
/**
 * The core function
 */
Double DoubleExponential::get_value(Double value) {
  if (value < beta_)
    return (0.0);

  if (value <= x0_) {
    return alpha_ * y0_ * pow((y1_ / y0_), (value - x0_) / (x1_ - x0_));
  } else if (value > x0_ && value <= x2_) {
    return alpha_ * y0_ * pow((y2_ / y0_), (value - x0_) / (x2_ - x0_));
  } else {
    return y2_;
  }
  return alpha_;
}
/**
 * The core function
 */
Double DoubleExponential::get_value(unsigned value) {
  if (value < beta_)
    return (0.0);

  if (value <= x0_) {
    return alpha_ * y0_ * pow((y1_ / y0_), (value - x0_) / (x1_ - x0_));
  } else if (value > x0_ && value <= x2_) {
    return alpha_ * y0_ * pow((y2_ / y0_), (value - x0_) / (x2_ - x0_));
  } else {
    return y2_;
  }
  return alpha_;
}
} /* namespace selectivities */
} /* namespace niwa */
