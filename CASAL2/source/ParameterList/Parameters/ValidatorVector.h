/**
 * @file ValidatorVector.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/08
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifndef VALIDATORVECTOR_H_
#define VALIDATORVECTOR_H_

// headers
#include <memory>
#include <string>

#include "../../Logging/Logging.h"
#include "../../Utilities/Types.h"
#include "BindableVector.h"

// namespaces
namespace niwa {
class Model;
class ParameterList;
}  // namespace niwa
namespace niwa::parameterlist {
class Parameter;
}  // namespace niwa::parameterlist

namespace niwa::parameters {
using niwa::utilities::Double;
using std::initializer_list;
using std::shared_ptr;
using std::string;
using std::weak_ptr;

/**
 * Class Definition
 */
class ValidatorVector : public std::enable_shared_from_this<ValidatorVector> {
public:
  ValidatorVector(shared_ptr<Model> model, niwa::ParameterList* list, niwa::parameterlist::Parameter* param) : model_(model), parameters_(list), parameter_(param) {};
  ~ValidatorVector() {};

  shared_ptr<ValidatorVector> GreaterThan(double value);
  shared_ptr<ValidatorVector> GreaterThan(unsigned value);
  shared_ptr<ValidatorVector> GreaterThanOrEqualTo(double value);
  shared_ptr<ValidatorVector> GreaterThanOrEqualTo(unsigned value);
  shared_ptr<ValidatorVector> LessThan(double value);
  shared_ptr<ValidatorVector> LessThanOrEqualTo(double value);
  shared_ptr<ValidatorVector> LessThanOrEqualTo(unsigned value);
  shared_ptr<ValidatorVector> SumToOne();
  shared_ptr<ValidatorVector> IsInList(initializer_list<string> list);
  shared_ptr<ValidatorVector> LessThanOrEqualToParameter(const string& label);
  shared_ptr<ValidatorVector> IsModelYear();
  shared_ptr<ValidatorVector> IsAge();
  shared_ptr<ValidatorVector> IsLengthBin();
  shared_ptr<ValidatorVector> DefaultToAllModelYears();
  shared_ptr<ValidatorVector> DefaultToModelYearsOnly();
  shared_ptr<ValidatorVector> DefaultToAllModelLengthBins();
  shared_ptr<ValidatorVector> NumberOfElements(unsigned count);
  shared_ptr<ValidatorVector> SameNumberOfElementsAs(const string& label, bool split_combined_categories = true);
  shared_ptr<ValidatorVector> ExpandToNumberOfElements(unsigned count);
  shared_ptr<ValidatorVector> ExpandToSameNumberOfElementsAs(const string& label);
  shared_ptr<ValidatorVector> DuplicateParameterIfNotAssigned(const string& label);
  shared_ptr<ValidatorVector> SameNumberOfElementsModelAgeSpread();
  shared_ptr<ValidatorVector> SameNumberOfElementsModelLengthBinMidPoints();
  shared_ptr<ValidatorVector> IsInIncreasingOrder();
  shared_ptr<ValidatorVector> IsUniqueFrom(const string& label);
  shared_ptr<ValidatorVector> EitherOrTableDefined(const string& table_label);
  shared_ptr<ValidatorVector> DefaultValue(double value, unsigned count);

protected:
  vector<unsigned>             ConvertValuesToUnsigned() const;
  vector<double>               ConvertValuesToDouble() const;
  BindableVector<Double>*      GetParameterAsVectorDouble(bool null_on_error = false);
  BindableVector<unsigned>*    GetParameterAsVectorUnsigned(bool null_on_error = false);
  BindableVector<std::string>* GetParameterAsVectorString(bool null_on_error = false);
  shared_ptr<Model>            model() const {
    auto locked = model_.lock();
    if (!locked)
      LOG_CODE_ERROR() << "ValidatorVector weak_ptr expired";
    return locked;
  }

  weak_ptr<Model>                 model_;
  niwa::ParameterList*            parameters_;
  niwa::parameterlist::Parameter* parameter_;
};

}  // namespace niwa::parameters
#endif  // VALIDATORVECTOR_H_
