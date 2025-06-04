/**
 * @file Validator.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/08
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */

#include "Validator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../Logging/Logging.h"
#include "../../Model/Model.h"
#include "../Parameter.h"
#include "../ParameterList.h"
#include "Bindable.h"
#include "BindableVector.h"

namespace niwa::parameters {
using niwa::parameters::Bindable;

/**
 * This method will return the current parameter as a Bindaable storing a double
 */
Bindable<Double>* Validator::GetParameterAsDouble(bool null_on_error) {
  auto* param = dynamic_cast<Bindable<Double>*>(parameter_);
  if (param == nullptr && !null_on_error) {
    LOG_CODE_ERROR() << "Parameter::Validator::GetParameterAsDouble " << parameter_->label() << " is not a double type";
  }
  return param;
}

/**
 * This method will return the current parameter as a Bindaable storing an unsigned
 */
Bindable<unsigned>* Validator::GetParameterAsUnsigned(bool null_on_error) {
  auto* param = dynamic_cast<Bindable<unsigned>*>(parameter_);
  if (param == nullptr && !null_on_error) {
    LOG_CODE_ERROR() << "Parameter::Validator::GetParameterAsUnsigned " << parameter_->label() << " is not an unsigned type";
  }
  return param;
}

/**
 * This method will check that the value of the parameter is greater than the value passed
 */
shared_ptr<Validator> Validator::GreaterThan(Double value) {
  auto*  param  = GetParameterAsDouble();
  Double source = *param->target();
  if (source <= value) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << AS_DOUBLE(source) << ") is invalid. Must be greater than "
                << AS_DOUBLE(value);
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is greater than the value passed
 */
shared_ptr<Validator> Validator::GreaterThan(unsigned value) {
  auto*    param  = GetParameterAsUnsigned();
  unsigned source = *param->target();
  if (source <= value) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be greater than " << value;
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is greater than or equal to the value passed
 * in as the parameter
 */
shared_ptr<Validator> Validator::GreaterThanOrEqualTo(Double value) {
  auto* param = dynamic_cast<Bindable<Double>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::GreaterThanOrEqualTo " << parameter_->label() << " is not a double type";
  }

  Double source = *param->target();
  if (source < value) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << AS_DOUBLE(source) << ") is invalid. Must be greater than or equal to "
                << AS_DOUBLE(value);
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is greater than or equal to the value passed
 */
shared_ptr<Validator> Validator::GreaterThanOrEqualTo(unsigned value) {
  auto* param = dynamic_cast<Bindable<unsigned>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::GreaterThanOrEqualTo " << parameter_->label() << " is not an unsigned type";
  }

  unsigned source = *param->target();
  if (source < value) {
    LOG_ERROR() << this->parameter_->location() << "parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be greater than or equal to " << value;
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is greater than or equal to the value of
 * the parameter passed in as the label
 */
shared_ptr<Validator> Validator::GreaterThanOrEqualToParameter(const string& label) {
  auto* source_unsigned = GetParameterAsUnsigned(true);
  auto* source_double   = GetParameterAsDouble(true);
  if (source_unsigned == nullptr && source_double == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::GreaterThanOrEqualToParameter " << parameter_->label() << " is not a double or unsigned type";
  }

  auto* target_unsigned = dynamic_cast<Bindable<unsigned>*>(parameters_->Get(label));
  auto* target_double   = dynamic_cast<Bindable<Double>*>(parameters_->Get(label));
  if (target_unsigned == nullptr && target_double == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::GreaterThanOrEqualToParameter " << label << " is not a double or unsigned type";
  }

  Double source = source_double ? *source_double->target() : static_cast<Double>(*source_unsigned->target());
  Double target = target_double ? *target_double->target() : static_cast<Double>(*target_unsigned->target());
  if (source < target) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << AS_DOUBLE(source) << ") is invalid. Must be greater than or equal to "
                << label << " (" << AS_DOUBLE(target) << ")";
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is greater than or equal to the value passed
 */
shared_ptr<Validator> Validator::LessThan(Double value) {
  auto* param = dynamic_cast<Bindable<Double>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThan " << parameter_->label() << " is not a double type";
  }
  Double source = *param->target();
  if (source >= value) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << AS_DOUBLE(source) << ") is invalid. Must be less than "
                << AS_DOUBLE(value);
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<Validator> Validator::LessThanOrEqualTo(Double value) {
  auto* param = dynamic_cast<Bindable<Double>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThanOrEqualTo " << parameter_->label() << " is not a double type";
  }
  Double source = *param->target();
  if (source > value) {
    LOG_ERROR() << this->parameter_->location() << "value (" << AS_DOUBLE(source) << ") is invalid. Must be less than or equal to " << AS_DOUBLE(value);
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<Validator> Validator::LessThanOrEqualTo(unsigned value) {
  auto* param = dynamic_cast<Bindable<unsigned>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThanOrEqualTo " << parameter_->label() << " is not an unsigned type";
  }
  unsigned source = *param->target();
  if (source > value) {
    LOG_ERROR() << this->parameter_->location() << "value (" << source << ") is invalid. Must be less than or equal to " << value;
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<Validator> Validator::LessThanParameter(const string& label) {
  if (auto* param = dynamic_cast<Bindable<Double>*>(parameter_)) {
    // Handle for when the parameters are both double
    auto* param2 = dynamic_cast<Bindable<Double>*>(parameters_->Get(label));
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter " << label << " is not a double type. Cannot compare to " << param->label() << " which is a double type";
    }

    Double source = *param->target();
    Double target = *param2->target();

    if (source >= target) {
      LOG_ERROR() << this->parameter_->location() << " parameter" << param->label() << " value (" << AS_DOUBLE(source) << ") is invalid. Must be less than " << label << " ("
                  << AS_DOUBLE(target) << ")";
    }

  } else if (auto* param = dynamic_cast<Bindable<unsigned>*>(parameter_)) {
    // handle for when the parameters are both unsigned
    auto* param2 = dynamic_cast<Bindable<unsigned>*>(parameters_->Get(label));
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter " << label << " is not an unsigned type. Cannot compare to " << param->label() << " which is an unsigned type";
    }

    unsigned source = *param->target();
    unsigned target = *param2->target();

    if (source >= target) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << param->label() << " value (" << source << ") is invalid. Must be less than " << label << " (" << target
                  << ")";
    }

  } else {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThanParameter " << parameter_->label() << " is not a double/unsigned or string type";
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<Validator> Validator::LessThanOrEqualToParameter(const string& label) {
  auto* source_unsigned = GetParameterAsUnsigned(true);
  auto* source_double   = GetParameterAsDouble(true);
  if (source_unsigned == nullptr && source_double == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThanOrEqualToParameter " << parameter_->label() << " is not a double or unsigned type";
  }

  auto* target_unsigned = dynamic_cast<Bindable<unsigned>*>(parameters_->Get(label));
  auto* target_double   = dynamic_cast<Bindable<Double>*>(parameters_->Get(label));
  if (target_unsigned == nullptr && target_double == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThanOrEqualToParameter " << label << " is not a double or unsigned type";
  }

  Double source = source_double ? *source_double->target() : static_cast<Double>(*source_unsigned->target());
  Double target = target_double ? *target_double->target() : static_cast<Double>(*target_unsigned->target());
  if (source > target) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << AS_DOUBLE(source) << ") is invalid. Must be less than or equal to "
                << label << " (" << AS_DOUBLE(target) << ")";
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<Validator> Validator::GreaterThanOrEqualToModelMinAge() {
  auto* param = dynamic_cast<Bindable<unsigned>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter " << parameter_->label() << " is not a double type";
  }

  unsigned source = *param->target();
  if (source < model_->min_age()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be greater than or equal to model min age ("
                << model_->min_age() << ")";
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<Validator> Validator::GreaterThanModelMinAge() {
  auto* param = dynamic_cast<Bindable<unsigned>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter " << parameter_->label() << " is not a double type";
  }

  unsigned source = *param->target();
  if (source <= model_->min_age()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be greater than model min age ("
                << model_->min_age() << ")";
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<Validator> Validator::LessThanOrEqualToModelMaxAge() {
  auto* param_double   = GetParameterAsDouble(true);
  auto* param_unsigned = GetParameterAsUnsigned(true);
  if (param_double == nullptr && param_unsigned == nullptr) {
    LOG_CODE_ERROR() << "Validator::LessThanOrEqualToModelMaxAge() - Parameter " << parameter_->label() << " is not a double or unsigned type";
  }

  unsigned value = param_unsigned ? *param_unsigned->target() : static_cast<unsigned>(*param_double->target());
  if (value > model_->max_age()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << value << ") is invalid. Must be less than or equal to model max age ("
                << model_->max_age() << ")";
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is less than the model max age
 */
shared_ptr<Validator> Validator::LessThanModelMaxAge() {
  auto* param_double   = GetParameterAsDouble(true);
  auto* param_unsigned = GetParameterAsUnsigned(true);
  if (param_double == nullptr && param_unsigned == nullptr) {
    LOG_CODE_ERROR() << "Validator::LessThanModelMaxAge() - Parameter " << parameter_->label() << " is not a double or unsigned type";
  }

  unsigned value = param_unsigned ? *param_unsigned->target() : static_cast<unsigned>(*param_double->target());
  if (value >= model_->max_age()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << value << ") is invalid. Must be less than or equal to model max age ("
                << model_->max_age() << ")";
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is an age in the model (between min_age and max_age inclusive)
 */
shared_ptr<Validator> Validator::IsAge() {
  auto* param = dynamic_cast<Bindable<unsigned>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter " << parameter_->label() << " is not a double type";
  }

  unsigned source = *param->target();

  // If the parameter is optional and we're using the default value, then we don't need to validate
  if (param->is_optional() && source == param->default_value()) {
    return shared_from_this();
  }

  if (source < model_->min_age() || source > model_->max_age()) {
    LOG_ERROR() << this->parameter_->location() << "value (" << source << ") is invalid. Must be between model min age (" << model_->min_age() << ") and max age ("
                << model_->max_age() << ")";
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<Validator> Validator::IsInList(initializer_list<string> list) {
  auto* param = dynamic_cast<Bindable<string>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter " << parameter_->label() << " is not a vector-string type";
  }

  string source = *param->target();
  for (const auto& item : list) {
    if (source == item) {
      return shared_from_this();
    }
  }

  LOG_ERROR() << this->parameter_->location() << "value (" << source << ") is invalid. Must be in the list";
  return shared_from_this();
}

/**
 *
 */
shared_ptr<Validator> Validator::DuplicateParameterIfNotAssigned(const string& label) {
  // If it's already been defined we don't need to do anything
  if (parameter_->has_been_defined()) {
    return shared_from_this();
  }

  if (auto* param = dynamic_cast<Bindable<Double>*>(parameter_)) {
    auto* param2 = dynamic_cast<Bindable<Double>*>(parameters_->Get(label));
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter::Validator::DuplicateParameterIfNotAssigned " << label << " is not a double type";
    }
    *param->target() = *param2->target();

  } else if (auto* param = dynamic_cast<Bindable<unsigned>*>(parameter_)) {
    auto* param2 = dynamic_cast<Bindable<unsigned>*>(parameters_->Get(label));
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter::Validator::DuplicateParameterIfNotAssigned " << label << " is not an unsigned type";
    }
    *param->target() = *param2->target();

  } else if (auto* param = dynamic_cast<Bindable<string>*>(parameter_)) {
    auto* param2 = dynamic_cast<Bindable<string>*>(parameters_->Get(label));
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter::Validator::DuplicateParameterIfNotAssigned " << label << " is not a string type";
    }
    *param->target() = *param2->target();

  } else {
    LOG_CODE_ERROR() << "Parameter::Validator::DuplicateParameterIfNotAssigned " << parameter_->label() << " is not a double/unsigned type";
  }

  return shared_from_this();
}

}  // namespace niwa::parameters
