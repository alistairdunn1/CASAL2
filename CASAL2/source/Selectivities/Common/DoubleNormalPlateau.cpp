/**
 * @file DoubleNormalPlateau.cpp
 * @author  C.Marsh
 * @version 1.0
 * @date 2020
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "DoubleNormalPlateau.h"

#include "../../Model/Model.h"

// Namespaces
namespace niwa {
namespace selectivities {

/**
 * Default constructor
 */
DoubleNormalPlateau::DoubleNormalPlateau(shared_ptr<Model> model) : Selectivity(model) {
  parameters_.Bind<Double>(PARAM_SIGMA_L, &sigma_l_, "The sigma L parameter");
  parameters_.Bind<Double>(PARAM_SIGMA_R, &sigma_r_, "The sigma R parameter");
  parameters_.Bind<Double>(PARAM_A1, &a1_, "The a1 parameter");
  parameters_.Bind<Double>(PARAM_A2, &a2_, "The a2 parameter");
  parameters_.Bind<Double>(PARAM_ALPHA, &alpha_, "The maximum value of the selectivity")->set_default_value(1.0);
  parameters_.Bind<Double>(PARAM_BETA, &beta_, "The minimum age for which the selectivity applies")->set_default_value(0.0);

  RegisterAsAddressable(PARAM_A1, &a1_);
  RegisterAsAddressable(PARAM_A2, &a2_);
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
void DoubleNormalPlateau::DoValidate() {
  parameters_.Validate(PARAM_SIGMA_L)->GreaterThan(0.0);
  parameters_.Validate(PARAM_SIGMA_R)->GreaterThan(0.0);
  parameters_.Validate(PARAM_A1)->GreaterThan(0.0);
  parameters_.Validate(PARAM_A2)->GreaterThan(0.0);
  parameters_.Validate(PARAM_ALPHA)->GreaterThan(0.0);
  parameters_.Validate(PARAM_BETA)->GreaterThanOrEqualTo(0.0);

  if (model_->partition_type() == PartitionType::kAge) {
    parameters_.Validate(PARAM_BETA)->LessThanOrEqualToModelMaxAge();
  }
}

/**
 * The core function
 */
Double DoubleNormalPlateau::get_value(Double value) {
  if (value < beta_)
    return (0.0);

  if (value < a1_)
    return alpha_ * pow(2.0, -((value - a1_) / sigma_l_ * (value - a1_) / sigma_l_));
  else if (value < (a1_ + a2_))
    return alpha_;
  else
    return alpha_ * pow(2.0, -((value - (a1_ + a2_)) / sigma_r_ * (value - (a1_ + a2_)) / sigma_r_));
}

/**
 * The core function
 */
Double DoubleNormalPlateau::get_value(unsigned value) {
  if (value < beta_)
    return (0.0);

  if (value < a1_)
    return alpha_ * pow(2.0, -((value - a1_) / sigma_l_ * (value - a1_) / sigma_l_));
  else if (value < (a1_ + a2_))
    return alpha_;
  else
    return alpha_ * pow(2.0, -((value - (a1_ + a2_)) / sigma_r_ * (value - (a1_ + a2_)) / sigma_r_));
}
} /* namespace selectivities */
} /* namespace niwa */
