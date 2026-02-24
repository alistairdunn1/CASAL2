/**
 * @file Gradient.Test.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2021-05-09
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifdef TESTMODE

// headers
#include "Gradient.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/algorithm/string/replace.hpp>
#include <cmath>
#include <iostream>

#include "../DerivedQuantities/Manager.h"
#include "../MCMCs/MCMC.h"
#include "../MCMCs/Manager.h"
#include "../Model/Managers.h"
#include "../Model/Model.h"
#include "../ObjectiveFunction/ObjectiveFunction.h"
#include "../TestResources/MockClasses/Model.h"
#include "../TestResources/TestCases/TwoSexModel.h"
#include "../TestResources/TestFixtures/BaseThreaded.h"
#include "../TestResources/TestFixtures/BasicModel.h"
#include "Math.h"

// namespaces
namespace niwa::utilities::gradient {

using std::cout;
using std::endl;

// Empty class to give us better printing of Unit test results
class GradientThreadedModel : public testfixtures::BaseThreaded {};

/**
 *
 */
TEST_F(GradientThreadedModel, Calc_With_1_ModelThread) {
  AddConfigurationLine(testcases::test_cases_two_sex_model_population, __FILE__, 27);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kTesting));
  ASSERT_EQ(1u, runner_->thread_pool()->threads().size());

  auto threads = runner_->thread_pool()->threads();
  ASSERT_NE(nullptr, threads[0]->model());
  ASSERT_EQ(PARAM_AGE, threads[0]->model()->type());

  vector<double> gradient_values;
  vector<double> estimate_values;
  vector<double> lower_bounds;
  vector<double> upper_bounds;
  double         step_size  = 0.0;
  double         last_score = 0.0;

  // Do a simple check
  last_score      = 1993.8041770617783186;
  lower_bounds    = {1.0000000000000000364e-10, 100000, 1, 0.010000000000000000208};
  upper_bounds    = {0.10000000000000000555, 10000000000, 20, 50};
  step_size       = 1e-7;
  estimate_values = {1.106930053441901161e-05, 2372190.8435692931525, 10.18865272253800569, 4.890967742961707998};

  ASSERT_NO_THROW(gradient_values = gradient::Calculate(runner_->thread_pool(), estimate_values, lower_bounds, upper_bounds, step_size, last_score, false, true));
  ASSERT_EQ(4u, gradient_values.size());
  EXPECT_DOUBLE_EQ(-0.071609171358150453, gradient_values[0]);
  EXPECT_DOUBLE_EQ(-0.097900205773013046, gradient_values[1]);
  EXPECT_DOUBLE_EQ(-0.029963302949047504, gradient_values[2]);
  EXPECT_DOUBLE_EQ(-0.076936430685638962, gradient_values[3]);
}

/**
 *
 */
TEST_F(GradientThreadedModel, Calc_With_6_ModelThreads) {
  string amended_definition = testcases::test_cases_two_sex_model_population;
  boost::replace_all(amended_definition, "threads 1", "threads 6");
  AddConfigurationLine(amended_definition, __FILE__, 84);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kTesting));
  ASSERT_EQ(6u, runner_->thread_pool()->threads().size());

  auto threads = runner_->thread_pool()->threads();
  ASSERT_NE(nullptr, threads[0]->model());
  ASSERT_EQ(PARAM_AGE, threads[0]->model()->type());

  vector<double> gradient_values;
  vector<double> estimate_values;
  vector<double> lower_bounds;
  vector<double> upper_bounds;
  double         step_size;
  double         last_score;

  // Do a simple check
  last_score      = 1993.8041770617783186;
  lower_bounds    = {1.0000000000000000364e-10, 100000, 1, 0.010000000000000000208};
  upper_bounds    = {0.10000000000000000555, 10000000000, 20, 50};
  step_size       = 1e-7;
  estimate_values = {1.106930053441901161e-05, 2372190.8435692931525, 10.18865272253800569, 4.890967742961707998};

  ASSERT_NO_THROW(gradient_values = gradient::Calculate(runner_->thread_pool(), estimate_values, lower_bounds, upper_bounds, step_size, last_score, false, true));
  ASSERT_EQ(4u, gradient_values.size());
  EXPECT_DOUBLE_EQ(-0.071609171358150453, gradient_values[0]);
  EXPECT_DOUBLE_EQ(-0.097900205773013046, gradient_values[1]);
  EXPECT_DOUBLE_EQ(-0.029963302949047504, gradient_values[2]);
  EXPECT_DOUBLE_EQ(-0.076936430685638962, gradient_values[3]);
}

/**
 * @brief Construct a new test f object
 *
 */
TEST_F(GradientThreadedModel, Calc_With_Scaled_Values) {
  AddConfigurationLine(testcases::test_cases_two_sex_model_population, __FILE__, 27);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kTesting));
  ASSERT_EQ(1u, runner_->thread_pool()->threads().size());

  auto threads = runner_->thread_pool()->threads();
  ASSERT_NE(nullptr, threads[0]->model());
  ASSERT_EQ(PARAM_AGE, threads[0]->model()->type());

  vector<double> gradient_values;
  vector<double> estimate_values;
  vector<double> lower_bounds;
  vector<double> upper_bounds;
  double         step_size;
  double         last_score;

  // Do a simple check
  last_score      = 1993.8041770617783186;
  lower_bounds    = {1.0000000000000000364e-10, 100000, 1, 0.010000000000000000208};
  upper_bounds    = {0.10000000000000000555, 10000000000, 20, 50};
  step_size       = 1e-7;
  estimate_values = {1.106930053441901161e-05, 2372190.8435692931525, 10.18865272253800569, 4.890967742961707998};

  ASSERT_NO_THROW(gradient_values = gradient::Calculate(runner_->thread_pool(), estimate_values, lower_bounds, upper_bounds, step_size, last_score, false, true));
  ASSERT_EQ(4u, gradient_values.size());
  EXPECT_DOUBLE_EQ(-0.071609171358150453, gradient_values[0]);
  EXPECT_DOUBLE_EQ(-0.097900205773013046, gradient_values[1]);
  EXPECT_DOUBLE_EQ(-0.029963302949047504, gradient_values[2]);
  EXPECT_DOUBLE_EQ(-0.076936430685638962, gradient_values[3]);
}

/**
 * @brief Test gradient with values at lower bound edge
 * When a value is very close to the lower bound, the scaling should still work correctly
 */
TEST_F(GradientThreadedModel, Calc_With_Value_Near_Lower_Bound) {
  AddConfigurationLine(testcases::test_cases_two_sex_model_population, __FILE__, 27);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kTesting));
  ASSERT_EQ(1u, runner_->thread_pool()->threads().size());

  vector<double> gradient_values;
  vector<double> estimate_values;
  vector<double> lower_bounds;
  vector<double> upper_bounds;
  double         step_size  = 1e-7;
  double         last_score = 1993.8041770617783186;

  // Set first value very close to lower bound
  lower_bounds    = {1.0000000000000000364e-10, 100000, 1, 0.010000000000000000208};
  upper_bounds    = {0.10000000000000000555, 10000000000, 20, 50};
  estimate_values = {1.0000000001e-10, 2372190.8435692931525, 10.18865272253800569, 4.890967742961707998};  // First value near lower bound

  ASSERT_NO_THROW(gradient_values = gradient::Calculate(runner_->thread_pool(), estimate_values, lower_bounds, upper_bounds, step_size, last_score, false, true));
  ASSERT_EQ(4u, gradient_values.size());
  // Gradient should still be computed without NaN or Inf
  EXPECT_FALSE(std::isnan(gradient_values[0]));
  EXPECT_FALSE(std::isinf(gradient_values[0]));
}

/**
 * @brief Test gradient with values at upper bound edge
 */
TEST_F(GradientThreadedModel, Calc_With_Value_Near_Upper_Bound) {
  AddConfigurationLine(testcases::test_cases_two_sex_model_population, __FILE__, 27);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kTesting));
  ASSERT_EQ(1u, runner_->thread_pool()->threads().size());

  vector<double> gradient_values;
  vector<double> estimate_values;
  vector<double> lower_bounds;
  vector<double> upper_bounds;
  double         step_size  = 1e-7;
  double         last_score = 1993.8041770617783186;

  // Set third value very close to upper bound (upper bound is 20)
  lower_bounds    = {1.0000000000000000364e-10, 100000, 1, 0.010000000000000000208};
  upper_bounds    = {0.10000000000000000555, 10000000000, 20, 50};
  estimate_values = {1.106930053441901161e-05, 2372190.8435692931525, 19.9999, 4.890967742961707998};  // Third value near upper bound

  ASSERT_NO_THROW(gradient_values = gradient::Calculate(runner_->thread_pool(), estimate_values, lower_bounds, upper_bounds, step_size, last_score, false, true));
  ASSERT_EQ(4u, gradient_values.size());
  // Gradient should still be computed without NaN or Inf
  EXPECT_FALSE(std::isnan(gradient_values[2]));
  EXPECT_FALSE(std::isinf(gradient_values[2]));
}

/**
 * @brief Test gradient with values at midpoint of bounds
 * Values at the midpoint should scale to 0 in the transformed space
 */
TEST_F(GradientThreadedModel, Calc_With_Value_At_Midpoint) {
  AddConfigurationLine(testcases::test_cases_two_sex_model_population, __FILE__, 27);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kTesting));
  ASSERT_EQ(1u, runner_->thread_pool()->threads().size());

  vector<double> gradient_values;
  vector<double> estimate_values;
  vector<double> lower_bounds;
  vector<double> upper_bounds;
  double         step_size  = 1e-7;
  double         last_score = 1993.8041770617783186;

  // Set third value at midpoint (midpoint of [1, 20] is 10.5)
  lower_bounds    = {1.0000000000000000364e-10, 100000, 1, 0.010000000000000000208};
  upper_bounds    = {0.10000000000000000555, 10000000000, 20, 50};
  estimate_values = {1.106930053441901161e-05, 2372190.8435692931525, 10.5, 4.890967742961707998};

  ASSERT_NO_THROW(gradient_values = gradient::Calculate(runner_->thread_pool(), estimate_values, lower_bounds, upper_bounds, step_size, last_score, false, true));
  ASSERT_EQ(4u, gradient_values.size());
  // All gradients should be finite
  for (size_t i = 0; i < gradient_values.size(); ++i) {
    EXPECT_FALSE(std::isnan(gradient_values[i])) << "Gradient " << i << " is NaN";
    EXPECT_FALSE(std::isinf(gradient_values[i])) << "Gradient " << i << " is Inf";
  }
}

/**
 * @brief Test gradient with different step sizes
 * Verify that gradients are computed without numerical errors for different step sizes
 */
TEST_F(GradientThreadedModel, Calc_With_Different_Step_Sizes) {
  AddConfigurationLine(testcases::test_cases_two_sex_model_population, __FILE__, 27);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kTesting));

  vector<double> gradient_values_small;
  vector<double> gradient_values_large;
  vector<double> estimate_values;
  vector<double> lower_bounds;
  vector<double> upper_bounds;
  double         last_score = 1993.8041770617783186;

  lower_bounds    = {1.0000000000000000364e-10, 100000, 1, 0.010000000000000000208};
  upper_bounds    = {0.10000000000000000555, 10000000000, 20, 50};
  estimate_values = {1.106930053441901161e-05, 2372190.8435692931525, 10.18865272253800569, 4.890967742961707998};

  // Calculate with small step size
  ASSERT_NO_THROW(gradient_values_small = gradient::Calculate(runner_->thread_pool(), estimate_values, lower_bounds, upper_bounds, 1e-8, last_score, false, true));

  // Calculate with larger step size
  ASSERT_NO_THROW(gradient_values_large = gradient::Calculate(runner_->thread_pool(), estimate_values, lower_bounds, upper_bounds, 1e-6, last_score, false, true));

  ASSERT_EQ(4u, gradient_values_small.size());
  ASSERT_EQ(4u, gradient_values_large.size());

  // All gradients should be finite (no NaN or Inf)
  // Note: Gradient values may differ significantly between step sizes due to
  // the nature of finite difference approximation on complex objective functions
  for (size_t i = 0; i < gradient_values_small.size(); ++i) {
    EXPECT_FALSE(std::isnan(gradient_values_small[i])) << "Small step gradient " << i << " is NaN";
    EXPECT_FALSE(std::isnan(gradient_values_large[i])) << "Large step gradient " << i << " is NaN";
    EXPECT_FALSE(std::isinf(gradient_values_small[i])) << "Small step gradient " << i << " is Inf";
    EXPECT_FALSE(std::isinf(gradient_values_large[i])) << "Large step gradient " << i << " is Inf";
  }
}

/**
 * @brief Test gradient with pre-scaled values
 * When values_are_scaled=true, the function should skip the scaling step
 */
TEST_F(GradientThreadedModel, Calc_With_PreScaled_Values) {
  AddConfigurationLine(testcases::test_cases_two_sex_model_population, __FILE__, 27);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kTesting));

  vector<double> gradient_values_unscaled;
  vector<double> gradient_values_prescaled;
  vector<double> estimate_values;
  vector<double> scaled_values;
  vector<double> lower_bounds;
  vector<double> upper_bounds;
  double         step_size  = 1e-7;
  double         last_score = 1993.8041770617783186;

  lower_bounds    = {1.0000000000000000364e-10, 100000, 1, 0.010000000000000000208};
  upper_bounds    = {0.10000000000000000555, 10000000000, 20, 50};
  estimate_values = {1.106930053441901161e-05, 2372190.8435692931525, 10.18865272253800569, 4.890967742961707998};

  // Pre-scale the values manually
  scaled_values.resize(estimate_values.size());
  for (size_t i = 0; i < estimate_values.size(); ++i) {
    scaled_values[i] = math::scale(estimate_values[i], lower_bounds[i], upper_bounds[i]);
  }

  // Calculate with unscaled values (let function scale them)
  ASSERT_NO_THROW(gradient_values_unscaled = gradient::Calculate(runner_->thread_pool(), estimate_values, lower_bounds, upper_bounds, step_size, last_score, false, true));

  // Calculate with pre-scaled values
  ASSERT_NO_THROW(gradient_values_prescaled = gradient::Calculate(runner_->thread_pool(), scaled_values, lower_bounds, upper_bounds, step_size, last_score, true, true));

  ASSERT_EQ(4u, gradient_values_unscaled.size());
  ASSERT_EQ(4u, gradient_values_prescaled.size());

  // Results should be identical (or very close)
  for (size_t i = 0; i < gradient_values_unscaled.size(); ++i) {
    EXPECT_NEAR(gradient_values_unscaled[i], gradient_values_prescaled[i], 1e-10) << "Gradient " << i << " differs between scaled and unscaled input";
  }
}

/**
 * @brief Test gradient consistency - running twice should give same result
 */
TEST_F(GradientThreadedModel, Calc_Consistency) {
  AddConfigurationLine(testcases::test_cases_two_sex_model_population, __FILE__, 27);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kTesting));

  vector<double> gradient_values_1;
  vector<double> gradient_values_2;
  vector<double> estimate_values;
  vector<double> lower_bounds;
  vector<double> upper_bounds;
  double         step_size  = 1e-7;
  double         last_score = 1993.8041770617783186;

  lower_bounds    = {1.0000000000000000364e-10, 100000, 1, 0.010000000000000000208};
  upper_bounds    = {0.10000000000000000555, 10000000000, 20, 50};
  estimate_values = {1.106930053441901161e-05, 2372190.8435692931525, 10.18865272253800569, 4.890967742961707998};

  // Calculate gradient twice
  ASSERT_NO_THROW(gradient_values_1 = gradient::Calculate(runner_->thread_pool(), estimate_values, lower_bounds, upper_bounds, step_size, last_score, false, true));
  ASSERT_NO_THROW(gradient_values_2 = gradient::Calculate(runner_->thread_pool(), estimate_values, lower_bounds, upper_bounds, step_size, last_score, false, true));

  ASSERT_EQ(gradient_values_1.size(), gradient_values_2.size());

  // Results should be identical
  for (size_t i = 0; i < gradient_values_1.size(); ++i) {
    EXPECT_DOUBLE_EQ(gradient_values_1[i], gradient_values_2[i]) << "Gradient " << i << " differs between consecutive calls";
  }
}

}  // namespace niwa::utilities::gradient
#endif
