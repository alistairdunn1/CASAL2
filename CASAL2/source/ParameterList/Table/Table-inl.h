/**
 * @file Table-inl.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 12/10/2018
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/�2018 - www.niwa.co.nz
 *
 */
#ifndef SOURCE_PARAMETERS_TABLE_TABLE_INL_H_
#define SOURCE_PARAMETERS_TABLE_TABLE_INL_H_

#include <vector>

#include "Utilities/To.h"
#include "Utilities/Types.h"

// namespaces
namespace niwa::parameters::table {

using std::vector;

/**
 * This method will run through all of the data in a specific column ensuring every value is numeric.
 * It'll throw up appropriate error if not. This makes validation of tables easier.
 *
 * @param column The column header to check data for
 */
template <typename T>
void Table::CheckColumnValuesAreType(const string& column) {
  unsigned index = column_index(column);
  T        value;

  for (const auto& row : data_) {
    if (!utilities::To<string, T>(row[index], value))
      LOG_ERROR() << location() << "The value" << row[index] << " in column " << column << " could not be converted to type " << utilities::demangle(typeid(value).name());
  }
}

/**
 * This method will go through every value in a column and check to ensure it's present
 * in the values parameter being passed in. This will easily allow us to pass in a set
 * of values to be verified.
 *
 * @param column The label for the column to process
 * @param values A vector of values to check each column value against
 */
template <typename T>
void Table::CheckColumnValuesContain(const string& column, const vector<T>& values) {
  unsigned  index = column_index(column);
  vector<T> table_values;
  table_values.reserve(data_.size());
  T value;

  for (const auto& row : data_) {
    if (!utilities::To<string, T>(row[index], value))
      LOG_ERROR() << location() << "The value" << row[index] << " in column " << column << " could not be converted to type " << utilities::demangle(typeid(value).name());

    table_values.push_back(value);
  }

  for (const auto& table_value : table_values) {
    if (std::find(values.begin(), values.end(), table_value) == values.end())
      LOG_ERROR() << location() << "The value " << table_value << " in column " << column << " is not a valid value for this model";
  }
}

/**
 * This method will return all values in a column in a single vector
 * as the target type. This makes it easy for objects using this table
 * to do something like table->GetColumnValuesAs<unsigned>(PARAM_YEAR);
 *
 * @param column The column label to prcess
 */
template <typename T>
vector<T> Table::GetColumnValuesAs(const string& column) {
  vector<T> result;
  result.reserve(data_.size());
  T value;

  unsigned index = column_index(column);
  for (const auto& row : data_) {
    if (!utilities::To<string, T>(row[index], value))
      LOG_ERROR() << location() << "The value" << row[index] << " in column " << column << " could not be converted to type " << utilities::demangle(typeid(value).name());

    result.push_back(value);
  }

  return result;
}

template <>
inline vector<string> Table::GetColumnValuesAs(const string& column) {
  vector<string> result;

  unsigned index = column_index(column);
  for (const auto& row : data_) {
    result.push_back(row[index]);
  }
  return result;
}

/**
 * This method will map the columns in a table to a year.
 * It will return a map of the year to a vector of proportions.
 *
 * @param year_column_index The index of the column containing the year values
 * @param data_index_start The starting index of the data columns to map
 * @param data_index_end The ending index of the data columns to map
 * @return A map of year to vector of proportions
 */
template <typename T>
std::map<unsigned, std::vector<T>> Table::MapColumnsToYear(unsigned year_column_index, unsigned data_index_start, unsigned data_index_end) const {
  std::map<unsigned, std::vector<T>> result;

  for (const auto& row : data_) {
    if (year_column_index >= row.size())
      LOG_ERROR() << location() << "The table " << label_ << " does not have a column at index " << year_column_index << ". We expected it to contain year values.";

    unsigned year;
    if (!utilities::To<string, unsigned>(row[year_column_index], year))
      LOG_ERROR() << location() << "The value" << row[year_column_index] << " in column at index " << year_column_index << " could not be converted to type "
                  << utilities::demangle(typeid(year).name());

    std::vector<T> values;
    for (unsigned i = data_index_start; i <= data_index_end; ++i) {
      if (i >= row.size())
        LOG_ERROR() << location() << "The table " << label_ << " does not have a column at index " << i << ". We expected it to contain data values for the year.";

      T value;
      if (!utilities::To<string, T>(row[i], value))
        LOG_ERROR() << location() << "The value" << row[i] << " in column at index " << i << " could not be converted to type " << utilities::demangle(typeid(value).name());

      values.push_back(value);
    }

    result[year] = values;
  }

  return result;
}

/**
 * This method will map a specific column to a year.
 * It will return a map of the year to the value in the specified column.
 *
 * @param year_column_index The index of the column containing the year values
 * @param data_column_index The index of the column containing the data values
 * @return A map of year to value in the specified column
 */
template <typename T>
std::map<unsigned, T> Table::MapColumnToYear(unsigned year_column_index, unsigned data_column_index) const {
  std::map<unsigned, T> result;

  for (const auto& row : data_) {
    if (year_column_index >= row.size())
      LOG_ERROR() << location() << "The table " << label_ << " does not have a column at index " << year_column_index << ". We expected it to contain year values.";

    unsigned year;
    if (!utilities::To<string, unsigned>(row[year_column_index], year))
      LOG_ERROR() << location() << "The value" << row[year_column_index] << " in column at index " << year_column_index << " could not be converted to type "
                  << utilities::demangle(typeid(year).name());

    if (data_column_index >= row.size())
      LOG_ERROR() << location() << "The table " << label_ << " does not have a column at index " << data_column_index << ". We expected it to contain data values for the year.";

    T value;
    if (!utilities::To<string, T>(row[data_column_index], value))
      LOG_ERROR() << location() << "The value" << row[data_column_index] << " in column at index " << data_column_index << " could not be converted to type "
                  << utilities::demangle(typeid(value).name());

    result[year] = value;
  }

  return result;
}

}  // namespace niwa::parameters::table

#endif /* SOURCE_PARAMETERS_TABLE_TABLE_INL_H_ */
