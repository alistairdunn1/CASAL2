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
    LOG_ERROR() << table_->location() << "The table " << table_->label() << " has no data rows, but expected " << count << ". " << error_message;
    return shared_from_this();
  }

  // Check if the table has the expected number of columns
  // If the table has no columns, we assume it is empty and return an error
  for (const auto& row : data) {
    for (unsigned i = 0; i < row.size(); ++i) {
      if (row[i].empty()) {
        LOG_ERROR() << table_->location() << "The table " << table_->label() << " has an empty value in row " << (i + 1) << ", column " << (i + 1) << ". " << error_message;
        return shared_from_this();
      }
    }
  }

  if (table_->column_count() != count) {
    LOG_ERROR() << table_->location() << "The table " << table_->label() << " has " << table_->column_count() << " columns, but expected " << count << ". " << error_message;
  }

  return shared_from_this();
}

}  // namespace niwa::parameters::table
