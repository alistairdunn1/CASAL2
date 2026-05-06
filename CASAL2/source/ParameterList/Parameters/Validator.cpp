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
#include "Utilities/To.h"

namespace niwa::parameters {
using niwa::parameters::Bindable;

/**
 * This method will convert the values of the parameter to a vector of doubles.
 * If the parameter is not a vector of doubles, it will error out.
 *
 * @return A vector of doubles representing the parameter values.
 */
vector<double> Validator::ConvertValuesToDouble() const {
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
vector<unsigned> Validator::ConvertValuesToUnsigned() const {
  vector<unsigned> parameter_values;
  auto             invalid = utilities::To<unsigned>(parameter_->values(), parameter_values);

  if (!invalid.empty()) {
    LOG_CODE_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " has " << invalid.size()
                     << " invalid values. This should've been picked up earlier";
  }

  return parameter_values;
}

/**
 * This method will check that the value of the parameter is greater than the value passed
 *
 * @param value The value to compare against the current parameter value.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::GreaterThan(double value) {
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
shared_ptr<Validator> Validator::GreaterThan(unsigned value) {
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
shared_ptr<Validator> Validator::GreaterThanOrEqualTo(double value) {
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
shared_ptr<Validator> Validator::GreaterThanOrEqualTo(unsigned value) {
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
 * This method will check that the value of the parameter is greater than or equal to the value of
 * the parameter passed in as the label
 *
 * @param label The label of the parameter to compare against the current parameter value.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::GreaterThanOrEqualToParameter(const string& label) {
  auto target_parameter = parameters_->Get(label);
  if (target_parameter == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::GreaterThanOrEqualToParameter " << label << " does not exist in the parameter list";
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
      if (source_value < target_value) {
        LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source_value << ") is invalid. Must be greater than or equal to "
                    << label << " (" << target_value << ")";
      }
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
shared_ptr<Validator> Validator::LessThan(double value) {
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
shared_ptr<Validator> Validator::LessThanOrEqualTo(double value) {
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
shared_ptr<Validator> Validator::LessThanOrEqualTo(unsigned value) {
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
 * This method will check that the value of the parameter is less than the value of the parameter passed in as the label.
 * If the parameter is optional and has not been defined, it will return without checking.
 *
 * @param label The label of the parameter to compare against the current parameter value.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::LessThanParameter(const string& label) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto target_parameter = parameters_->Get(label);
  if (target_parameter == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::GreaterThanOrEqualToParameter " << label << " does not exist in the parameter list";
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
      if (source_value >= target_value) {
        LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source_value << ") is invalid. Must be less than " << label << " ("
                    << target_value << ")";
      }
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
shared_ptr<Validator> Validator::LessThanOrEqualToParameter(const string& label) {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto target_parameter = parameters_->Get(label);
  if (target_parameter == nullptr) {
    LOG_CODE_ERROR() << "Parameter::Validator::GreaterThanOrEqualToParameter " << label << " does not exist in the parameter list";
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
 * This method will check that the value of the parameter is greater than or equal to the model min age.
 * If the parameter is optional and has not been defined, it will return without checking.
 *
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::GreaterThanOrEqualToModelMinAge() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto current_model    = model();
  auto parameter_values = ConvertValuesToUnsigned();
  for (const auto& source : parameter_values) {
    if (source < current_model->min_age()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source
                  << ") is invalid. Must be greater than or equal to model min age (" << current_model->min_age() << ")";
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is greater than the model min age.
 * If the parameter is optional and has not been defined, it will return without checking.
 *
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::GreaterThanModelMinAge() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto current_model    = model();
  auto parameter_values = ConvertValuesToUnsigned();
  for (const auto& source : parameter_values) {
    if (source <= current_model->min_age()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source
                  << ") is invalid. Must be greater than or equal to model min age (" << current_model->min_age() << ")";
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is less than or equal to the model max age.
 * If the parameter is optional and has not been defined, it will return without checking.
 *
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::LessThanOrEqualToModelMaxAge() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto current_model    = model();
  auto parameter_values = ConvertValuesToUnsigned();
  for (const auto& source : parameter_values) {
    if (source > current_model->max_age()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be less than or equal to model max age ("
                  << current_model->max_age() << ")";
    }
  }

  return shared_from_this();
}

/**
 * This method will check that the value of the parameter is less than the model max age
 * If the parameter is optional and has not been defined, it will return without checking.
 *
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::LessThanModelMaxAge() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto current_model    = model();
  auto parameter_values = ConvertValuesToUnsigned();
  for (const auto& source : parameter_values) {
    if (source >= current_model->max_age()) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be less than model max age ("
                  << current_model->max_age() << ")";
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
shared_ptr<Validator> Validator::IsAge() {
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
 * This method will check that the value of the parameter is in the list of values passed in.
 * If the parameter is optional and has not been defined, it will return without checking.
 *
 * @param list An initializer list of strings that the parameter values should be in.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::IsInList(initializer_list<string> list) {
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
 * This method will duplicate the value of the parameter if it has not been assigned yet.
 * It will look for the parameter with the label passed in and copy its value to the current parameter.
 *
 * @param label The label of the parameter to duplicate the value from.
 * @return A shared pointer to the Validator object for method chaining.
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

/**
 * This method will check that the value of the parameter is a valid model year.
 * If the parameter is optional and has not been defined, it will return without checking.
 *
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::IsModelYear() {
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
 * @brief This method will check that the value of the parameter is a valid projection year.
 * If the parameter is optional and has not been defined, it will return without checking.
 */
shared_ptr<Validator> Validator::IsProjectionYear() {
  if (!parameter_->has_been_defined() && parameter_->is_optional()) {
    return shared_from_this();
  }

  auto     parameter_values      = ConvertValuesToUnsigned();
  auto     current_model         = model();
  unsigned projection_start_year = current_model->projection_start_year();
  unsigned projection_final_year = current_model->projection_final_year();

  for (const auto& source : parameter_values) {
    if (source < projection_start_year || source > projection_final_year) {
      LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " value (" << source << ") is invalid. Must be between model projection start year ("
                  << projection_start_year << ") and end year (" << projection_final_year << ")";
    }
  }

  return shared_from_this();
}

/**
 * This method will set the default value of the parameter to the value passed in.
 * If the parameter has already been defined, it will return without doing anything.
 *
 * @param value The default value to set for the parameter.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::DefaultValue(unsigned value) {
  if (parameter_->has_been_defined()) {
    return shared_from_this();
  }

  auto* param = dynamic_cast<Bindable<unsigned>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " cannot set default value because it is not an unsigned type";
  }

  *param->target() = value;

  return shared_from_this();
}

/**
 * This method will set the default value of the parameter to the value passed in.
 * If the parameter has already been defined, it will return without doing anything.
 *
 * @param value The default value to set for the parameter.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::DefaultValue(bool value) {
  if (parameter_->has_been_defined()) {
    return shared_from_this();
  }

  auto* param = dynamic_cast<Bindable<bool>*>(parameter_);
  if (param == nullptr) {
    LOG_CODE_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " cannot set default value because it is not an boolean type";
  }

  *param->target() = value;

  return shared_from_this();
}

/**
 * This method will check if the parameter is true or false, if the parameter is true then
 * the current parameter is required and must have been defined with values.
 */
shared_ptr<Validator> Validator::RequiredIf(bool required) {
  if (parameter_->has_been_defined()) {
    return shared_from_this();
  }

  if (required && !parameter_->has_been_defined() && !parameter_->values().empty()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " is required but has not been defined.";
  } else {
    LOG_WARNING() << this->parameter_->location() << " parameter " << parameter_->label() << " is optional and has not been defined.";
  }

  return shared_from_this();
}

/**
 * This method will check if the parameter defined by the parameter label has been defined. If it
 * has then the current parameter is forbidden to be defined.
 *
 * @param label The label of the parameter that is checked to see if it has been defined.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::ForbiddenIfDefined(const string& label) {
  if (parameter_->has_been_defined() && parameters_->Get(label)->has_been_defined()) {
    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " is forbidden because " << label << " has been defined.";
  }

  return shared_from_this();
}

/**
 * This method will check if the current parameter or the parameter defined by the label has been defined.
 * If neither has been defined, it will log an error.
 *
 * @param label The label of the parameter that is checked to see if it has been defined.
 * @return A shared pointer to the Validator object for method chaining.
 */
shared_ptr<Validator> Validator::EitherOrDefined(const string& label) {
  auto* param = parameters_->Get(label);
  if (!param->has_been_defined() && !parameter_->has_been_defined()) {
    if (parameter_->is_optional()) {
      // If the parameter is optional and not defined, we can return without error
      return shared_from_this();
    }

    LOG_ERROR() << this->parameter_->location() << " parameter " << parameter_->label() << " or " << label << " must be defined in the model";
  }

  return shared_from_this();
}

}  // namespace niwa::parameters
