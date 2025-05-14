/**
 * @file ValidatorVector.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/08
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */

#include "ValidatorVector.h"

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
shared_ptr<ValidatorVector> ValidatorVector::GreaterThan(Double value) {
  LOG_CODE_ERROR() << "ValidatorVector::GreaterThan is not implemented";
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
shared_ptr<ValidatorVector> ValidatorVector::GreaterThanOrEqualTo(Double value) {
  auto* param = dynamic_cast<BindableVector<Double>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::GreaterThanOrEqualTo " << parameter_->label() << " is not a vector<double> type";
  }

  for (auto& val : *param->target()) {
    if (val < value) {
      LOG_ERROR() << this->parameter_->location() << "parameter " << parameter_->label() << " value (" << AS_DOUBLE(val) << ") is invalid. Must be greater than or equal to "
                  << AS_DOUBLE(value);
    }
  }

  return shared_from_this();
}

shared_ptr<ValidatorVector> ValidatorVector::LessThan(Double value) {
  LOG_CODE_ERROR() << "ValidatorVector::LessThan is not implemented";
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
shared_ptr<ValidatorVector> ValidatorVector::LessThanOrEqualTo(Double value) {
  auto* param = dynamic_cast<BindableVector<Double>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThanOrEqualTo " << parameter_->label() << " is not a vector<double> type";
  }

  for (auto& val : *param->target()) {
    if (val > value) {
      LOG_ERROR() << this->parameter_->location() << "parameter " << parameter_->label() << " value (" << AS_DOUBLE(val) << ") is invalid. Must be less than or equal to "
                  << AS_DOUBLE(value);
    }
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<ValidatorVector> ValidatorVector::IsInList(initializer_list<string> list) {
  LOG_CODE_ERROR() << "ValidatorVector::IsInList is not implemented";
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
