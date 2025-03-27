/**
 * @file ObjectiveFunction.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 21/02/2013
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * This report will print the results from the
 * objective function
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */
#ifndef REPORTS_TEST_OBJECTIVEFUNCTION_H_
#define REPORTS_TEST_OBJECTIVEFUNCTION_H_

// Headers
#include "../../Reports/Report.h"

// Namespaces
namespace niwa::reports::test {

/**
 * Class definition
 */
class ObjectiveFunction : public niwa::Report {
public:
  // Methods
  ObjectiveFunction();
  virtual ~ObjectiveFunction() = default;
  void DoValidate(shared_ptr<Model> model) final {};
  void DoBuild(shared_ptr<Model> model) final {};
  void DoExecute(shared_ptr<Model> model) final;

  vector<double>& scores() { return scores_; }

private:
  vector<double> scores_;
};

}  // namespace niwa::reports::test
#endif /* REPORTS_TEST_OBJECTIVEFUNCTION_H_ */
