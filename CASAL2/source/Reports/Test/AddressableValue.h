/**
 * @file AddressableValue.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/07
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * This report sill log the values of an addressable at different points in time
 * throughout the model. This is primarily used to track changes to an addressable
 * when we're doing things like unit testing. It's difficult to test things like
 * time-varying values, so we'll use this report to log them and query.
 *
 *
 */
#ifndef REPORTS_TEST_ADDRESSABLEVALUE_H_
#define REPORTS_TEST_ADDRESSABLEVALUE_H_

// Headers
#include "../../Reports/Report.h"

// Namespaces
namespace niwa::reports::test {

/**
 * Class definition
 */
class AddressableValue : public niwa::Report {
public:
  // Methods
  AddressableValue();
  virtual ~AddressableValue() = default;
  void DoValidate(shared_ptr<Model> model) final;
  void DoBuild(shared_ptr<Model> model) final;
  void DoExecute(shared_ptr<Model> model) final;

  map<unsigned, Double>& values() { return values_; }

protected:
  string                addressable_label_;
  Double*               addressable_;
  map<unsigned, Double> values_;
};

}  // namespace niwa::reports::test
#endif /* REPORTS_TEST_ADDRESSABLEVALUE_H_ */
