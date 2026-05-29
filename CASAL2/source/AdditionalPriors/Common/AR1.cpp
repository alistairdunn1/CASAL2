/**
 * @file AR1.cpp
 * @author Casal2
 * @date 2026/05/29
 * @section LICENSE
 *
 * Copyright Casal2 Project 2026 - https://github.com/Casal2/
 *
 */

// headers
#include "AR1.h"

#include <cmath>

#include "../../Model/Model.h"
#include "../../Model/Objects.h"

// namespaces
namespace niwa {
namespace additionalpriors {

/**
 * Default constructor
 */
AR1::AR1(shared_ptr<Model> model) : AdditionalPrior(model) {
  parameters_.Bind<string>(PARAM_PARAMETER, &parameter_, "The name of the parameter for the additional prior", "");
  parameters_.Bind<Double>(PARAM_RHO, &rho_, "The AR1 autocorrelation parameter (rho)")->set_default_value(0.0);
  parameters_.Bind<Double>(PARAM_SIGMA, &sigma_, "The innovation standard deviation (sigma)")->set_default_value(1.0);
  parameters_.Bind<Double>(PARAM_MEAN, &mean_, "The process mean")->set_default_value(0.0);
  parameters_.Bind<Double>(PARAM_MULTIPLIER, &multiplier_, "Multiply the AR1 penalty by this factor")->set_default_value(1.0);
  parameters_.Bind<bool>(PARAM_LOG_SCALE, &log_scale_, "Should the AR1 be applied in log space?")->set_default_value(false);
}

/**
 * Validate parameter values
 */
void AR1::DoValidate() {
  parameters_.Validate(PARAM_SIGMA)->GreaterThan(0.0);
  parameters_.Validate(PARAM_RHO)->GreaterThan(-1.0);
  parameters_.Validate(PARAM_RHO)->LessThan(1.0);
}

/**
 * Build object links
 */
void AR1::DoBuild() {
  string error = "";
  if (!model()->objects().VerifyAddressableForUse(parameter_, addressable::kLookup, error)) {
    LOG_FATAL_P(PARAM_PARAMETER) << "could not be verified for use in additional_prior.ar1. Error: " << error;
  }

  addressable::Type addressable_type = model()->objects().GetAddressableType(parameter_);
  switch (addressable_type) {
    case addressable::kVector:
      addressable_vector_ = model()->objects().GetAddressableVector(parameter_);
      break;
    case addressable::kUnsignedMap:
      addressable_map_ = model()->objects().GetAddressableUMap(parameter_);
      break;
    case addressable::kMultiple:
      addressable_ptr_vector_ = model()->objects().GetAddressables(parameter_);
      break;
    default:
      LOG_ERROR() << "The addressable provided '" << parameter_ << "' has a type that is not supported for ar1 additional priors";
      break;
  }
}

/**
 * Get the AR1 score contribution
 * @return Penalty score
 */
Double AR1::GetScore() {
  vector<Double> values;
  if (addressable_vector_ != nullptr) {
    values.assign((*addressable_vector_).begin(), (*addressable_vector_).end());
  } else if (addressable_map_ != nullptr) {
    for (auto iter : (*addressable_map_)) values.push_back(iter.second);
  } else if (addressable_ptr_vector_ != nullptr) {
    for (auto ptr : (*addressable_ptr_vector_)) values.push_back(*ptr);
  } else {
    LOG_CODE_ERROR() << "No addressable has been linked for additional_prior.ar1";
  }

  if (values.size() < 2) {
    LOG_FATAL_P(PARAM_PARAMETER) << "must have at least 2 values for ar1 additional prior";
  }

  if (log_scale_) {
    for (Double& value : values) value = log(value);
  }

  const Double sigma2 = sigma_ * sigma_;
  Double       score  = 0.0;

  for (unsigned i = 1; i < values.size(); ++i) {
    const Double current_dev  = values[i] - mean_;
    const Double previous_dev = values[i - 1] - mean_;
    const Double residual     = current_dev - rho_ * previous_dev;
    score += residual * residual;
  }

  return 0.5 * multiplier_ * score / sigma2;
}

} /* namespace additionalpriors */
} /* namespace niwa */
