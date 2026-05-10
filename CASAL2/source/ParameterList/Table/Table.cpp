/**
 * @file Table.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 16/11/2012
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/�2012 - www.niwa.co.nz
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "Table.h"

#include <algorithm>

#include "Categories/Categories.h"
#include "Model/Model.h"
#include "Translations/Translations.h"
#include "Utilities/String.h"
#include "Utilities/To.h"

// Namespaces
namespace niwa::parameters::table {

/**
 * Default constructor
 */
Table::Table(const string& label) : label_(label) {}

/**
 * Add columns to the table
 *
 * @param columns A list of columns for this table
 */
void Table::AddColumns(const vector<string>& columns) {
  columns_.assign(columns.begin(), columns.end());
}

/**
 * Add a row of data to the table
 *
 * @param row The row of data to add
 */
void Table::AddRow(const vector<string>& row) {
  data_.push_back(row);
}

/**
 * Get the index for the specified column
 *
 * @param label The column label
 * @return The index for the label
 */
unsigned Table::column_index(const string& label, bool throw_error) const {
  for (unsigned i = 0; i < columns_.size(); ++i) {
    if (columns_[i] == label)
      return i;
  }

  if (throw_error)
    LOG_FATAL() << location() << "could not find column " << label << " in the table definition";

  return columns_.size();
}

/**
 * This method will set a list of required columns. This will be used during the Populate method
 * to validate the columns exist and the table structure is as expected.
 */
void Table::set_required_columns(const vector<string>& columns, bool allow_others) {
  required_columns_    = columns;
  allow_other_columns_ = allow_others;
}

/**
 * Return a string that shows the location this parameter was defined
 *
 * @return string containing the file and line details for this parameter
 */
string Table::location() const {
  string line_number;
  niwa::utilities::To<unsigned, string>(line_number_, line_number);
  return string("At line " + line_number + " in " + file_name_ + " the table " + label_ + " ");
}

/**
 * The populate method works as a validate method and a data container
 */
void Table::Populate(shared_ptr<Model> model) {
  if (model == nullptr)
    LOG_CODE_ERROR() << "model == nullptr";

  /**
   * Check the required columns if we've specified any.
   */
  if (required_columns_.size() > 0) {
    vector<string> missing_columns;
    for (const string& column : required_columns_) {
      if (column_index(column, false) == columns_.size())
        missing_columns.push_back(column);
    }
    if (missing_columns.size() > 0)
      LOG_ERROR() << location() << " is missing the following column headers: " << boost::join(missing_columns, ", ");

    // See if we have any extra columns that are not allowed.
    if (required_columns_.size() != columns_.size() && !allow_other_columns_) {
      vector<string> extra_columns;
      for (const string& column : columns_) {
        if (std::find(required_columns_.begin(), required_columns_.end(), column) == required_columns_.end()
            && std::find(optional_columns_.begin(), optional_columns_.end(), column) == optional_columns_.end())
          extra_columns.push_back(column);
      }

      if (extra_columns.size() > 0)
        LOG_ERROR() << location() << "has extra columns not allowed in the definition: " << boost::join(extra_columns, ", ");
    }
  }

  // get the index for PARAM_CATEGORY or PARAM_CATEGORIES if it exists
  unsigned category_index = column_index(PARAM_CATEGORY, false);
  category_index          = category_index == columns_.size() ? column_index(PARAM_CATEGORIES, false) : category_index;
  if (category_index != columns_.size()) {
    // Make a copy of our data object so we can manipulate the container
    vector<vector<string>> data_copy = data_;
    data_.clear();

    /**
     * This code will handle category shorthand while verifying the categories actually
     * exist. So something like male,female.2000 will check male.2000 and female.2000.
     * This code should probably be handled elsewhere?
     */
    vector<string> category_labels;
    string         error = "";
    for (auto row : data_copy) {
      string category_lookup = row[category_index];
      category_labels        = model->categories()->GetCategoryLabelsV(category_lookup, location());
      if (!utilities::String::HandleOperators(category_labels, error))
        LOG_FATAL() << location() << error;
      LOG_FINE() << "category_labels: " << boost::join(category_labels, " ");

      for (auto label : category_labels) {
        if (!model->categories()->IsValid(label)) {
          LOG_ERROR() << location() << "contains an invalid category: " << label;
        }

        row[category_index] = label;
        LOG_FINE() << "re-adding row to table: " << boost::join(row, " ");
        data_.push_back(row);
      }
    }
  }  // if (category_index != columns_.size()) {

  /**
   * This code will check for any columns called year or years and if one is found
   * it'll check that any values in this column fall within the years for this model
   */
  unsigned year_index = column_index(PARAM_YEAR, false);
  year_index          = year_index == columns_.size() ? column_index(PARAM_YEARS, false) : year_index;
  if (year_index != columns_.size()) {
    vector<unsigned> years         = model->years_all();
    const string&    year_col_name = (column_index(PARAM_YEAR, false) != columns_.size()) ? PARAM_YEAR : PARAM_YEARS;

    // Years outside the model range (including projection years) are a warning, not an error.
    // Extra rows will simply be ignored by the object that owns this table.
    for (const auto& row : data_) {
      unsigned year_val = 0;
      if (!utilities::To<string, unsigned>(row[year_index], year_val))
        LOG_ERROR() << location() << "The value " << row[year_index] << " in column " << year_col_name << " could not be converted to an unsigned integer";
      if (std::find(years.begin(), years.end(), year_val) == years.end())
        LOG_WARNING() << location() << "The value " << year_val << " in column " << year_col_name << " is not a valid year for this model and will be ignored";
    }
  }  // if (year_index != columns_.size()) {
}

/**
 * This method will map the columns in a table to a year and category.
 * It will return a map of the year to a map of category labels to a vector of values.
 * If we had values like:
 * | Year | Cat1_A | Cat1_B | Cat2_A | Cat2_B |
 * | 2000 | 1.0    | 2.0    | 3.0    | 4.0    |
 * would return a map with the first key being 2000, and the second key being a map with 1.0 and 2.0,
 * and a second value for 3.0 and 4.0, so the result would look like:
 * * {
 * *   2000: {
 * *     "Cat1": [1.0, 2.0],
 * *     "Cat2": [3.0, 4.0]
 * *   }
 *
 * @param category_labels The labels for the categories to map
 * @param year_column_label The label for the column containing the year values
 * @param data_index_start_label The label for the starting index of the data columns to map
 * @param data_index_end_label The label for the ending index of the data columns to map
 * @return A map of year to a map of category labels to a vector of values
 */
std::map<unsigned, std::map<string, std::vector<double>>> Table::MapColumnsToYearAndCategory(const vector<string>& category_labels, unsigned year_column_index,
                                                                                             unsigned data_index_start, unsigned data_column_count) const {
  LOG_TRACE();

  unsigned elements_per_category = data_column_count / category_labels.size();
  LOG_FINE() << "elements_per_category: " << elements_per_category;

  std::map<unsigned, std::map<std::string, std::vector<double>>> result;
  if (data_column_count % category_labels.size() != 0)
    LOG_ERROR() << location() << "The number of data columns (" << data_column_count << ") is not evenly divisible by the number of category labels (" << category_labels.size()
                << "). This will result in an incorrect mapping of categories to data values.";

  for (const auto& row : data_) {
    if (year_column_index >= row.size())
      LOG_ERROR() << location() << "The table " << label_ << " does not have a column at index " << year_column_index << ". We expected it to contain year values.";

    unsigned year = 0;
    if (!utilities::To<string, unsigned>(row[year_column_index], year))
      LOG_ERROR() << location() << "The value" << row[year_column_index] << " in column at index " << year_column_index << " could not be converted to type "
                  << utilities::demangle(typeid(year).name());

    vector<string>::const_iterator it = row.begin() + data_index_start;
    for (unsigned category_index = 0; category_index < category_labels.size(); ++category_index) {
      if (it + elements_per_category > row.end())
        LOG_ERROR() << location() << "The table " << label_ << " does not have enough data columns for category " << category_labels[category_index]
                    << ". We expected it to contain " << elements_per_category << " values for this category.";

      std::vector<string> values_for_category(it, it + elements_per_category);
      if (values_for_category.size() != elements_per_category)
        LOG_ERROR() << location() << "The table " << label_ << " does not have enough data columns for category " << category_labels[category_index]
                    << ". We expected it to contain " << elements_per_category << " values for this category, but found " << values_for_category.size() << ".";

      std::vector<double> values;
      for (const auto& value : values_for_category) {
        double double_value = 0.0;
        if (!utilities::To<string, double>(value, double_value))
          LOG_ERROR() << location() << "The value '" << value << "' in column at index " << (data_index_start + category_index * elements_per_category)
                      << " could not be converted to type " << utilities::demangle(typeid(double_value).name());

        values.push_back(double_value);
      }

      result[year][category_labels[category_index]] = values;

      it += elements_per_category;
    }
  }  // for (const auto& row : data_) {

  if (result.size() != data_.size()) {
    LOG_CODE_ERROR() << "MapColumnsToYearAndCategory: The number of years in the result does not match the number of rows in the data. "
                     << "Expected: " << data_.size() << ", Actual: " << result.size();
  }
  for (const auto& category_map : result) {
    if (category_map.second.size() != category_labels.size()) {
      LOG_CODE_ERROR() << "MapColumnsToYearAndCategory: The number of categories in the result does not match the number of category labels. "
                       << "Expected: " << category_labels.size() << ", Actual: " << category_map.second.size();
    }
    for (const auto& category : category_map.second) {
      if (category.second.size() != elements_per_category) {
        LOG_CODE_ERROR() << "MapColumnsToYearAndCategory: The number of values in the category '" << category.first
                         << "' does not match the expected number of values. Expected: " << elements_per_category << ", Actual: " << category.second.size();
      }
    }
  }

  return result;
}

}  // namespace niwa::parameters::table
