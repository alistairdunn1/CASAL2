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

namespace niwa::parameters {
using niwa::parameters::Bindable;

/**
 * This method will check that the value of the parameter is greater than the value passed
 */
shared_ptr<Validator> Validator::GreaterThan(Double value) {
  auto* param = dynamic_cast<Bindable<Double>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::GreaterThan " << parameter_->label() << " is not a double type";
  }
  Double source = *param->target();
  if (source <= value) {
    LOG_ERROR() << this->parameter_->location() << "parameter " << parameter_->label() << " value (" << AS_DOUBLE(source) << ") is invalid. Must be greater than "
                << AS_DOUBLE(value);
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
    LOG_ERROR() << this->parameter_->location() << "value (" << AS_DOUBLE(source) << ") is invalid. Must be greater than or equal to " << AS_DOUBLE(value);
  }

  return shared_from_this();
}

shared_ptr<Validator> Validator::LessThan(Double value) {
  auto* param = dynamic_cast<Bindable<Double>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThan " << parameter_->label() << " is not a double type";
  }
  Double source = *param->target();
  if (source >= value) {
    LOG_ERROR() << this->parameter_->location() << "value (" << AS_DOUBLE(source) << ") is invalid. Must be less than " << AS_DOUBLE(value);
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
shared_ptr<Validator> Validator::LessThanParameter(const string& label) {
  auto* param = dynamic_cast<Bindable<Double>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThanParameter " << parameter_->label() << " is not a double type";
  }

  auto* param2 = dynamic_cast<Bindable<Double>*>(parameters_->Get(label));
  if (param2 == nullptr) {
    LOG_CODE_ERROR() << "Parameter " << label << " is not a double type";
  }

  Double source = *param->target();
  Double target = *param2->target();

  if (source < target) {
    LOG_ERROR() << this->parameter_->location() << "value (" << AS_DOUBLE(source) << ") is invalid. Must be less than " << label << " (" << AS_DOUBLE(target) << ")";
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<Validator> Validator::LessThanOrEqualToParameter(const string& label) {
  auto* param = dynamic_cast<Bindable<Double>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThanOrEqualToParameter " << parameter_->label() << " is not a double type";
  }

  auto* param2 = dynamic_cast<Bindable<Double>*>(parameters_->Get(label));
  if (param2 == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThanOrEqualToParameter " << label << " is not a double type";
  }

  Double source = *param->target();
  Double target = *param2->target();

  if (source <= target) {
    LOG_ERROR() << this->parameter_->location() << "value (" << AS_DOUBLE(source) << ") is invalid. Must be less than or equal to " << label << " (" << AS_DOUBLE(target) << ")";
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
    LOG_ERROR() << this->parameter_->location() << "value (" << source << ") is invalid. Must be greater than or equal to model min age (" << model_->min_age() << ")";
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<Validator> Validator::LessThanOrEqualToModelMaxAge() {
  auto* param = dynamic_cast<Bindable<unsigned>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter " << parameter_->label() << " is not a double type";
  }

  unsigned source = *param->target();
  if (source > model_->max_age()) {
    LOG_ERROR() << this->parameter_->location() << "value (" << source << ") is invalid. Must be less than or equal to model max age (" << model_->max_age() << ")";
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

}  // namespace niwa::parameters
