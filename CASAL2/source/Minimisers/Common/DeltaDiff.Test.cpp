/**
 * @file GammaDiff.Test.cpp
 * @author Scott Rasmussen (scott@zaita.com)
 * @brief Test minimiser GammaDiff to ensure we get consistent covariance matrixes etc
 * @version 0.1
 * @date 2021-05-01
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifdef TESTMODE
// Headers
#include "DeltaDiff.h"

#include <boost/algorithm/string/replace.hpp>
#include <iostream>

#include "../../DerivedQuantities/Manager.h"
#include "../../Estimates/Manager.h"
#include "../../MCMCs/MCMC.h"
#include "../../MCMCs/Manager.h"
#include "../../MPD/MPD.Mock.h"
#include "../../Model/Managers.h"
#include "../../Model/Model.h"
#include "../../ObjectiveFunction/ObjectiveFunction.h"
#include "../../TestResources/MockClasses/Model.h"
#include "../../TestResources/TestCases/TwoSexModel.h"
#include "../../TestResources/TestFixtures/BaseThreaded.h"
#include "../../TestResources/TestFixtures/BasicModel.h"
#include "../Manager.h"

// namespaces
namespace niwa::minimisers {

// using
using std::cout;
using std::endl;

// nicer class for displaying outputs
class DeltaDiffModel : public niwa::testfixtures::BaseThreaded {};

/**
 * @brief Test a simple minimisation of the TwoSex model
 *
 */
TEST_F(DeltaDiffModel, Minimise_TwoSex_OneThread) {
  string ammended_definition = testcases::test_cases_two_sex_model_population;
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  AddConfigurationLine(ammended_definition, __FILE__, 84);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kEstimation));
  auto model = runner_->model();

  auto estimates = model->managers()->estimate()->GetIsEstimated();
  ASSERT_EQ(4u, estimates.size());
#ifdef GITHUB_ACTIONS_WINDOWS
  EXPECT_DOUBLE_EQ(1.3923340393224122e-06, estimates[0]->value());
  EXPECT_DOUBLE_EQ(20158673.650022902, estimates[1]->value());
  EXPECT_DOUBLE_EQ(10.134307762912039, estimates[2]->value());
  EXPECT_DOUBLE_EQ(4.8741448830880802, estimates[3]->value());

  EXPECT_DOUBLE_EQ(1978.1596979263302, model->objective_function().score());
#elif _WIN64
  EXPECT_DOUBLE_EQ(2.3229213187706944e-06, estimates[0]->value());
  EXPECT_DOUBLE_EQ(12458963.006770428, estimates[1]->value());
  EXPECT_DOUBLE_EQ(10.154749765784082, estimates[2]->value());
  EXPECT_DOUBLE_EQ(4.8763681695193615, estimates[3]->value());

  EXPECT_DOUBLE_EQ(1979.302049961175, model->objective_function().score());
#elif __linux__
  EXPECT_DOUBLE_EQ(2.5062542677147851e-06, estimates[0]->value());
  EXPECT_DOUBLE_EQ(11295955.569032623, estimates[1]->value());
  EXPECT_DOUBLE_EQ(10.140710759352372, estimates[2]->value());
  EXPECT_DOUBLE_EQ(4.8763693792257161, estimates[3]->value());

  EXPECT_DOUBLE_EQ(1979.3191094154802, model->objective_function().score());
#endif
}

/**
 * @brief Test a simple minimisation of the TwoSex model
 *
 */
TEST_F(DeltaDiffModel, Minimise_TwoSex_FourThreads) {
  string ammended_definition = testcases::test_cases_two_sex_model_population;
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  boost::replace_all(ammended_definition, "threads 1", "threads 4");
  AddConfigurationLine(ammended_definition, __FILE__, 76);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kEstimation));
  auto model = runner_->model();

  auto estimates = model->managers()->estimate()->GetIsEstimated();
  ASSERT_EQ(4u, estimates.size());
#ifdef GITHUB_ACTIONS_WINDOWS
  EXPECT_DOUBLE_EQ(1.3923340393224122e-06, estimates[0]->value());
  EXPECT_DOUBLE_EQ(20158673.650022902, estimates[1]->value());
  EXPECT_DOUBLE_EQ(10.134307762912039, estimates[2]->value());
  EXPECT_DOUBLE_EQ(4.8741448830880802, estimates[3]->value());

  EXPECT_DOUBLE_EQ(1978.1596979263302, model->objective_function().score());
#elif _WIN64
  EXPECT_DOUBLE_EQ(2.3229213187706944e-06, estimates[0]->value());
  EXPECT_DOUBLE_EQ(12458963.006770428, estimates[1]->value());
  EXPECT_DOUBLE_EQ(10.154749765784082, estimates[2]->value());
  EXPECT_DOUBLE_EQ(4.8763681695193615, estimates[3]->value());

  EXPECT_DOUBLE_EQ(1979.302049961175, model->objective_function().score());
#elif __linux__
  EXPECT_DOUBLE_EQ(2.5062542677147851e-06, estimates[0]->value());
  EXPECT_DOUBLE_EQ(11295955.569032623, estimates[1]->value());
  EXPECT_DOUBLE_EQ(10.140710759352372, estimates[2]->value());
  EXPECT_DOUBLE_EQ(4.8763693792257161, estimates[3]->value());

  EXPECT_DOUBLE_EQ(1979.3191094154802, model->objective_function().score());
#endif
}

}  // namespace niwa::minimisers

#endif