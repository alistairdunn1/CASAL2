/**
 * @file LogisticProducing.cpp
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
#include "LogisticProducing.h"

#include "../../Model/Model.h"

// namespaces
namespace niwa {
namespace selectivities {

/**
 * Default constructor
 */
LogisticProducing::LogisticProducing(shared_ptr<Model> model) : Selectivity(model) {
  parameters_.Bind<unsigned>(PARAM_L, &low_, "The low value (L)");
  parameters_.Bind<unsigned>(PARAM_H, &high_, "The high value (H)");
  parameters_.Bind<Double>(PARAM_A50, &a50_, "the a50 parameter");
  parameters_.Bind<Double>(PARAM_ATO95, &ato95_, "The ato95 parameter");
  parameters_.Bind<Double>(PARAM_ALPHA, &alpha_, "The maximum value of the selectivity")->set_default_value(1.0);

  RegisterAsAddressable(PARAM_A50, &a50_);
  RegisterAsAddressable(PARAM_ATO95, &ato95_);
  RegisterAsAddressable(PARAM_ALPHA, &alpha_);
  allowed_length_based_in_age_based_model_ = false;
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
void LogisticProducing::DoValidate() {
  parameters_.Validate(PARAM_L)->GreaterThanOrEqualTo(0u)->LessThanParameter(PARAM_H)->LessThanOrEqualToParameter(PARAM_A50);
  parameters_.Validate(PARAM_H)->GreaterThanOrEqualToParameter(PARAM_A50);
  ;
  parameters_.Validate(PARAM_A50)->GreaterThanOrEqualTo(0.0);
  parameters_.Validate(PARAM_ATO95)->GreaterThan(0.0);
  parameters_.Validate(PARAM_ALPHA)->GreaterThan(0.0);
}

/**
 * The core function
 */
Double LogisticProducing::get_value(Double value) {
  if (value < low_) {
    return 0.0;
  } else if (value >= high_) {
    return alpha_;
  } else if (value == low_) {
    return alpha_ / (1.0 + pow(19.0, (a50_ - value) / ato95_));
  } else {
    Double lambda2 = 1.0 / (1.0 + pow(19.0, (a50_ - (value - 1)) / ato95_));
    if (lambda2 > 0.99999) {
      return alpha_;
    } else {
      Double lambda1 = 1.0 / (1.0 + pow(19.0, (a50_ - value) / ato95_));
      return (lambda1 - lambda2) / (1.0 - lambda2) * alpha_;
    }
  }
  return 1.0;
}
/**
 * The core function
 */
Double LogisticProducing::get_value(unsigned value) {
  if (value < low_) {
    return 0.0;
  } else if (value >= high_) {
    return alpha_;
  } else if (value == low_) {
    return alpha_ / (1.0 + pow(19.0, (a50_ - value) / ato95_));
  } else {
    Double lambda2 = 1.0 / (1.0 + pow(19.0, (a50_ - (value - 1)) / ato95_));
    if (lambda2 > 0.99999) {
      return alpha_;
    } else {
      Double lambda1 = 1.0 / (1.0 + pow(19.0, (a50_ - value) / ato95_));
      return (lambda1 - lambda2) / (1.0 - lambda2) * alpha_;
    }
  }
  return 1.0;
}
} /* namespace selectivities */
} /* namespace niwa */
