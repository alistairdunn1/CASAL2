/**
 * @file Uniform.cpp
 * @author C.Marsh
 * @github https://github.com/Zaita
 * @date 3/7/2017
 * @section LICENSE
 *
 */

// headers
#include "Uniform.h"

#include "../../Model/Model.h"
#include "../../Model/Objects.h"

// namespaces
namespace niwa {
namespace additionalpriors {

/**
 * Default constructor
 *
 * Bind any parameters that are allowed to be loaded from the configuration files.
 * Set bounds on registered parameters
 * Register any parameters that can be an estimated or utilised in other run modes (e.g., profiling, yields, projections, etc.)
 * Set some initial values
 *
 * Note: The constructor is parsed to generate LaTeX for the documentation.
 */
Uniform::Uniform(shared_ptr<Model> model) : AdditionalPrior(model) {
  parameters_.Bind<string>(PARAM_PARAMETER, &parameter_, "The name of the parameter for the additional prior", "");
}

/**
 * Build the object
 */
void Uniform::DoBuild() {
  string error = "";
  if (!model_->objects().VerifyAddressableForUse(parameter_, addressable::kLookup, error)) {
    LOG_FATAL_P(PARAM_PARAMETER) << "could not be found. Error: " << error;
  }

  addressable::Type addressable_type = model_->objects().GetAddressableType(parameter_);
  LOG_FINEST() << "type = " << addressable_type;
  switch (addressable_type) {
    case addressable::kInvalid:
      LOG_CODE_ERROR() << "Invalid addressable type: " << parameter_;
      break;
    case addressable::kSingle:
      addressable_ = model_->objects().GetAddressable(parameter_);
      break;
    default:
      LOG_ERROR() << "The addressable provided '" << parameter_ << "' has a type that is not supported for uniform additional priors";
      break;
  }
}

/**
 * Get the score
 * @return The score
 */
Double Uniform::GetScore() {
  Double value = (*addressable_);
  score_       = 0.0;
  LOG_FINEST() << "score = " << score_ << " value = " << value;
  return score_;
}

} /* namespace additionalpriors */
} /* namespace niwa */
