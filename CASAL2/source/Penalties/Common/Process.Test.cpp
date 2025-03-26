/**
 * @file Process.Test.cpp
 * @author your_name
 * @github https://github.com/your_github
 * @date 26/03/2025
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

// headers
#include "Process.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "../../Model/Factory.h"
#include "../../TestResources/MockClasses/Model.h"
#include "../../Utilities/Math.h"

// namespaces
namespace niwa::penalties {

using ::testing::Return;

/**
 * Mock class for testing Process penalty
 */
class MockProcessPenalty : public Process {
public:
  MockProcessPenalty() : Process(shared_ptr<Model>(nullptr)) {
    has_score_  = false;
    log_scale_  = false;
    score_      = 0.0;
    multiplier_ = 1.0;
  }

  // Expose methods to directly set parameters for testing
  void SetMultiplier(Double value) { multiplier_ = value; }
  void SetLogScale(bool value) { log_scale_ = value; }

  // Expose the score for verification
  Double GetScore() { return score_; }

  // Reset the score for testing multiple scenarios
  void ResetScore() { score_ = 0.0; }
};

/**
 * Test the Process penalty
 */
TEST(Penalties, Process) {
  MockProcessPenalty process;

  // Test default values
  EXPECT_DOUBLE_EQ(0.0, process.GetScore());  // Force conversion for comparison
  EXPECT_FALSE(process.GetScore() != 0.0);    // Score should be zero initially

  // Test non-log scale penalty calculation (default)
  process.ResetScore();
  process.Trigger(10.0, 5.0);
  // Expected: (10.0 - 5.0)² * 1.0 = 25.0
  EXPECT_DOUBLE_EQ(25.0, process.GetScore());

  // Test with custom multiplier
  process.ResetScore();
  process.SetMultiplier(2.5);
  process.Trigger(10.0, 5.0);
  // Expected: (10.0 - 5.0)² * 2.5 = 62.5
  EXPECT_DOUBLE_EQ(62.5, process.GetScore());

  // Test log scale penalty calculation
  process.ResetScore();
  process.SetLogScale(true);
  process.SetMultiplier(1.0);
  process.Trigger(10.0, 5.0);
  // Expected: (log(10) - log(5))² * 1.0 = (2.302585 - 1.609438)² = (0.693147)² = 0.48045301...
  EXPECT_NEAR(0.4804530139182014, process.GetScore(), 1e-10);

  // Test with zero values with log scale (should use ZeroFun)
  process.ResetScore();
  process.SetLogScale(true);
  process.Trigger(0.0, 5.0);
  // Using ZeroFun(), 0.0 becomes utilities::math::ZeroFun(0.0)
  Double expected_value_1 = log(utilities::math::ZeroFun(0.0));
  Double expected_value_2 = log(utilities::math::ZeroFun(5.0));
  Double expected         = (expected_value_1 - expected_value_2) * (expected_value_1 - expected_value_2);
  EXPECT_NEAR(expected, process.GetScore(), 1e-10);

  // Test accumulation of score
  process.ResetScore();
  process.SetLogScale(false);
  process.SetMultiplier(1.0);
  process.Trigger(10.0, 8.0);                  // (10-8)² = 4
  process.Trigger(5.0, 2.0);                   // (5-2)² = 9
  EXPECT_DOUBLE_EQ(13.0, process.GetScore());  // 4 + 9 = 13
}

}  // namespace niwa::penalties

#endif /* TESTMODE */