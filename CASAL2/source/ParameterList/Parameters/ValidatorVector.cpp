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

#include "Bindable.h"
#include "Categories/Categories.h"
#include "Logging/Logging.h"
#include "Model/Model.h"
#include "ParameterList/Parameter.h"
#include "ParameterList/ParameterList.h"
#include "Utilities/Math.h"

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
 * This method will return the current parameter as a Bindable storing a vector of strings
 */
BindableVector<std::string>* ValidatorVector::GetParameterAsVectorString(bool null_on_error) {
  auto* param = dynamic_cast<BindableVector<std::string>*>(parameter_);
  if (param == nullptr && !null_on_error) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::GetParameterAsVectorString " << parameter_->label() << " is not a vector<string> type";
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
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::GreaterThanOrEqualTo " << parameter_->label() << " is not a vector<double> type";
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
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::GreaterThanOrEqualTo " << parameter_->label() << " is not an unsigned type";
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
 * This method will check that the value of the parameter is less than or equal to the value passed
 */
shared_ptr<ValidatorVector> ValidatorVector::LessThanOrEqualTo(unsigned value) {
  auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::LessThanOrEqualTo " << parameter_->label() << " is not a vector<unsigned> type";
  }

  for (auto& val : *param->target()) {
    if (val > value) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << val << ") is invalid. Must be less than or equal to " << value;
    }
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<ValidatorVector> ValidatorVector::SumToOne() {
  auto* param = dynamic_cast<BindableVector<Double>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter " << parameter_->label() << " is not a vector-double type";
  }

  Double sum = 0.0;
  for (const auto& value : *param->target()) {
    sum += value;
  }

  if (!utilities::math::IsOne(sum)) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " values do not sum to one. Sum is " << AS_DOUBLE(sum);
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
shared_ptr<ValidatorVector> ValidatorVector::IsModelAge() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto* param = GetParameterAsVectorUnsigned();
  for (auto& val : *param->target()) {
    if (val < model_->min_age() || val > model_->max_age()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << val << ") is invalid. Must be between " << model_->min_age() << " and "
                  << model_->max_age();
    }
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

  auto*                   param      = GetParameterAsVectorUnsigned();
  auto                    final_year = fmax(model_->final_year(), model_->projection_final_year());
  map<unsigned, unsigned> seen_years;
  for (auto& val : *param->target()) {
    if (val < model_->start_year() || val > final_year) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << val << ") is invalid. Must be between " << model_->start_year()
                  << " and " << final_year;
    }

    if (seen_years[val] > 0) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << val << ") is invalid. Must be only defined once in the parameter.";
    }
    seen_years[val]++;  // mark this year as seen
  }

  return shared_from_this();
}

/**
 * This method will check that the values of the parameter are valid length bins
 */
shared_ptr<ValidatorVector> ValidatorVector::IsLengthBin() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto* param = GetParameterAsVectorDouble();
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::IsLengthBin " << parameter_->label() << " is not a vector<double> type";
    return shared_from_this();
  }

  auto model_length_bins = model_->length_bins();
  for (auto& val : *param->target()) {
    for (const auto& bin : model_length_bins) {
      if (utilities::math::IsEqual(val, bin)) {
        return shared_from_this();  // found a match
      }
    }
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << AS_DOUBLE(val) << ") is invalid. Must be a valid length bin";
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<ValidatorVector> ValidatorVector::DefaultToAllModelYears() {
  if (parameter_->has_been_defined()) {
    return shared_from_this();
  }

  auto* param = GetParameterAsVectorUnsigned();
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::DefaultToAllModelYears " << parameter_->label() << " is not a vector<unsigned> type";
    return shared_from_this();
  }

  // std::cout << "Model Years Size: " << model_->years().size() << endl;
  // return shared_from_this();

  if (param->target() != nullptr || param->target()->size() == 0) {
    *param->target() = model_->years();
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
  auto* param_string   = GetParameterAsVectorString(true);
  if (param_double == nullptr && param_unsigned == nullptr && param_string == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::NumberOfElements " << parameter_->label() << " is not a vector<double/unsigned/string> type";
    return shared_from_this();
  }

  unsigned actual_count = param_double ? param_double->target()->size() : (param_unsigned ? param_unsigned->target()->size() : param_string->target()->size());
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

  auto* source_double   = GetParameterAsVectorDouble(true);
  auto* source_unsigned = GetParameterAsVectorUnsigned(true);
  auto* source_string   = GetParameterAsVectorString(true);
  if (source_double == nullptr && source_unsigned == nullptr && source_string == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::SameNumberOfElementsAs " << parameter_->label() << " is not a vector<double/unsigned/string> type";
    return shared_from_this();
  }
  auto source_size = source_double ? source_double->target()->size() : (source_unsigned ? source_unsigned->target()->size() : source_string->target()->size());

  auto* target = parameters_->Get(label);
  if (target == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::SameNumberOfElementsAs " << label << " does not exist in the parameter list";
    return shared_from_this();
  }

  auto* target_double   = dynamic_cast<BindableVector<Double>*>(target);
  auto* target_unsigned = dynamic_cast<BindableVector<unsigned>*>(target);
  auto* target_string   = dynamic_cast<BindableVector<std::string>*>(target);
  if (target_double == nullptr && target_unsigned == nullptr && target_string == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::SameNumberOfElementsAs " << label << " is not a vector<double/unsigned/string> type";
    return shared_from_this();
  }
  auto target_size = target_double ? target_double->target()->size() : (target_unsigned ? target_unsigned->target()->size() : target_string->target()->size());

  if (target_string != nullptr && target_string->is_categories()) {
    auto category_total_size = model_->categories()->total_categories_defined(target_string->values());
    if (source_size != category_total_size) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has " << source_size << " elements, but category parameter '" << label << "' has "
                  << category_total_size << " elements";
    }

    return shared_from_this();
  }

  if (source_size != target_size) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has " << source_size << " elements, but " << label << " has " << target_size
                << " elements";
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
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " requires either 1 element or same number of elements as " << label << " ("
                  << dst.size() << "), but has " << src.size() << " elements";
      return;
    }
    if (src.size() == 1) {
      auto temp = src[0];
      src.assign(dst.size(), temp);
    }
  };

  if (auto* param = dynamic_cast<BindableVector<Double>*>(parameter_)) {
    if (auto* param2 = dynamic_cast<BindableVector<Double>*>(parameters_->Get(label))) {
      expand(param, param2, "double", "double");
    } else if (auto* param2 = dynamic_cast<BindableVector<unsigned>*>(parameters_->Get(label))) {
      expand(param, param2, "double", "unsigned");
    } else if (auto* param2 = dynamic_cast<BindableVector<std::string>*>(parameters_->Get(label))) {
      expand(param, param2, "double", "string");
    } else {
      LOG_CODE_ERROR() << "Parameter::Validator::ExpandToSameNumberOfElementsAs " << parameter_->label() << " -> " << label << " is not a vector<double/unsigned> type";
    }
  } else if (auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_)) {
    if (auto* param2 = dynamic_cast<BindableVector<Double>*>(parameters_->Get(label))) {
      expand(param, param2, "unsigned", "double");
    } else if (auto* param2 = dynamic_cast<BindableVector<unsigned>*>(parameters_->Get(label))) {
      expand(param, param2, "unsigned", "unsigned");
    } else if (auto* param2 = dynamic_cast<BindableVector<std::string>*>(parameters_->Get(label))) {
      expand(param, param2, "double", "string");
    } else {
      LOG_CODE_ERROR() << "Parameter::Validator::ExpandToSameNumberOfElementsAs " << parameter_->label() << " -> " << label << " is not a vector<double/unsigned> type";
    }
  } else if (auto* param = dynamic_cast<BindableVector<std::string>*>(parameter_)) {
    if (auto* param2 = dynamic_cast<BindableVector<Double>*>(parameters_->Get(label))) {
      expand(param, param2, "string", "double");
    } else if (auto* param2 = dynamic_cast<BindableVector<unsigned>*>(parameters_->Get(label))) {
      expand(param, param2, "string", "unsigned");
    } else if (auto* param2 = dynamic_cast<BindableVector<std::string>*>(parameters_->Get(label))) {
      expand(param, param2, "string", "string");
    } else {
      LOG_CODE_ERROR() << "Parameter::Validator::ExpandToSameNumberOfElementsAs " << parameter_->label() << " -> " << label << " is not a vector<double/unsigned/string> type";
    }

  } else {
    LOG_CODE_ERROR() << "Parameter::Validator::ExpandToSameNumberOfElementsAs " << parameter_->label() << " -> " << label << " is not a vector<double/unsigned> type";
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

  } else if (auto* param = dynamic_cast<BindableVector<std::string>*>(parameter_)) {
    auto* param2 = dynamic_cast<BindableVector<std::string>*>(parameters_->Get(label));
    if (param2 == nullptr) {
      LOG_CODE_ERROR() << "Parameter::Validator::DuplicateParameterIfNotAssigned " << label << " is not a vector<string> type";
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

/**
 * This method will check that the values in the parameter are in increasing order.
 * If the parameter is optional and has not been defined, it will return the current instance without validation.
 */
shared_ptr<ValidatorVector> ValidatorVector::IsInIncreasingOrder() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto* param = GetParameterAsVectorUnsigned();
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::IsInIncreasingOrder " << parameter_->label() << " is not a vector<unsigned> type";
  }

  const auto& values = *param->target();
  for (size_t i = 1; i < values.size(); ++i) {
    if (values[i] < values[i - 1]) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " values (" << values[i - 1] << ", " << values[i]
                  << ") are not in increasing order at index " << i;
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the values in the parameter are unique compared to another parameter.
 * If the parameter is optional and has not been defined, it will return the current instance without validation.
 */
shared_ptr<ValidatorVector> ValidatorVector::IsUniqueFrom(const string& label) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto* param = GetParameterAsVectorString();
  if (param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::IsUniqueFrom " << parameter_->label() << " is not a vector<string> type";
  }

  const auto& values       = *param->target();
  auto*       target_param = parameters_->Get(label);
  if (target_param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::IsUniqueFrom " << label << " does not exist in the parameter list";
  }

  auto* target_vector = dynamic_cast<BindableVector<std::string>*>(target_param);
  if (target_vector == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::IsUniqueFrom " << label << " is not a vector<string> type";
  }

  const auto& target_values = *target_vector->target();
  for (const auto& value : values) {
    if (std::find(target_values.begin(), target_values.end(), value) != target_values.end()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << value << ") is not unique compared to parameter " << label;
    }
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<ValidatorVector> ValidatorVector::EitherOrTableDefined(const string& table_label) {
  auto* table_param = parameters_->GetTable(table_label);
  if (table_param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::ForbiddenIfTableDefined " << table_label << " does not exist in the parameter list";
  }

  if (parameter_->has_been_defined() && table_param->has_been_defined()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " is forbidden if table " << table_label << " is defined";
  }
  if (!parameter_->has_been_defined() && !table_param->has_been_defined()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " must be defined if table " << table_label << " is not defined";
  }

  return shared_from_this();
}

}  // namespace niwa::parameters
