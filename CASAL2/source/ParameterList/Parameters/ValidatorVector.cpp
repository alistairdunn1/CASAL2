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

#include <iostream>

#include "../../Logging/Logging.h"
#include "../../Model/Model.h"
#include "../Parameter.h"
#include "../ParameterList.h"
#include "Bindable.h"

namespace niwa::parameters {
using niwa::parameters::Bindable;
using std::cout;
using std::endl;

/**
 *
 */
BindableVector<Double>* ValidatorVector::GetParameterAsVectorDouble(bool null_on_error) {
  auto* param = dynamic_cast<BindableVector<Double>*>(parameter_);
  if (param == nullptr && !null_on_error) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::GetParameterAsVectorDouble " << parameter_->label() << " is not a vector<double> type";
  }
  return param;
}

/**
 * This method will return the current parameter as a Bindaable storing an unsigned
 */
BindableVector<unsigned>* ValidatorVector::GetParameterAsVectorUnsigned(bool null_on_error) {
  auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_);
  if (param == nullptr && !null_on_error) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::GetParameterAsVectorUnsigned " << parameter_->label() << " is not a vector<unsigned> type";
  }
  return param;
}

/**
 * This method will check that the value of the parameter is greater than the value passed
 */
shared_ptr<ValidatorVector> ValidatorVector::GreaterThan(Double value) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto* param = GetParameterAsVectorDouble();
  for (auto& val : *param->target()) {
    if (val <= value) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << AS_DOUBLE(val) << ") is invalid. Must be greater than "
                  << AS_DOUBLE(value);
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is greater than the value passed
 */
shared_ptr<ValidatorVector> ValidatorVector::GreaterThan(unsigned value) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto* param = GetParameterAsVectorUnsigned();
  for (auto& val : *param->target()) {
    if (val <= value) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << val << ") is invalid. Must be greater than " << value;
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is greater than or equal to the value passed
 * in as the parameter
 */
shared_ptr<ValidatorVector> ValidatorVector::GreaterThanOrEqualTo(Double value) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

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

/**
 * This method will check that the value of the parameter is greater than or equal to the value passed
 */
shared_ptr<ValidatorVector> ValidatorVector::GreaterThanOrEqualTo(unsigned value) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::GreaterThanOrEqualTo " << parameter_->label() << " is not an unsigned type";
  }

  for (auto& val : *param->target()) {
    if (val < value) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << val << ") is invalid. Must be greater than or equal to " << value;
    }
  }

  return shared_from_this();
}

/**
 *
 */
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

/**
 *
 */
shared_ptr<ValidatorVector> ValidatorVector::LessThanOrEqualToParameter(const string& label) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  if (auto* param = dynamic_cast<BindableVector<Double>*>(parameter_)) {
    // handle for when the parameters are both double
    auto* param2 = dynamic_cast<Bindable<Double>*>(parameters_->Get(label));
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter::Validator::LessThanOrEqualToParameter " << label << " is not a double type";
    }

    Double target = *param2->target();
    for (auto& source : *param->target()) {
      if (source > target) {
        LOG_ERROR() << this->parameter_->location() << " parameter " << param->label() << " value (" << AS_DOUBLE(source) << ") is invalid. Must be less than or equal to " << label
                    << " (" << AS_DOUBLE(target) << ")";
      }
    }

  } else if (auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_)) {
    // handle for when the parameters are both unsigned
    auto* param2 = dynamic_cast<Bindable<unsigned>*>(parameters_->Get(label));
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter::Validator::LessThanOrEqualToParameter " << label << " is not an unsigned type";
    }
    unsigned target = *param2->target();
    for (auto& source : *param->target()) {
      if (source > target) {
        LOG_ERROR() << this->parameter_->location() << " parameter " << param->label() << " value (" << source << ") is invalid. Must be less than or equal to " << label << " ("
                    << target << ")";
      }
    }

  } else {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThanOrEqualToParameter " << parameter_->label() << " is not a double/unsigned type";
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<ValidatorVector> ValidatorVector::IsModelYear() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto* param = GetParameterAsVectorUnsigned();
  for (auto& val : *param->target()) {
    if (val < model_->start_year() || val > model_->final_year()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << val << ") is invalid. Must be between " << model_->start_year()
                  << " and " << model_->final_year();
    }
  }
  return shared_from_this();
}

/**
 *
 */
shared_ptr<ValidatorVector> ValidatorVector::NumberOfElements(unsigned count) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto* param_double   = GetParameterAsVectorDouble(true);
  auto* param_unsigned = GetParameterAsVectorUnsigned(true);
  if (param_double == nullptr && param_unsigned == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::NumberOfElements " << parameter_->label() << " is not a vector<double/unsigned> type";
    return shared_from_this();
  }

  unsigned actual_count = param_double ? param_double->target()->size() : param_unsigned->target()->size();
  if (actual_count != count) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has " << actual_count << " elements, but requires exactly " << count;
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<ValidatorVector> ValidatorVector::SameNumberOfElementsAs(const string& label) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto check_size = [&](auto* param1, auto* param2) {
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter::Validator::SameNumberOfElementsAs " << label << " is not a compatible vector type";
      return;
    }
    if (param1->target()->size() != param2->target()->size()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has a different number of elements than " << label;
    }
  };

  if (auto* param = dynamic_cast<BindableVector<Double>*>(parameter_)) {
    if (auto* param2 = dynamic_cast<BindableVector<Double>*>(parameters_->Get(label))) {
      check_size(param, param2);
    } else if (auto* param2 = dynamic_cast<BindableVector<unsigned>*>(parameters_->Get(label))) {
      check_size(param, param2);
    } else {
      LOG_CODE_ERROR() << "Parameter::Validator::SameNumberOfElementsAs " << parameter_->label() << " is not a vector<double/unsigned> type";
    }
  } else if (auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_)) {
    if (auto* param2 = dynamic_cast<BindableVector<Double>*>(parameters_->Get(label))) {
      check_size(param, param2);
    } else if (auto* param2 = dynamic_cast<BindableVector<unsigned>*>(parameters_->Get(label))) {
      check_size(param, param2);
    } else {
      LOG_CODE_ERROR() << "Parameter::Validator::SameNumberOfElementsAs " << parameter_->label() << " is not a vector<double/unsigned> type";
    }
  } else {
    LOG_CODE_ERROR() << "Parameter::Validator::SameNumberOfElementsAs " << parameter_->label() << " is not a vector<double/unsigned> type";
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<ValidatorVector> ValidatorVector::ExpandToSameNumberOfElementsAs(const string& label) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto expand = [&](auto* param, auto* param2, const char* type_name1, const char* type_name2) {
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter::Validator::ExpandToSameNumberOfElementsAs " << label << " is not a vector<" << type_name2 << "> type";
      return;
    }
    auto& src = *param->target();
    auto& dst = *param2->target();
    if (src.size() != 1 && src.size() != dst.size()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " requires either 1 element or same number of elements as " << label;
      return;
    }
    if (src.size() == 1) {
      std::cout << "src.size() = " << src.size() << " dst.size() = " << dst.size() << endl;
      auto temp = src[0];
      src.assign(dst.size(), temp);
    }
  };

  if (auto* param = dynamic_cast<BindableVector<Double>*>(parameter_)) {
    if (auto* param2 = dynamic_cast<BindableVector<Double>*>(parameters_->Get(label))) {
      expand(param, param2, "double", "double");
    } else if (auto* param2 = dynamic_cast<BindableVector<unsigned>*>(parameters_->Get(label))) {
      expand(param, param2, "double", "unsigned");
    } else {
      LOG_CODE_ERROR() << "Parameter::Validator::ExpandToSameNumberOfElementsAs " << parameter_->label() << " is not a vector<double/unsigned> type";
    }
  } else if (auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_)) {
    if (auto* param2 = dynamic_cast<BindableVector<Double>*>(parameters_->Get(label))) {
      expand(param, param2, "unsigned", "double");
    } else if (auto* param2 = dynamic_cast<BindableVector<unsigned>*>(parameters_->Get(label))) {
      expand(param, param2, "unsigned", "unsigned");
    } else {
      LOG_CODE_ERROR() << "Parameter::Validator::ExpandToSameNumberOfElementsAs " << parameter_->label() << " is not a vector<double/unsigned> type";
    }
  } else {
    LOG_CODE_ERROR() << "Parameter::Validator::ExpandToSameNumberOfElementsAs " << parameter_->label() << " is not a vector<double/unsigned> type";
  }

  return shared_from_this();
}

/**
 * This method will take the current parameter and check if it has been defined by the user in the configuration file.
 *
 * If the parameter has not been defined, then we're going to assign it based on the parameter passed in as the label.
 *
 * @param label The label of the parameter to copy from if current parameter has not been defined
 */
shared_ptr<ValidatorVector> ValidatorVector::DuplicateParameterIfNotAssigned(const string& label) {
  // If it's already been defined we don't need to do anything
  if (parameter_->has_been_defined()) {
    return shared_from_this();
  }

  if (auto* param = dynamic_cast<BindableVector<Double>*>(parameter_)) {
    auto* param2 = dynamic_cast<BindableVector<Double>*>(parameters_->Get(label));
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter::Validator::DuplicateParameterIfNotAssigned " << label << " is not a vector<double> type";
    }
    param->target()->assign(param2->target()->begin(), param2->target()->end());

  } else if (auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_)) {
    auto* param2 = dynamic_cast<BindableVector<unsigned>*>(parameters_->Get(label));
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter::Validator::DuplicateParameterIfNotAssigned " << label << " is not a vector<unsigned> type";
    }
    param->target()->assign(param2->target()->begin(), param2->target()->end());

  } else {
    LOG_CODE_ERROR() << "Parameter::Validator::DuplicateParameterIfNotAssigned " << parameter_->label() << " is not a vector<double/unsigned> type";
  }

  return shared_from_this();
}

/**
 * This method will check that the number of elements in the parameter is the same as the model age spread.
 * If the parameter is optional and has not been defined, it will return the current instance without validation.
 */
shared_ptr<ValidatorVector> ValidatorVector::SameNumberOfElementsModelAgeSpread() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto* param = GetParameterAsVectorDouble();
  if (param->target()->size() != model_->age_spread()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has a different number of elements than the model age spread (" << model_->age_spread()
                << ")";
  }

  return shared_from_this();
}

/**
 * This method will check that the number of elements in the parameter is the same as the number of model length bin mid points.
 * If the parameter is optional and has not been defined, it will return the current instance without validation.
 */
shared_ptr<ValidatorVector> ValidatorVector::SameNumberOfElementsModelLengthBinMidPoints() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto* param = GetParameterAsVectorDouble();
  if (param->target()->size() != model_->length_bin_mid_points().size()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has a different number of elements than the model length bin mid points ("
                << model_->length_bin_mid_points().size() << ")";
  }

  return shared_from_this();
}

}  // namespace niwa::parameters
