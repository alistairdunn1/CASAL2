/**
 * @file Normal.cpp
 * @author Casal2 Team
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "Normal.h"

#include "../../Model/Model.h"
#include "../../Model/Objects.h"

// namespaces
namespace niwa {
namespace additionalpriors {

/**
 * Default constructor
 *
 * Bind any parameters that are allowed to be loaded from the configuration files.
 */
Normal::Normal(shared_ptr<Model> model) : AdditionalPrior(model) {
  parameters_.Bind<string>(PARAM_PARAMETER, &parameter_, "The name of the parameter for the additional prior", "");
  parameters_.Bind<Double>(PARAM_MU, &mu_, "The normal prior mean (mu) parameter");
  parameters_.Bind<Double>(PARAM_CV, &cv_, "The normal CV parameter");
}

/**
 * Populate any parameters,
 * Validate that values are within expected ranges when bind<>() overloads cannot be used
 *
 * Note: all parameters are populated from configuration files
 */
void Normal::DoValidate() {
  parameters_.Validate(PARAM_MU)->GreaterThan(0.0);
  parameters_.Validate(PARAM_CV)->GreaterThan(0.0);
}

/**
 * Build the object
 */
void Normal::DoBuild() {
  string error = "";
  if (!model()->objects().VerifyAddressableForUse(parameter_, addressable::kLookup, error)) {
    LOG_FATAL_P(PARAM_PARAMETER) << "could not be found. Error: " << error;
  }

  addressable::Type addressable_type = model()->objects().GetAddressableType(parameter_);
  LOG_FINEST() << "type = " << addressable_type;
  switch (addressable_type) {
    case addressable::kInvalid:
      LOG_CODE_ERROR() << "Invalid addressable type: " << parameter_;
      break;
    case addressable::kMultiple:
      addressable_ptr_vector_ = model()->objects().GetAddressables(parameter_);
      break;
    case addressable::kVector:
      addressable_vector_ = model()->objects().GetAddressableVector(parameter_);
      break;
    case addressable::kUnsignedMap:
      addressable_map_ = model()->objects().GetAddressableUMap(parameter_);
      break;
    case addressable::kSingle:
      addressable_ = model()->objects().GetAddressable(parameter_);
      break;
    default:
      LOG_ERROR() << "The addressable provided '" << parameter_ << "' has a type that is not supported for normal additional priors";
      break;
  }
}

/**
 * Get the score
 * @return The score
 */
Double Normal::GetScore() {
  score_ = 0.0;
  vector<Double> values;

  if (addressable_vector_ != nullptr)
    values.assign((*addressable_vector_).begin(), (*addressable_vector_).end());
  else if (addressable_ptr_vector_ != nullptr) {
    for (auto ptr : (*addressable_ptr_vector_)) values.push_back((*ptr));
  } else if (addressable_map_ != nullptr) {
    for (auto iter : (*addressable_map_)) values.push_back(iter.second);
  } else if (addressable_ != nullptr) {
    values.push_back((*addressable_));
  } else {
    LOG_CODE_ERROR() << "(addressable_map_ != 0) && (addressable_vector_ != 0)";
  }

  sigma_ = mu_ * cv_;
  for (auto value : values) score_ += 0.5 * pow((value - mu_) / sigma_, 2);

  return score_;
}

} /* namespace additionalpriors */
} /* namespace niwa */
