/**
 * @file ValidatorVector.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/06/16
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifndef VALIDATORTABLE_H_
#define VALIDATORTABLE_H_

// headers
#include <memory>
#include <string>

#include "Table.h"
#include "Utilities/Types.h"

// namespaces
namespace niwa {
class Model;
class ParameterList;
}  // namespace niwa

namespace niwa::parameters::table {
using niwa::utilities::Double;
using std::initializer_list;
using std::shared_ptr;
using std::string;

/**
 * Class Definition
 */
class ValidatorTable : public std::enable_shared_from_this<ValidatorTable> {
public:
  ValidatorTable(shared_ptr<Model> model, niwa::ParameterList* list, niwa::parameters::table::Table* table) : model_(model), parameters_(list), table_(table) {};
  ~ValidatorTable() {};

  shared_ptr<ValidatorTable> Rows(unsigned count, const string& error_message = "");
  shared_ptr<ValidatorTable> ExpandColumnsTo(unsigned grow_to, unsigned base_index);
  shared_ptr<ValidatorTable> Columns(unsigned count, const string& error_message = "");
  shared_ptr<ValidatorTable> ColumnIsYear(unsigned column_index, const string& error_message = "");
  shared_ptr<ValidatorTable> DoubleDataRange(unsigned start_index, unsigned number_of_columns, const string& error_message = "");
  shared_ptr<ValidatorTable> GreaterThan(unsigned column_index, double value, const string& error_message = "");
  shared_ptr<ValidatorTable> GreaterThanForRange(unsigned start_column_index, unsigned number_of_columns, double value, const string& error_message = "");
  shared_ptr<ValidatorTable> GreaterThanOrEqualToForRange(unsigned start_column_index, unsigned number_of_columns, double value, const string& error_message = "");
  shared_ptr<ValidatorTable> LessThanOrEqualToForRange(unsigned column_index, unsigned number_of_columns, double value, const string& error_message = "");

  // shared_ptr<ValidatorVector> GreaterThan(double value);
  // shared_ptr<ValidatorVector> GreaterThan(unsigned value);
  // shared_ptr<ValidatorVector> GreaterThanOrEqualTo(double value);
  // shared_ptr<ValidatorVector> GreaterThanOrEqualTo(unsigned value);
  // shared_ptr<ValidatorVector> LessThan(double value);
  // shared_ptr<ValidatorVector> LessThanOrEqualTo(double value);
  // shared_ptr<ValidatorVector> LessThanOrEqualTo(unsigned value);
  // shared_ptr<ValidatorVector> SumToOne();
  // shared_ptr<ValidatorVector> IsInList(initializer_list<string> list);
  // shared_ptr<ValidatorVector> LessThanOrEqualToParameter(const string& label);
  // shared_ptr<ValidatorVector> IsModelYear();
  // shared_ptr<ValidatorVector> IsAge();
  // shared_ptr<ValidatorVector> IsLengthBin();
  // shared_ptr<ValidatorVector> DefaultToAllModelYears();
  // shared_ptr<ValidatorVector> DefaultToAllModelLengthBins();
  // shared_ptr<ValidatorVector> NumberOfElements(unsigned count);
  // shared_ptr<ValidatorVector> SameNumberOfElementsAs(const string& label, bool split_combined_categories = true);
  // shared_ptr<ValidatorVector> ExpandToSameNumberOfElementsAs(const string& label);
  // shared_ptr<ValidatorVector> DuplicateParameterIfNotAssigned(const string& label);
  // shared_ptr<ValidatorVector> SameNumberOfElementsModelAgeSpread();
  // shared_ptr<ValidatorVector> SameNumberOfElementsModelLengthBinMidPoints();
  // shared_ptr<ValidatorVector> IsInIncreasingOrder();
  // shared_ptr<ValidatorVector> IsUniqueFrom(const string& label);
  // shared_ptr<ValidatorVector> EitherOrTableDefined(const string& table_label);
  // shared_ptr<ValidatorVector> DefaultValue(double value, unsigned count);

protected:
  // vector<unsigned>             ConvertValuesToUnsigned() const;
  // vector<double>               ConvertValuesToDouble() const;
  // BindableVector<Double>*      GetParameterAsVectorDouble(bool null_on_error = false);
  // BindableVector<unsigned>*    GetParameterAsVectorUnsigned(bool null_on_error = false);
  // BindableVector<std::string>* GetParameterAsVectorString(bool null_on_error = false);

  shared_ptr<Model>               model_;
  niwa::ParameterList*            parameters_;
  niwa::parameters::table::Table* table_;
};

}  // namespace niwa::parameters::table
#endif  // VALIDATORTABLE_H_