/**
 * @file ValidatorVector.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/06/16
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#include "ValidatorTable.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <numeric>

#include "Categories/Categories.h"
#include "Logging/Logging.h"
#include "Model/Model.h"
#include "ParameterList/ParameterList.h"
#include "Utilities/Math.h"
#include "Utilities/To.h"

namespace niwa::parameters::table {

/**
 * This method will validate that the table has the specified number of rows.
 * If the table does not have the specified number of rows it will throw an error.
 *
 * @param count The number of rows to validate against
 * @param error_message An optional error message to include in the log if validation fails
 *                      This can be used to provide additional context for the error.
 * @return A shared pointer to the ValidatorTable object for method chaining
 */
shared_ptr<ValidatorTable> ValidatorTable::Rows(unsigned count, const string& error_message) {
  if (table_ == nullptr)
    LOG_CODE_ERROR() << "table_ is nullptr";

  if (table_->data().size() != count) {
    LOG_ERROR() << table_->location() << "The table " << table_->label() << " has " << table_->row_count() << " rows, but expected " << count << ". " << error_message;
  }

  return shared_from_this();
}

/**
 * This method will grow a table's columns to a specified number (grow_to + 1) if there are no more
 * columns than at base_index. That is if the table only has one data column, it will grow until
 * there are grow_to columns, duplicating the value in the base_index column.addressable
 *
 * For example
 * @table error_values
 * 2000 0.1
 * 2002 0.2
 * @end_table
 *
 * With grow_to = 3 and base_index = 1, the table will be expanded to: 2000 0.1 0.1 0.1, 2002 0.2 0.2 0.2.
 *
 * @param grow_to The number of data columns to grow the table to
 * @param base_index The index of the column to duplicate values from
 * @return A shared pointer to the ValidatorTable object for method chaining
 */
shared_ptr<ValidatorTable> ValidatorTable::ExpandColumnsTo(unsigned grow_to, unsigned base_index) {
  LOG_FINEST() << "Expanding columns to " << grow_to << " with base index " << base_index;

  if (table_ == nullptr)
    LOG_CODE_ERROR() << "table_ is nullptr";

  auto& data = table_->data();
  if (data.empty()) {
    LOG_ERROR() << table_->location() << "The table " << table_->label() << " has no data rows, but expected to expand columns to " << grow_to;
    return shared_from_this();
  }

  // Check if the table has enough columns to expand
  if (base_index >= data[0].size()) {
    LOG_ERROR() << table_->location() << "The table " << table_->label() << " does not have a column at index " << base_index
                << ". We expected it to contain values to expand columns to " << grow_to;
    return shared_from_this();
  }

  // Do nothing if we already have more columns than 1 data column
  if (data[0].size() > 2) {
    LOG_FINEST() << "The table " << table_->label() << " already has more than 2 columns, no expansion possible. Has " << data[0].size() << " columns.";
    return shared_from_this();
  }

  // Expand the columns
  for (auto& row : data) {
    auto duplicate_value = row[base_index];
    LOG_FINEST() << "Expanding row from " << row.size() << " to " << (grow_to + 1) << " columns, duplicating value '" << duplicate_value << "' from base index " << base_index;
    while (row.size() < grow_to + 1) {
      row.push_back(duplicate_value);
    }
  }

  return shared_from_this();
}

/**
 * This method will validate that the table has the specified number of columns.
 *  If the table does not have the specified number of columns it will throw an error.
 *
 * @param count The number of columns to validate against
 * @param error_message An optional error message to include in the log if validation fails
 *                      This can be used to provide additional context for the error.
 * @return A shared pointer to the ValidatorTable object for method chaining
 */
shared_ptr<ValidatorTable> ValidatorTable::Columns(unsigned count, const string& error_message) {
  if (table_ == nullptr)
    LOG_CODE_ERROR() << "table_ is nullptr";

  auto data = table_->data();
  if (data.empty()) {
    LOG_ERROR() << table_->location() << "has no data rows, but expected " << count << ". " << error_message;
    return shared_from_this();
  }

  // Check if the table has the expected number of columns If the table has no columns, we assume it is empty and return an error
  for (const auto& row : data) {
    if (row.size() != count) {
      LOG_ERROR() << table_->location() << "has " << row.size() << " columns, but expected " << count << ". " << error_message;
      return shared_from_this();
    }
  }

  return shared_from_this();
}

/**
 * This method will validate that the specified column index contains year values.
 * If the column does not contain year values it will throw an error.
 *
 * @param column_index The index of the column to validate
 * @param error_message An optional error message to include in the log if validation fails
 *                      This can be used to provide additional context for the error.
 * @return A shared pointer to the ValidatorTable object for method chaining
 */
shared_ptr<ValidatorTable> ValidatorTable::ColumnIsYear(unsigned column_index, const string& error_message) {
  if (table_ == nullptr)
    LOG_CODE_ERROR() << "table_ is nullptr";

  auto data = table_->data();
  for (const auto& row : data) {
    if (column_index >= row.size()) {
      LOG_ERROR() << table_->location() << "The table " << table_->label() << " does not have a column at index " << column_index << ". We expected it to contain year values.";
      return shared_from_this();
    }

    auto     model_years = model_->years();
    unsigned year        = 0;
    if (!utilities::To<unsigned>(row[column_index], year)) {
      LOG_ERROR() << table_->location() << "The table " << table_->label() << " column at index " << column_index << " does not contain a valid year value: '" << row[column_index]
                  << "'. " << error_message;
      return shared_from_this();
    }
    if (std::find(model_years.begin(), model_years.end(), year) == model_years.end()) {
      LOG_ERROR() << table_->location() << "The table " << table_->label() << " column at index " << column_index << " contains a year value '" << year
                  << "' that is not a valid model year. " << error_message;
      return shared_from_this();
    }
  }

  return shared_from_this();
}

/**
 * This method will validate that the specified range of columns contains double values.
 * If the columns do not contain double values it will throw an error.
 *
 * @param start_index The starting index of the range of columns to validate
 * @param number_of_columns The number of columns in the range to validate
 * @param error_message An optional error message to include in the log if validation fails
 *                      This can be used to provide additional context for the error.
 * @return A shared pointer to the ValidatorTable object for method chaining
 */
shared_ptr<ValidatorTable> ValidatorTable::DoubleDataRange(unsigned start_index, unsigned number_of_columns, const string& error_message) {
  if (table_ == nullptr)
    LOG_CODE_ERROR() << "table_ is nullptr";

  auto data = table_->data();
  for (const auto& row : data) {
    if (start_index >= row.size() || (start_index + number_of_columns - 1) >= row.size()) {
      LOG_ERROR() << table_->location() << "The table " << table_->label() << " does not have columns in the range [" << start_index << ", "
                  << (start_index + number_of_columns - 1) << "). We expected it to contain double values. " << error_message;
      return shared_from_this();
    }

    for (unsigned i = 0; i < number_of_columns; ++i) {
      unsigned i_index = start_index + i;
      double   value   = 0.0;
      if (!utilities::To<double>(row[i_index], value)) {
        LOG_ERROR() << table_->location() << "The table " << table_->label() << " column at index " << i_index << " does not contain a valid double value: '" << row[i_index]
                    << "'. " << error_message;
        return shared_from_this();
      }
    }
  }

  return shared_from_this();
}

/**
 *  This method will validate that the specified column index contains values greater than a specified value.
 *  If the column does not contain values greater than the specified value it will throw an error.
 * @param column_index The index of the column to validate
 * @param value The value to compare against
 * @param error_message An optional error message to include in the log if validation fails
 *                     This can be used to provide additional context for the error.
 * @return A shared pointer to the ValidatorTable object for method chaining
 */
shared_ptr<ValidatorTable> ValidatorTable::GreaterThan(unsigned column_index, double value, const string& error_message) {
  if (table_ == nullptr)
    LOG_CODE_ERROR() << "table_ is nullptr";

  auto data = table_->data();
  for (const auto& row : data) {
    if (column_index >= row.size()) {
      LOG_ERROR() << table_->location() << "The table " << table_->label() << " does not have a column at index " << column_index
                  << ". We expected it to contain values greater than " << value << ". " << error_message;
      return shared_from_this();
    }

    double column_value = 0.0;
    if (!utilities::To<double>(row[column_index], column_value)) {
      LOG_ERROR() << table_->location() << "The table " << table_->label() << " column at index " << column_index << " does not contain a valid double value: '"
                  << row[column_index] << "'. It should be greater than " << value << ". " << error_message;
      return shared_from_this();
    }

    if (column_value <= value) {
      LOG_ERROR() << table_->location() << "The table " << table_->label() << " column at index " << column_index << " contains a value '" << column_value
                  << "' that is less than or equal to the specified value '" << value << "'. It should be greater than " << value << ". " << error_message;
      return shared_from_this();
    }
  }

  return shared_from_this();
}

/**
 * This method will validate that the specified range of columns contains values greater than a specified value.
 * If the columns do not contain values greater than the specified value it will throw an error.
 * @param start_column_index The starting index of the range of columns to validate
 * @param number_of_columns The number of columns in the range to validate
 * @param value The value to compare against
 * @param error_message An optional error message to include in the log if validation fails
 *                     This can be used to provide additional context for the error.
 * @return A shared pointer to the ValidatorTable object for method chaining
 */
shared_ptr<ValidatorTable> ValidatorTable::GreaterThanForRange(unsigned start_column_index, unsigned number_of_columns, double value, const string& error_message) {
  if (table_ == nullptr)
    LOG_CODE_ERROR() << "table_ is nullptr";

  auto data = table_->data();
  for (const auto& row : data) {
    if (start_column_index >= row.size() || (start_column_index + number_of_columns - 1) >= row.size()) {
      LOG_ERROR() << table_->location() << "The table " << table_->label() << " does not have columns in the range [" << start_column_index << ", "
                  << (start_column_index + number_of_columns - 1) << "). Actual row size was " << row.size() << ". " << error_message;
      return shared_from_this();
    }

    for (unsigned i = 0; i < number_of_columns; ++i) {
      unsigned column_index = start_column_index + i;
      if (column_index >= row.size()) {
        LOG_ERROR() << table_->location() << "The table " << table_->label() << " does not have a column at index " << column_index
                    << ". We expected it to contain values greater than " << value << ". " << error_message;
        return shared_from_this();
      }
      double column_value = 0.0;
      if (!utilities::To<double>(row[column_index], column_value)) {
        LOG_ERROR() << table_->location() << "The table " << table_->label() << " column at index " << column_index << " does not contain a valid double value: '"
                    << row[column_index] << "'. It should be greater than " << value << ". " << error_message;
        return shared_from_this();
      }

      if (column_value <= value) {
        LOG_ERROR() << table_->location() << "The table " << table_->label() << " column at index " << column_index << " contains a value '" << column_value
                    << "' that is less than or equal to the specified value '" << value << "'. It should be greater than " << value << ". " << error_message;
        return shared_from_this();
      }
    }
  }

  return shared_from_this();
}

/**
 * This method will validate that the specified range of columns contains values greater than a specified value.
 * If the columns do not contain values greater than the specified value it will throw an error.
 * @param start_column_index The starting index of the range of columns to validate
 * @param number_of_columns The number of columns in the range to validate
 * @param value The value to compare against
 * @param error_message An optional error message to include in the log if validation fails
 *                     This can be used to provide additional context for the error.
 * @return A shared pointer to the ValidatorTable object for method chaining
 */
shared_ptr<ValidatorTable> ValidatorTable::GreaterThanOrEqualToForRange(unsigned start_column_index, unsigned number_of_columns, double value, const string& error_message) {
  if (table_ == nullptr)
    LOG_CODE_ERROR() << "table_ is nullptr";

  auto data = table_->data();
  for (const auto& row : data) {
    if (start_column_index >= row.size() || (start_column_index + number_of_columns - 1) >= row.size()) {
      LOG_ERROR() << table_->location() << "The table " << table_->label() << " does not have columns in the range [" << start_column_index << ", "
                  << (start_column_index + number_of_columns - 1) << "). Actual row size was " << row.size() << ". " << error_message;
      return shared_from_this();
    }

    for (unsigned i = 0; i < number_of_columns; ++i) {
      unsigned column_index = start_column_index + i;
      if (column_index >= row.size()) {
        LOG_ERROR() << table_->location() << "The table " << table_->label() << " does not have a column at index " << column_index
                    << ". We expected it to contain values greater than " << value << ". " << error_message;
        return shared_from_this();
      }
      double column_value = 0.0;
      if (!utilities::To<double>(row[column_index], column_value)) {
        LOG_ERROR() << table_->location() << "The table " << table_->label() << " column at index " << column_index << " does not contain a valid double value: '"
                    << row[column_index] << "'. It should be greater than " << value << ". " << error_message;
        return shared_from_this();
      }

      if (column_value < value) {
        LOG_ERROR() << table_->location() << "The table " << table_->label() << " column at index " << column_index << " contains a value '" << column_value
                    << "' that is less than or equal to the specified value '" << value << "'. It should be greater than " << value << ". " << error_message;
        return shared_from_this();
      }
    }
  }

  return shared_from_this();
}

/**
 *  This method will validate that the specified column index contains values less than or equal to a specified value.
 *  If the column does not contain values less than or equal to than the specified value it will throw an error.
 * @param start_column_index The starting index of the range of columns to validate
 * @param number_of_columns The number of columns in the range to validate
 * @param value The value to compare against
 * @param error_message An optional error message to include in the log if validation fails
 *                     This can be used to provide additional context for the error.
 * @return A shared pointer to the ValidatorTable object for method chaining
 */
shared_ptr<ValidatorTable> ValidatorTable::LessThanOrEqualToForRange(unsigned start_column_index, unsigned number_of_columns, double value, const string& error_message) {
  if (table_ == nullptr)
    LOG_CODE_ERROR() << "table_ is nullptr";

  auto data = table_->data();
  for (const auto& row : data) {
    if (start_column_index >= row.size() || (start_column_index + number_of_columns - 1) >= row.size()) {
      LOG_ERROR() << table_->location() << "The table " << table_->label() << " does not have columns in the range [" << start_column_index << ", "
                  << (start_column_index + number_of_columns - 1) << "). Actual row size was " << row.size() << ". " << error_message;
      return shared_from_this();
    }

    for (unsigned i = 0; i < number_of_columns; ++i) {
      unsigned column_index = start_column_index + i;
      if (column_index >= row.size()) {
        LOG_ERROR() << table_->location() << "The table " << table_->label() << " does not have a column at index " << column_index
                    << ". We expected it to contain values greater than " << value << ". " << error_message;
        return shared_from_this();
      }
      double column_value = 0.0;
      if (!utilities::To<double>(row[column_index], column_value)) {
        LOG_ERROR() << table_->location() << "The table " << table_->label() << " column at index " << column_index << " does not contain a valid double value: '"
                    << row[column_index] << "'. It should be greater than " << value << ". " << error_message;
        return shared_from_this();
      }

      if (column_value > value) {
        LOG_ERROR() << table_->location() << "The table " << table_->label() << " column at index " << column_index << " contains a value '" << column_value
                    << "' that is greater than the specified value '" << value << "'. It should be less than or equal to " << value << ". " << error_message;
        return shared_from_this();
      }
    }
  }

  return shared_from_this();
}

}  // namespace niwa::parameters::table
