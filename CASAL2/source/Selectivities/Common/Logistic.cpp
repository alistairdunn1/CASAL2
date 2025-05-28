/**
 * @file Logistic.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 15/01/2013
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "Logistic.h"

#include "../../Model/Model.h"

// Namespaces
namespace niwa {
namespace selectivities {

/**
 * Default Constructor
 */
Logistic::Logistic(shared_ptr<Model> model) : Selectivity(model) {
  parameters_.Bind<Double>(PARAM_A50, &a50_, "The a50 parameter");
  parameters_.Bind<Double>(PARAM_ATO95, &ato95_, "The ato95 parameter");
  parameters_.Bind<Double>(PARAM_ALPHA, &alpha_, "The maximum value of the selectivity")->set_default_value(1.0);
  parameters_.Bind<Double>(PARAM_BETA, &beta_, "The minimum age for which the selectivity applies")->set_default_value(0.0);

  RegisterAsAddressable(PARAM_A50, &a50_);
  RegisterAsAddressable(PARAM_ATO95, &ato95_);
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
void Logistic::DoValidate() {
  parameters_.Validate(PARAM_A50)->GreaterThanOrEqualTo(0.0);
  parameters_.Validate(PARAM_ATO95)->GreaterThan(0.0);
  parameters_.Validate(PARAM_ALPHA)->GreaterThan(0.0);
  parameters_.Validate(PARAM_BETA)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualToModelMaxAge();
}

/**
 * The core function
 */
Double Logistic::get_value(Double value) {
  if (value < beta_)
    return (0.0);

  Double threshold = (a50_ - value) / ato95_;
  if (threshold > 5.0)
    return 0.0;
  else if (threshold < -5.0)
    return alpha_;
  else
    return alpha_ / (1.0 + pow(19.0, threshold));
  return alpha_;
}

/**
 * The core function
 */
Double Logistic::get_value(unsigned value) {
  if (value < beta_)
    return (0.0);

  Double threshold = (a50_ - value) / ato95_;
  if (threshold > 5.0)
    return 0.0;
  else if (threshold < -5.0)
    return alpha_;
  else
    return alpha_ / (1.0 + pow(19.0, threshold));
  return alpha_;
}
} /* namespace selectivities */
} /* namespace niwa */
