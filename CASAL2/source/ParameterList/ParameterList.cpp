/**
 * @file CParameterList.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 20/02/2012
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "ParameterList.h"

#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../Categories/Categories.h"
#include "../Logging/Logging.h"
#include "../Model/Model.h"
#include "../Translations/Translations.h"
#include "../Utilities/String.h"
#include "../Utilities/To.h"

// Using
namespace util = niwa::utilities;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

namespace niwa {

/**
 * Destructor
 */
ParameterList::~ParameterList() {
  for (auto parameter : parameters_) {
    if (parameter.second != nullptr)
      delete parameter.second;
  }
  // DO NOT CLEAN UP TABLE MEMORY. IT'S HANDLE BY THE OWNER
}

/**
 * Add a single value to an existing parameter on the parameter list
 *
 * @param label The label for the parameter
 * @param value The value of the parameter
 * @param file_name The file where the parameter was specified
 * @param line_number The line number where the parameter was specified
 * @return true on success, false on failure
 */
bool ParameterList::Add(const string& label, const vector<string>& values, const string& file_name, const unsigned& line_number) {
  auto iter = Get(label);
  if (iter == nullptr) {
    return false;
  }

  iter->set_values(values);
  iter->set_file_name(file_name);
  iter->set_line_number(line_number);

  return true;
}

/**
 * Append a single value to the parameter list
 *
 * @param label The label for the parameter
 * @param value The value of the parameter
 * @param file_name The file where the parameter was specified
 * @param line_number The line number where the parameter was specified
 * @return true on success, false on failure
 */
bool ParameterList::Add(const string& label, const string& value, const string& file_name, const unsigned& line_number) {
  LOG_FINEST() << "Adding " << label << ", " << value << ", " << file_name << ", " << line_number;
  vector<string> values = {value};
  return this->Add(label, values, file_name, line_number);
}

/**
 * Add a new table to the parameter list
 *
 * @param label The label for the table
 * @param columns A vector containing the columns
 * @param data A double vector containing the data
 * @param file_name Name of file where table definition finished
 * @param line_number Line number where table definition finished
 * @return true on success, false on failure
 */

/**
 * Populate the parameter list
 *
 * @param model The model
 */
void ParameterList::Populate(shared_ptr<Model> model) {
  LOG_TRACE();
  model_ = model;
  if (already_populated_) {
    LOG_CODE_ERROR() << "  if (already_populated_): " << parent_block_type_;
  }
  if (ignore_all_parameters_)
    return;

  already_populated_ = true;
  /**
   * go through and look for missing required parameters
   */
  string missing_parameters = "";
  for (auto iter = parameters_.begin(); iter != parameters_.end(); ++iter) {
    if ((iter->second->values().size() == 0 && !iter->second->is_optional() && !iter->second->is_deprecated())
        && (iter->second->partition_type() == PartitionType::kModel || iter->second->partition_type() == model->partition_type()))
      missing_parameters += iter->first + " ";
  }
  for (auto iter = tables_.begin(); iter != tables_.end(); ++iter) {
    if (iter->second->data().size() == 0 && !iter->second->is_optional())
      missing_parameters += iter->first + "(Table) ";
  }

  if (missing_parameters != "") {
    if (parameters_.find(PARAM_LABEL) == parameters_.end()) {
      LOG_ERROR() << "At line " << defined_line_number_ << " in " << defined_file_name_ << " the following required parameters for the command @" << parent_block_type_
                  << " are required but have not been defined: " << missing_parameters;
    } else {
      auto parameter = parameters_.find(PARAM_LABEL);
      if (parameter->second->values().size() == 0) {
        LOG_ERROR() << "At line " << defined_line_number_ << " in " << defined_file_name_ << " the following required parameters for the command @" << parent_block_type_
                    << " are required but have not been defined: " << missing_parameters;
      } else {
        LOG_ERROR() << parameter->second->location() << " the following required parameters for the command @" << parent_block_type_
                    << " are required but have not been defined: " << missing_parameters;
      }
    }
    return;
  }

  /*
   * Handle expansion of the categories here if it's not already been done.
   * e.g. male,female.immature,mature would be:
   * male.immature male.mature female.immature female.mature
   */
  for (auto iter : parameters_) {
    string label = iter.first;

    bool                    is_category     = false;
    bool                    allow_combined  = true;  // TODO: Change to false once parameter upgrae is done
    BindableVector<string>* bindable_vector = dynamic_cast<BindableVector<string>*>(iter.second);
    if (bindable_vector != nullptr) {
      is_category    = bindable_vector->is_categories();
      allow_combined = bindable_vector->allow_combined_categories();
    }

    vector<string> possible_categories = {PARAM_CATEGORIES,
                                          PARAM_FROM,
                                          PARAM_TO,
                                          PARAM_PREY_CATEGORIES,
                                          PARAM_PREDATOR_CATEGORIES,
                                          PARAM_TOTAL_CATEGORIES,
                                          PARAM_TAGGED_CATEGORIES,
                                          PARAM_TARGET_CATEGORIES,
                                          PARAM_INDIVIDUAL_CATEGORIES,
                                          PARAM_NUMERATOR_CATEGORIES};

    if (is_category || std::find(possible_categories.begin(), possible_categories.end(), label) != possible_categories.end()) {
      LOG_FINE() << "Expanding category name values for " << label << " at " << iter.second->location();
      LOG_FINE() << "Expanding " << boost::join(iter.second->values(), " ");
      vector<string> expanded_values = model->categories()->ExpandLabels(iter.second->values(), iter.second->location());
      iter.second->set_values(expanded_values);
      LOG_FINE() << "Expanded to: " << boost::join(expanded_values, " ");

      /**
       * Check to see if each category is value. Handle + syntax by breaking it up
       * e.g. male+female would be checked as male, then female to function.
       */
      for (const string& category_groups : iter.second->values()) {
        if (!allow_combined && model_->categories()->IsCombinedLabels(category_groups)) {
          LOG_FATAL() << iter.second->location() << ": the parameter '" << label << "' does not allow combined categories, but the value '" << category_groups
                      << "' contains a '+' character. Please remove the '+' character from the value.";
        }

        vector<string> plus_split_categories;
        boost::split(plus_split_categories, category_groups, boost::is_any_of("+"));
        for (string& single_category : plus_split_categories) {
          if (!model->categories()->IsValid(single_category)) {
            LOG_FATAL() << iter.second->location() << ": the parameter '" << label << "' contains an invalid category '" << single_category
                        << "'. Please check the categories defined in your model.";
          }
        }
      }
    }
  }

  // Check for deprecated parameters
  for (auto iter = parameters_.begin(); iter != parameters_.end(); ++iter) {
    if (iter->second->values().size() > 0 && iter->second->is_deprecated()) {
      if (iter->second->deprecated_replacement() != "") {
        LOG_ERROR() << iter->second->location() << " the parameter '" << iter->first << "' is deprecated and should be replaced with '" << iter->second->deprecated_replacement()
                    << "'. Please update your model.";
      } else {
        // No replacement, just log an error
        LOG_ERROR() << iter->second->location() << " the parameter '" << iter->first << "' is deprecated and should not be used in Casal2. Please remove it from your model.";
      }
    }
  }

  // NOTE: This has to be last
  // bind parameters
  LOG_FINEST() << "Binding parameters for @" << parent_block_type_ << " defined at line " << defined_line_number_ << " in " << defined_file_name_;
  for (auto iter = parameters_.begin(); iter != parameters_.end(); ++iter) {
    if (iter->second->values().size() == 0 && !iter->second->is_optional())
      continue;
    LOG_FINEST() << "Binding: " << iter->first;
    iter->second->Bind();
  }
  LOG_FINEST() << "Binding complete";

  LOG_FINEST() << "Doing Partition Type Checks";
  if (parameters_.find(PARAM_PARTITION_TYPE) != parameters_.end()) {
    Parameter* param = parameters_[PARAM_PARTITION_TYPE];
    if (param->values().size() != 0) {
      string        temp           = parameters_.find(PARAM_PARTITION_TYPE)->second->values()[0];
      PartitionType partition_type = PartitionType::kInvalid;
      if (!utilities::To<PartitionType>(temp, partition_type))
        LOG_FATAL() << "X";
      bool using_model_partition_type = partition_type == PartitionType::kModel;

      for (auto& iter : parameters_) {
        if (iter.second->partition_type() != partition_type) {
          if (using_model_partition_type) {
            LOG_ERROR() << iter.second->location() << " cannot be defined with the current model partition type defined at " << model->location();
          } else {
            LOG_ERROR() << iter.second->location() << " cannot be defined with the current partition_type parameter";
          }
        }
      }
    }
  }
  // Set subcommand type to lower case
  if (parameters_.find(PARAM_TYPE) != parameters_.end()) {
    Parameter* param = parameters_[PARAM_TYPE];
    if (param->values().size() != 0) {
      param->set_value(utilities::ToLowercase(param->values()[0]));
    }
  }
  if (parameters_.find(PARAM_LABEL) != parameters_.end()) {
    Parameter* param = parameters_[PARAM_LABEL];
    if (param->values().size() != 0) {
      string invalid = utilities::String::find_invalid_characters(param->values()[0]);
      if (invalid != "")
        LOG_ERROR() << param->location() << " the label '" << param->values()[0] << "' contains the following invalid characters: " << invalid;
    }
  }

  LOG_FINEST() << "Populating Tables";
  for (auto table : tables_) table.second->Populate(model);

  LOG_FINEST() << "Populate complete";
}

/**
 * Return a constant reference to one of the parameter objects.
 *
 * NOTE: This method MUST be called with a valid label otherwise
 * a reference to an empty parameter will be returned.
 *
 * @param label The label of the parameter to return
 * @return The parameter reference
 */
Parameter* ParameterList::Get(const string& label) {
  auto iter = parameters_.find(label);
  if (iter == parameters_.end()) {
    // Check for aliases
    for (const auto& param : parameters_) {
      if (std::find(param.second->alias_labels().begin(), param.second->alias_labels().end(), label) != param.second->alias_labels().end()) {
        return param.second;
      }
    }
    return nullptr;
  }

  return iter->second;
}

/**
 * Return a constant pointer to one of the parameter tables
 *
 * NOTE: This method MUST be called with a valid label otherwise
 * a reference to an empty parameter will be returned.
 *
 * @param label The label of the table to return
 * @return The parameter reference
 */
parameters::Table* ParameterList::GetTable(const string& label) {
  auto iter = tables_.find(label);
  if (iter == tables_.end())
    return nullptr;

  return iter->second;
}

/**
 * This method copies all of the parameters from
 * the source parameter list into this parameter list.
 *
 * NOTE: The TablesPtr are not recreated.
 *
 * @param source The source parameter list
 * @param parameter_label The parameter to copy over
 */
void ParameterList::CopyFrom(const ParameterList& source, string parameter_label) {
  LOG_TRACE();
  this->defined_file_name_   = source.defined_file_name_;
  this->defined_line_number_ = source.defined_line_number_;
  this->parent_block_type_   = source.parent_block_type_;

  auto iter = source.parameters_.find(parameter_label);
  if (iter == source.parameters_.end()) {
    LOG_CODE_ERROR() << "iter == source.parameters_.end() for label: " << parameter_label;
  }

  Add(parameter_label, iter->second->values(), iter->second->file_name(), iter->second->line_number());
}

/**
 * This method copies all of the parameters from
 * the source parameter list into this parameter list.
 *
 * NOTE: The TablesPtr are not recreated.
 *
 * @param source The source parameter list
 * @param parameter_label The parameter to copy over
 * @param value_index
 */
void ParameterList::CopyFrom(const ParameterList& source, string parameter_label, const unsigned& value_index) {
  LOG_TRACE();
  auto iter = source.parameters_.find(parameter_label);
  if (iter == source.parameters_.end())
    LOG_CODE_ERROR() << "iter == source.parameters_.end() for label: " << parameter_label;
  if (iter->second->values().size() == 0)
    return;

  vector<string> values;
  if (iter->second->values().size() <= value_index)
    LOG_CODE_ERROR() << "iter->second->values().size(" << iter->second->values().size() << ") <= value_index(" << value_index << "): " << parameter_label;

  values.push_back(iter->second->values()[value_index]);
  Add(parameter_label, values, iter->second->file_name(), iter->second->line_number());
}

/**
 * Clear the parameter list
 */
void ParameterList::Clear() {
  auto iter = parameters_.begin();
  for (; iter != parameters_.end(); ++iter) {
    iter->second->Clear();
  }

  tables_.clear();
}

/**
 * Find the location string for one of our parameters.
 *
 * @param label The label for the parameter
 * @return The location string for an error message
 */
string ParameterList::location(const string& label) {
  map<string, Parameter*>::iterator iter       = parameters_.find(label);
  auto                              table_iter = tables_.find(label);
  if (iter == parameters_.end() && table_iter == tables_.end()) {
    LOG_CODE_ERROR() << "Trying to find the configuration file location for the parameter " << label
                     << " failed because it has not been previously bound to this object. This is a developer"
                     << " error most likely caused by using mismatched PARAM_X values";
  }

  if (iter != parameters_.end())
    return iter->second->location() + " the parameter '" + label + "' ";

  return table_iter->second->location();
}

/**
 * Find the location string for one of our parameters, but write a quiet string.
 *
 * @param label The label for the parameter
 * @return The location string for a quiet error message
 */
string ParameterList::quiet_location(const string& label) {
  map<string, Parameter*>::iterator iter       = parameters_.find(label);
  auto                              table_iter = tables_.find(label);
  if (iter == parameters_.end() && table_iter == tables_.end()) {
    LOG_CODE_ERROR() << "Trying to find the configuration file location for the parameter " << label
                     << " failed because it has not been previously bound to this object. This is a developer"
                     << " error most likely caused by using mismatched PARAM_X values";
  }

  if (iter != parameters_.end())
    return iter->second->location() + ": ";

  return table_iter->second->location();
}

/**
 * Bind a table pointer to the map so it can be recognised and retrieved by the configuration loader
 *
 * @param label The label of the table to bind
 * @param table The pointer to the table
 * @param description Information used for documentation, ignored
 * @param values Information used for documentation, ignored
 */
void ParameterList::BindTable(const string& label, parameters::Table* table, const string& description, const string& values, bool requires_columns, bool optional) {
  table->set_requires_columns(requires_columns);
  table->set_is_optional(optional);
  tables_[label] = table;
}

shared_ptr<Validator> ParameterList::Validate(const string& label) {
  auto it = Get(label);
  if (it == nullptr) {
    LOG_CODE_ERROR() << "The parameter " << label << " has not been bound to " << parent_block_type_ << " at " << defined_file_name_ << ":" << defined_line_number_;
  }
  return std::make_shared<Validator>(model_, this, it);
}

shared_ptr<ValidatorVector> ParameterList::ValidateVector(const string& label) {
  auto it = Get(label);
  if (it == nullptr) {
    LOG_CODE_ERROR() << "The parameter " << label << " has not been bound to " << parent_block_type_ << " at " << defined_file_name_ << ":" << defined_line_number_;
  }
  return std::make_shared<ValidatorVector>(model_, this, it);
}

/**
 *
 */
void ParameterList::Unbind(const string& label) {
  auto iter = parameters_.find(label);
  if (iter != parameters_.end()) {
    delete iter->second;
    parameters_.erase(iter);
  } else {
    LOG_CODE_ERROR() << "Parameter " << label << " not found in ParameterList.";
  }
}  // namespace niwa

} /* namespace niwa */
