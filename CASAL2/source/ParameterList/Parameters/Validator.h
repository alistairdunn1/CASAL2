/**
 * @file Validator.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/08
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifndef VALIDATOR_H_
#define VALIDATOR_H_

// headers
#include <memory>
#include <string>

#include "../../Utilities/Types.h"
#include "Bindable.h"

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
class Validator : public std::enable_shared_from_this<Validator> {
public:
  Validator(shared_ptr<Model> model, niwa::ParameterList* list, niwa::parameterlist::Parameter* param) : model_(model), parameters_(list), parameter_(param) {};
  ~Validator() {};

  shared_ptr<Validator> GreaterThan(Double value);
  shared_ptr<Validator> GreaterThan(unsigned value);
  shared_ptr<Validator> GreaterThanOrEqualTo(Double value);
  shared_ptr<Validator> GreaterThanOrEqualTo(unsigned value);
  shared_ptr<Validator> GreaterThanOrEqualToParameter(const string& label);
  shared_ptr<Validator> LessThan(Double value);
  shared_ptr<Validator> LessThanOrEqualTo(Double value);
  shared_ptr<Validator> LessThanOrEqualTo(unsigned value);
  shared_ptr<Validator> LessThanParameter(const string& label);
  shared_ptr<Validator> LessThanOrEqualToParameter(const string& label);
  shared_ptr<Validator> GreaterThanOrEqualToModelMinAge();
  shared_ptr<Validator> GreaterThanModelMinAge();
  shared_ptr<Validator> LessThanOrEqualToModelMaxAge();
  shared_ptr<Validator> LessThanModelMaxAge();
  shared_ptr<Validator> IsAge();
  shared_ptr<Validator> IsInList(initializer_list<string> list);
  shared_ptr<Validator> DuplicateParameterIfNotAssigned(const string& label);
  shared_ptr<Validator> IsModelYear();
  shared_ptr<Validator> DefaultValue(unsigned value);
  shared_ptr<Validator> RequiredIf(bool required);

protected:
  Bindable<Double>*   GetParameterAsDouble(bool null_on_error = false);
  Bindable<unsigned>* GetParameterAsUnsigned(bool null_on_error = false);

  shared_ptr<Model>               model_;
  niwa::ParameterList*            parameters_;
  niwa::parameterlist::Parameter* parameter_;
};

}  // namespace niwa::parameters
#endif  // VALIDATOR_H_