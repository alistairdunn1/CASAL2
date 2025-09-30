/**
 * @file Sum.cpp
 * @author A. Dunn
 * @date 2/8/2023
 * @section LICENSE
 */

// headers
#include "Sum.h"

#include "../../Estimates/Manager.h"
#include "../../Model/Model.h"
#include "../../Model/Objects.h"

// namespaces
namespace niwa {
namespace additionalpriors {

/**
 * Default constructor
 */
Sum::Sum(shared_ptr<Model> model) : AdditionalPrior(model) {
  parameters_.Bind<string>(PARAM_PARAMETERS, &parameter_list_, "The names of the parameters for summing", "");
  parameters_.Bind<string>(PARAM_DISTRIBUTION, &distribution_, "The additional prior distribution to apply", "", PARAM_LOGNORMAL)
      ->set_allowed_values({PARAM_LOGNORMAL, PARAM_NORMAL});
  parameters_.Bind<Double>(PARAM_MU, &mu_, "Mean of the distribution", "", 1.0)->set_lower_bound(0.0, false);
  parameters_.Bind<Double>(PARAM_CV, &cv_, "cv of the distribution", "")->set_lower_bound(0.0, false);
}

/**
 * Validate the parameters
 */
void Sum::DoValidate() {
  // TODO:  Needs to be improved
  parameter_ = parameter_list_[0];
}

/**
 * Build the parameters
 */
void Sum::DoBuild() {
  LOG_TRACE();
  string error = "";
  for (unsigned i = 0; i < parameter_list_.size(); ++i)
    if (!model_->objects().VerifyAddressableForUse(parameter_list_[i], addressable::kLookup, error)) {
      LOG_FATAL_P(PARAM_PARAMETERS) << "the parameter '" << parameter_list_[i] << "' could not be found.";
    }

  // first parameter
  vector<Double> values;

  for (unsigned i = 0; i < parameter_list_.size(); ++i) {
    addressable::Type addressable_type = model_->objects().GetAddressableType(parameter_list_[i]);
    LOG_FINEST() << "addressable type = " << addressable_type;
    switch (addressable_type) {
      case addressable::kInvalid:
        LOG_CODE_ERROR() << "Invalid addressable type: " << parameter_list_[i];
        break;
      case addressable::kMultiple:
        LOG_CODE_ERROR() << "Invalid addressable type. The addressable cannot be multiple values: " << parameter_list_[i];
        break;
      case addressable::kVector:
        LOG_CODE_ERROR() << "Invalid addressable type. The addressable cannot be a vector: " << parameter_list_[i];
        break;
      case addressable::kUnsignedMap:
        LOG_CODE_ERROR() << "Invalid addressable type. The addressable cannot be a vector: " << parameter_list_[i];
        break;
      case addressable::kSingle:
        addressable_ = model_->objects().GetAddressable(parameter_list_[i]);
        values.push_back((*addressable_));
        break;
      default:
        LOG_ERROR() << "The addressable provided '" << parameter_list_[i] << "' has a type that is not supported for the sum additional prior";
        break;
    }
  }
}

/**
 * Get the score for this penalty
 * @return The penalty score
 */
Double Sum::GetScore() {
  LOG_TRACE();
  vector<Double> values;

  // first parameter
  for (unsigned i = 0; i < parameter_list_.size(); ++i) {
    addressable_ = model_->objects().GetAddressable(parameter_list_[i]);
    values.push_back((*addressable_));
  }

  Double score = 0.0;
  Double value = 0.0;

  for (unsigned i = 0; i < values.size(); ++i) value += values[i];
  LOG_FINEST() << "size of paramter_list = " << values.size() << " with value=" << value;

  if (distribution_ == PARAM_NORMAL) {
    sigma_ = mu_ * cv_;
    score  = 0.5 * ((value - mu_) / sigma_) * ((value - mu_) / sigma_);
  } else if (distribution_ == PARAM_LOGNORMAL) {
    sigma_ = sqrt(log(1 + cv_ * cv_));
    score  = log(value) + 0.5 * pow(log(value / mu_) / sigma_ + sigma_ * 0.5, 2);
  }

  return score;
}

} /* namespace additionalpriors */
} /* namespace niwa */
