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
  parameters_.Bind<Double>(PARAM_A, &a_, "The constant value (default a = 0) in ax^b + c")->set_default_value(0.0);
  parameters_.Bind<Double>(PARAM_B, &b_, "The constant value (default b = 0) in ax^b + c")->set_default_value(0.0);
  parameters_.Bind<Double>(PARAM_C, &c_, "The constant value");
  parameters_.Bind<Double>(PARAM_BETA, &beta_, "The minimum age for which the selectivity applies")->set_default_value(0.0);

  RegisterAsAddressable(PARAM_C, &c_);
  allowed_length_based_in_age_based_model_ = false;
}

void Constant::DoValidate() {
  parameters_.Validate(PARAM_A)->GreaterThanOrEqualTo(0.0);
  parameters_.Validate(PARAM_B)->GreaterThanOrEqualTo(0.0);
  parameters_.Validate(PARAM_C)->GreaterThanOrEqualTo(0.0);
  parameters_.Validate(PARAM_BETA)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualToModelMaxAge();
}

/**
 * @brief Get the selectivity value for a given age
 * @param value The age
 * @return The selectivity value
 */
Double Constant::get_value(Double value) {
  if (value < beta_)
    return (0.0);
  return (a_ * pow(value, b_) + c_);
}

/**
 * @brief Get the selectivity value for a given age
 * @param value The age
 * @return The selectivity value
 */
Double Constant::get_value(unsigned value) {
  if (value < beta_)
    return (0.0);
  return (a_ * pow(value, b_) + c_);
}

} /* namespace selectivities */
} /* namespace niwa */
