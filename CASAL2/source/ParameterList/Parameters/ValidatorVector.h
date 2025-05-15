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

/**
 * Class Definition
 */
class ValidatorVector : public std::enable_shared_from_this<ValidatorVector> {
public:
  ValidatorVector(shared_ptr<Model> model, niwa::ParameterList* list, niwa::parameterlist::Parameter* param) : model_(model), parameters_(list), parameter_(param) {};
  ~ValidatorVector() {};

  shared_ptr<ValidatorVector> GreaterThan(Double value);
  shared_ptr<ValidatorVector> GreaterThan(unsigned value);
  shared_ptr<ValidatorVector> GreaterThanOrEqualTo(Double value);
  shared_ptr<ValidatorVector> GreaterThanOrEqualTo(unsigned value);
  shared_ptr<ValidatorVector> LessThan(Double value);
  shared_ptr<ValidatorVector> LessThanOrEqualTo(Double value);
  shared_ptr<ValidatorVector> IsInList(initializer_list<string> list);
  shared_ptr<ValidatorVector> LessThanOrEqualToParameter(const string& label);

protected:
  BindableVector<Double>*   GetParameterAsVectorDouble();
  BindableVector<unsigned>* GetParameterAsVectorUnsigned();

  shared_ptr<Model>               model_;
  niwa::ParameterList*            parameters_;
  niwa::parameterlist::Parameter* parameter_;
};

}  // namespace niwa::parameters
#endif  // VALIDATORVECTOR_H_