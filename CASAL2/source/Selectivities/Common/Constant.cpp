/*
 * Constant.cpp
 *
 *  Created on: 9/01/2013
 *      Author: Admin
 */
#include "Constant.h"

#include <boost/math/distributions/lognormal.hpp>
#include <cmath>

#include "../../AgeLengths/AgeLength.h"
#include "../../Model/Model.h"

namespace niwa {
namespace selectivities {

/**
 * Default constructor
 */
Constant::Constant(shared_ptr<Model> model) : Selectivity(model) {
  parameters_.Bind<Double>(PARAM_A, &a_, "The constant value (default a = 0) in ax^b + c", "", 0.0);
  parameters_.Bind<Double>(PARAM_B, &b_, "The constant value (default b = 0) in ax^b + c", "", 0.0);
  parameters_.Bind<Double>(PARAM_C, &c_, "The constant value (default c = 1) in ax^b + c", "", 1.0);
  parameters_.Bind<Double>(PARAM_BETA, &beta_, "The minimum age for which the selectivity applies", "", 0.0)->set_lower_bound(0.0, true);

  RegisterAsAddressable(PARAM_A, &a_);
  RegisterAsAddressable(PARAM_B, &b_);
  RegisterAsAddressable(PARAM_C, &c_);

  allowed_length_based_in_age_based_model_ = true;
}

void Constant::DoValidate() {
  if (beta_ > model_->max_age())
    LOG_ERROR_P(PARAM_BETA) << ": beta (" << AS_DOUBLE(beta_) << ") cannot be greater than the model maximum age";
}

Double Constant::get_value(Double value) {
  if (value < beta_)
    return (0.0);

  return (a_ * pow(value, b_) + c_);
}

Double Constant::get_value(unsigned value) {
  if (value < beta_)
    return (0.0);

  return (a_ * pow(value, b_) + c_);
}

} /* namespace selectivities */
} /* namespace niwa */
