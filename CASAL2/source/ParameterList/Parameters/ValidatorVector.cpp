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
#include <numeric>

#include "Bindable.h"
#include "Categories/Categories.h"
#include "Logging/Logging.h"
#include "Model/Model.h"
#include "ParameterList/Parameter.h"
#include "ParameterList/ParameterList.h"
#include "Utilities/Math.h"
#include "Utilities/To.h"

namespace niwa::parameters {
using niwa::parameters::Bindable;

/**
 * This method will convert the values of the parameter to a vector of doubles.
 * If the parameter is not a vector of doubles, it will error out.
 *
 * @return A vector of doubles representing the parameter values.
 */
vector<double> ValidatorVector::ConvertValuesToDouble() const {
  vector<double> parameter_values;
  auto           invalid = utilities::To<double>(parameter_->values(), parameter_values);

  if (!invalid.empty()) {
    LOG_CODE_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has " << invalid.size()
                     << " invalid values. This should've been picked up earlier";
  }

  return parameter_values;
}

/**
 * This method will convert the values of the parameter to a vector of unsigned integers.
 * If the parameter is not a vector of unsigned integers, it will error out.
 *
 * @return A vector of unsigned integers representing the parameter values.
 */
vector<unsigned> ValidatorVector::ConvertValuesToUnsigned() const {
  vector<unsigned> parameter_values;
  auto             invalid = utilities::To<unsigned>(parameter_->values(), parameter_values);

  if (!invalid.empty()) {
    LOG_CODE_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has " << invalid.size()
                     << " invalid values. This should've been picked up earlier";
  }

  return parameter_values;
}

BindableVector<Double>* ValidatorVector::GetParameterAsVectorDouble(bool null_on_error) {
  auto* param = dynamic_cast<BindableVector<Double>*>(parameter_);
  if (param == nullptr && !null_on_error) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::GetParameterAsVectorDouble " << this->parameter_->location() << " " << parameter_->label() << " is not a vector<double> type";
  }
  return param;
}
BindableVector<unsigned>* ValidatorVector::GetParameterAsVectorUnsigned(bool null_on_error) {
  auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_);
  if (param == nullptr && !null_on_error) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::GetParameterAsVectorUnsigned " << this->parameter_->location() << " " << parameter_->label()
                     << " is not a vector<unsigned> type";
  }
  return param;
}
BindableVector<std::string>* ValidatorVector::GetParameterAsVectorString(bool null_on_error) {
  auto* param = dynamic_cast<BindableVector<std::string>*>(parameter_);
  if (param == nullptr && !null_on_error) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::GetParameterAsVectorString " << this->parameter_->location() << " " << parameter_->label() << " is not a vector<string> type";
  }
  return param;
}

/**
 * This method will check that the value of the parameter is greater than the value passed
 *
 * @param value The value to compare against the current parameter value.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::GreaterThan(double value) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto parameter_values = ConvertValuesToDouble();
  for (const auto& source : parameter_values) {
    if (source <= value) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be greater than " << value;
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is greater than the value passed
 *
 * @param value The value to compare against the current parameter value.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::GreaterThan(unsigned value) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto parameter_values = ConvertValuesToUnsigned();
  for (const auto& source : parameter_values) {
    if (source <= value) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be greater than " << value;
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is greater than or equal to the value passed
 * in as the parameter
 *
 * @param value The value to compare against the current parameter value.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::GreaterThanOrEqualTo(double value) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto parameter_values = ConvertValuesToDouble();
  for (const auto& source : parameter_values) {
    if (source < value) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be greater than " << value;
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is greater than or equal to the value passed
 *
 * @param value The value to compare against the current parameter value.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::GreaterThanOrEqualTo(unsigned value) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto parameter_values = ConvertValuesToUnsigned();
  for (const auto& source : parameter_values) {
    if (source < value) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be greater than " << value;
    }
  }

  return shared_from_this();
}

/**
 * This method does a validation to ensure the current parameter value is less than the value passed in.
 * This check is done by checking the current parameter value is not greater than or equal to the value passed in.
 *
 * @param value The value to compare against the current parameter value.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::LessThan(double value) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto parameter_values = ConvertValuesToDouble();
  for (const auto& source : parameter_values) {
    if (source >= value) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be less than " << value;
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is less than or equal to the value passed.
 * This check is done by checking the current parameter value is not greater than the value passed in.
 *
 * @param value The value to compare against the current parameter value.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::LessThanOrEqualTo(double value) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto parameter_values = ConvertValuesToDouble();
  for (const auto& source : parameter_values) {
    if (source > value) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be less than or equal to " << value;
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is less than or equal to the value passed.
 * This check is done by checking the current parameter value is not greater than the value passed in.
 *
 * @param value The value to compare against the current parameter value.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::LessThanOrEqualTo(unsigned value) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto parameter_values = ConvertValuesToUnsigned();
  for (const auto& source : parameter_values) {
    if (source > value) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be less than or equal to " << value;
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the values of the parameter sum to one.
 * If the parameter is not defined, it will return the current instance if it is optional.
 */
shared_ptr<ValidatorVector> ValidatorVector::SumToOne() {
  if (!parameter_->has_been_defined() && parameter_->is_optional())
    return shared_from_this();

  auto   parameter_values = ConvertValuesToDouble();
  double sum              = std::accumulate(parameter_values.begin(), parameter_values.end(), 0.0);
  if (!utilities::math::IsOne(sum))
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " values do not sum to one. Sum is " << sum;

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is in the list of values passed in.
 * If the parameter is optional and has not been defined, it will return without checking.
 *
 * @param list An initializer list of strings that the parameter values should be in.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::IsInList(initializer_list<string> list) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto parameter_values = parameter_->values();
  for (const auto& source : parameter_values) {
    if (std::find(list.begin(), list.end(), source) == list.end()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid.";
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is less than or equal to the value of the parameter passed in as the label.
 * If the parameter is optional and has not been defined, it will return without checking.
 *
 * @param label The label of the parameter to compare against the current parameter value.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::LessThanOrEqualToParameter(const string& label) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto target_parameter = parameters_->Get(label);
  if (target_parameter == nullptr) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::GreaterThanOrEqualToParameter " << label << " does not exist in the parameter list";
  }

  vector<double> target_parameter_values;
  auto           invalid = utilities::To<double>(target_parameter->values(), target_parameter_values);
  if (!invalid.empty()) {
    LOG_CODE_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has " << invalid.size()
                     << " invalid values. This should've been picked up earlier";
  }

  auto parameter_values = ConvertValuesToDouble();
  for (const auto& target_value : target_parameter_values) {
    for (const auto& source_value : parameter_values) {
      if (source_value > target_value) {
        LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source_value << ") is invalid. Must be less than " << label << " ("
                    << target_value << ")";
      }
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is a valid model year.
 * If the parameter is optional and has not been defined, it will return without checking.
 *
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::IsModelYear() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto parameter_values = ConvertValuesToUnsigned();
  auto years            = model()->years_all();
  for (const auto& source : parameter_values) {
    if (std::find(years.begin(), years.end(), source) == years.end()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be between model start year ("
                  << *years.begin() << ") and end year (" << *years.rbegin() << ")";
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is an age in the model (between min_age and max_age inclusive)
 * If the parameter is optional and has not been defined, it will return without checking.
 *
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::IsAge() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto current_model    = model();
  auto parameter_values = ConvertValuesToUnsigned();
  for (const auto& source : parameter_values) {
    if (source < current_model->min_age() || source > current_model->max_age()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be between model min age ("
                  << current_model->min_age() << ") and max age (" << current_model->max_age() << ")";
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the values of the parameter are valid length bins. It
 * will also check that all length bin values are unique.
 *
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::IsLengthBin() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto parameter_values  = ConvertValuesToDouble();
  auto model_length_bins = model()->length_bins();
  for (const auto& val : parameter_values) {
    bool is_valid = false;

    for (const auto& bin : model_length_bins) {
      if (utilities::math::IsEqual(val, bin)) {
        is_valid = true;  // exact match
        break;
      }
    }
    if (!is_valid) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << val << ") is invalid. Must be a valid length bin";
    }
  }

  // see if we have any duplicate values in the vector
  std::set<Double> unique_values(parameter_values.begin(), parameter_values.end());
  if (unique_values.size() != parameter_values.size()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has duplicate values. Each length bin must be unique.";
  }

  return shared_from_this();
}

/**
 * This method will check if the current paramter has been defined. If it has not, then
 * it will default the parameter to all model years.
 *
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::DefaultToAllModelYears() {
  if (parameter_->has_been_defined())
    return shared_from_this();  // do nothing

  auto current_model = model();
  if (auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_)) {
    if (param->target() != nullptr && param->target()->empty()) {
      *param->target() = current_model->years();
    }
  } else {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::DefaultToAllModelYears " << parameter_->location() << " " << parameter_->label() << " is not a vector<unsigned> type";
  }

  return shared_from_this();
}

/**
 * This method will check if the current parameter has been defined. If it has not,
 * it will default the parameter to only the model estimation years (start_year to
 * final_year), never including projection years. Use this instead of
 * DefaultToAllModelYears() when projection years must not inflate the default set
 * (e.g. standardise_years in recruitment processes).
 *
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::DefaultToModelYearsOnly() {
  if (parameter_->has_been_defined())
    return shared_from_this();  // do nothing

  auto current_model = model();
  if (auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_)) {
    if (param->target() != nullptr && param->target()->empty()) {
      *param->target() = current_model->years_model();
    }
  } else {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::DefaultToModelYearsOnly " << parameter_->location() << " " << parameter_->label() << " is not a vector<unsigned> type";
  }

  return shared_from_this();
}

/**
 * This method will return a ValidatorVector that defaults to all model length bins
 * if the parameter has not been defined.
 *
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::DefaultToAllModelLengthBins() {
  if (parameter_->has_been_defined())
    return shared_from_this();  // do nothing

  auto current_model = model();
  if (current_model->length_bins().size() == 0) {
    LOG_CODE_ERROR() << "Model::length_bins() is empty. Cannot default to all model length bins for parameter " << parameter_->label()
                     << ". Please define length bins in the model.";
  }

  if (auto* param = dynamic_cast<BindableVector<double>*>(parameter_)) {
    if (param->target() != nullptr && param->target()->empty()) {
      (*param->target()).assign(current_model->length_bins().begin(), current_model->length_bins().end());
    }
  } else {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::DefaultToAllModelLengthBins " << parameter_->location() << " " << parameter_->label() << " is not a vector<Double> type";
  }

  return shared_from_this();
}

/**
 * This method will check that the number of elements in the parameter matches the count passed in.
 *
 * @param count The number of elements that the parameter should have.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::NumberOfElements(unsigned count) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  size_t actual_count = parameter_->values().size();
  if (actual_count != count) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has " << actual_count << " elements, but requires exactly " << count;
  }

  return shared_from_this();
}

/**
 * This method will check that the number of elements in the parameter matches the number of elements in the parameter with the label passed in.
 *
 * @param label The label of the parameter to compare against the current parameter value.
 * @param split_combined_categories If true, the method will expand combined categories into individual values.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::SameNumberOfElementsAs(const string& label, bool split_combined_categories) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  // Get our expected parameter and count
  auto expected_parameter = parameters_->Get(label);
  if (expected_parameter == nullptr) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::SameNumberOfElementsAs " << label << " does not exist in the parameter list";
    return shared_from_this();
  }
  auto expected_count = expected_parameter->values().size();

  // Check if the expected parameter is actuall a category holding parameter.
  // if it is, we want to expand the categories out to handle combined categories as individual values.
  if (auto* target_string = dynamic_cast<BindableVector<std::string>*>(expected_parameter)) {
    if (split_combined_categories && target_string->is_categories()) {
      expected_count = model()->categories()->total_categories_defined(expected_parameter->values());
      LOG_FINEST() << "Parameter::ValidatorVector::SameNumberOfElementsAs " << label
                   << " is a categories parameter, expanding to total categories defined. Expected count: " << expected_count;
    }
  }

  // We need to check the target size here and not just the values size because the target may have been expanded to match the expected count.
  unsigned actual_target_size = 0;
  if (auto* target_vector = dynamic_cast<BindableVector<Double>*>(parameter_)) {
    actual_target_size = target_vector->target()->size();
  } else if (auto* target_vector = dynamic_cast<BindableVector<double>*>(parameter_)) {
    actual_target_size = target_vector->target()->size();
  } else if (auto* target_vector = dynamic_cast<BindableVector<unsigned>*>(parameter_)) {
    actual_target_size = target_vector->target()->size();
  } else if (auto* target_vector = dynamic_cast<BindableVector<std::string>*>(parameter_)) {
    if (split_combined_categories && target_vector->is_categories())
      actual_target_size = model()->categories()->total_categories_defined(target_vector->values());
    else
      actual_target_size = target_vector->target()->size();

  } else {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::SameNumberOfElementsAs " << parameter_->location() << " parameter " << parameter_->label()
                     << " is not a vector<double/unsigned/string> type";
  }

  // Do our final check and now
  if (actual_target_size != expected_count) {
    std::ostringstream param_values_stream;
    for (const auto& val : parameter_->values()) {
      param_values_stream << val << " ";
    }
    std::ostringstream expected_values_stream;
    for (const auto& val : expected_parameter->values()) {
      expected_values_stream << val << " ";
    }
    LOG_FINEST() << "Parameter::ValidatorVector::SameNumberOfElementsAs " << parameter_->label() << " values: " << param_values_stream.str();
    LOG_FINEST() << "Parameter::ValidatorVector::SameNumberOfElementsAs " << label << " values: " << expected_values_stream.str();
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has " << actual_target_size << " elements, but " << label << " has " << expected_count
                << " elements";
    LOG_CODE_ERROR() << "Check";
  }

  return shared_from_this();
}

/**
 * @brief This method will expand the current parameter to have the number of elements specified by count.
 * If the parameter already has the expected count, it will do nothing.
 * If the parameter has a single value, it will duplicate that value to match the expected count
 * @param count The number of elements to expand to.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::ExpandToNumberOfElements(unsigned count) {
  LOG_TRACE();
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  // Set up a lambda to expand the parameter to the expected count
  // if it is not already the expected count.
  auto expand = [&](auto* param) {
    auto& src = *param->target();
    if (src.size() != 1 && src.size() != count) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " requires either 1 element or " << count << " elements, but has " << src.size()
                  << " elements";
      return;
    }
    if (src.size() == 1) {
      auto temp = src[0];
      src.assign(count, temp);
    }
  };

  // Call the lambda based on the type of the parameter
  // We can use dynamic_cast here because we know the parameter is a BindableVector type.
  if (auto* param = dynamic_cast<BindableVector<Double>*>(parameter_)) {
    expand(param);
  } else if (auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_)) {
    expand(param);
  } else if (auto* param = dynamic_cast<BindableVector<std::string>*>(parameter_)) {
    expand(param);
  } else {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::ExpandToNumberOfElements " << parameter_->label() << " is not a vector<double/unsigned> type";
  }

  return shared_from_this();
}

/**
 *
 */
shared_ptr<ValidatorVector> ValidatorVector::ExpandToSameNumberOfElementsAs(const string& label) {
  LOG_TRACE();
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  // Get our expected parameter and count
  auto expected_parameter = parameters_->Get(label);
  if (expected_parameter == nullptr) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::ExpandToSameNumberOfElementsAs " << label << " does not exist in the parameter list";
    return shared_from_this();
  }
  auto expected_count = expected_parameter->values().size();

  // Check if the expected parameter is actuall a category holding parameter.
  // if it is, we want to expand the categories out to handle combined categories as individual values.
  if (auto* target_string = dynamic_cast<BindableVector<std::string>*>(expected_parameter)) {
    if (target_string->is_categories()) {
      expected_count = model()->categories()->total_categories_defined(expected_parameter->values());
      LOG_FINEST() << "Parameter::ValidatorVector::ExpandToSameNumberOfElementsAs " << label
                   << " is a categories parameter, expanding to total categories defined. Expected count: " << expected_count;
    }
  }

  // Set up a lambda to expand the parameter to the expected count
  // if it is not already the expected count.
  auto expand = [&](auto* param) {
    if (param == nullptr) {
      LOG_CODE_ERROR() << "Parameter::ValidatorVector::ExpandToSameNumberOfElementsAs " << label << " is null";
      return;
    }
    auto& src = *param->target();
    if (src.size() != 1 && src.size() != expected_count) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " requires either 1 element or same number of elements as " << label << " ("
                  << expected_count << "), but has " << src.size() << " elements";
      return;
    }
    if (src.size() == 1) {
      auto temp = src[0];
      src.assign(expected_count, temp);
    }
  };

  // Call the lambda based on the type of the parameter
  // We can use dynamic_cast here because we know the parameter is a BindableVector type.
  if (auto* param = dynamic_cast<BindableVector<Double>*>(parameter_)) {
    expand(param);
  } else if (auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_)) {
    expand(param);
  } else if (auto* param = dynamic_cast<BindableVector<std::string>*>(parameter_)) {
    expand(param);
  } else {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::ExpandToSameNumberOfElementsAs " << parameter_->label() << " -> " << label << " is not a vector<double/unsigned> type";
  }

  return shared_from_this();
}

/**
 * This method will duplicate the values of another parameter into the current parameter if the current parameter has not been defined.
 * This method is modifying a parameter which has already been populated, so we will need to modify the target and not the values vector.
 *
 * This means we will need to be considerate of the source and destination types.
 *
 * @param label The label of the parameter to copy from if current parameter has not been defined
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::DuplicateParameterIfNotAssigned(const string& label) {
  // If it's already been defined we don't need to do anything
  if (parameter_->has_been_defined()) {
    return shared_from_this();
  }

  auto source = parameters_->Get(label);
  if (source == nullptr) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::DuplicateParameterIfNotAssigned " << label << " does not exist in the parameter list";
    return shared_from_this();
  }

  if (auto* param = dynamic_cast<BindableVector<Double>*>(parameter_)) {
    if (auto* param2 = dynamic_cast<BindableVector<Double>*>(source)) {
      if (param2->target() == nullptr) {
        LOG_CODE_ERROR() << "Parameter::ValidatorVector::DuplicateParameterIfNotAssigned " << label << " target is null";
      }
      param->target()->assign(param2->target()->begin(), param2->target()->end());
      return shared_from_this();
    }

  } else if (auto* param = dynamic_cast<BindableVector<unsigned>*>(parameter_)) {
    if (auto* param2 = dynamic_cast<BindableVector<unsigned>*>(parameters_->Get(label))) {
      if (param2->target() == nullptr) {
        LOG_CODE_ERROR() << "Parameter::ValidatorVector::DuplicateParameterIfNotAssigned " << label << " target is null";
      }
      param->target()->assign(param2->target()->begin(), param2->target()->end());
      return shared_from_this();
    }

    LOG_CODE_ERROR() << "Parameter::ValidatorVector::DuplicateParameterIfNotAssigned " << label << " is not a vector<unsigned> type";

  } else if (auto* param = dynamic_cast<BindableVector<std::string>*>(parameter_)) {
    if (auto* param2 = dynamic_cast<BindableVector<std::string>*>(parameters_->Get(label))) {
      if (param2->target() == nullptr) {
        LOG_CODE_ERROR() << "Parameter::ValidatorVector::DuplicateParameterIfNotAssigned " << label << " target is null";
      }
      param->target()->assign(param2->target()->begin(), param2->target()->end());
      return shared_from_this();
    }
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::DuplicateParameterIfNotAssigned " << label << " is not a vector<string> type";

  } else {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::DuplicateParameterIfNotAssigned " << parameter_->label() << " is not a vector<double/unsigned> type";
  }

  return shared_from_this();
}

/**
 * This method will check that the number of elements in the parameter is the same as the model age spread.
 * If the parameter is optional and has not been defined, it will return the current instance without validation.
 *
 * @return A shared pointer to the Validator object for method chaining.
 *
 */
shared_ptr<ValidatorVector> ValidatorVector::SameNumberOfElementsModelAgeSpread() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto current_model = model();
  if (parameter_->values().size() != current_model->age_spread()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has a different number of elements than the model age spread ("
                << current_model->age_spread() << ")";
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

  auto* param         = GetParameterAsVectorDouble();
  auto  current_model = model();
  if (param->target()->size() != current_model->length_bin_mid_points().size()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has a different number of elements than the model length bin mid points ("
                << current_model->length_bin_mid_points().size() << ")";
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

  auto check_increasing_order = [](const auto& values, const std::string& label, const std::string& location) {
    for (size_t i = 1; i < values.size(); ++i) {
      if (values[i] < values[i - 1]) {
        LOG_ERROR() << location << " parameter " << label << " values (" << values[i - 1] << ", " << values[i] << ") are not in increasing order at index " << i;
      }
    }
  };

  auto values = ConvertValuesToDouble();
  check_increasing_order(values, parameter_->label(), this->parameter_->location());

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
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::IsUniqueFrom " << parameter_->label() << " is not a vector<string> type";
  }

  const auto& values       = *param->target();
  auto*       target_param = parameters_->Get(label);
  if (target_param == nullptr) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::IsUniqueFrom " << label << " does not exist in the parameter list";
  }

  auto* target_vector = dynamic_cast<BindableVector<std::string>*>(target_param);
  if (target_vector == nullptr) {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::IsUniqueFrom " << label << " is not a vector<string> type";
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

/**
 * This method will set the default value of the parameter to a vector of doubles with the specified value and count.
 * If the parameter has already been defined, it will return the current instance without modifying the parameter.
 *
 * @param value The default value to set for each element in the vector.
 * @param count The number of elements in the vector.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<ValidatorVector> ValidatorVector::DefaultValue(double value, unsigned count) {
  if (parameter_->has_been_defined()) {
    return shared_from_this();  // do nothing
  }

  if (auto* param = dynamic_cast<BindableVector<Double>*>(parameter_)) {
    if (param->target()->size() == 0) {
      param->target()->assign(count, value);
    }
  } else {
    LOG_CODE_ERROR() << "Parameter::ValidatorVector::DefaultValue " << parameter_->label() << " is not a vector<double> type";
  }
  return shared_from_this();
}

}  // namespace niwa::parameters
