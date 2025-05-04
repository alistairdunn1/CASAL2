/**
 * @file TagRecaptureByLengthForGrowth.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/02
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "TagRecaptureByLengthForGrowth.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Observations/Manager.h"
#include "TestResources/Models/UnsexedLengthBased.h"
#include "TestResources/TestFixtures/InternalEmptyLengthModel.h"

// Namespaces
namespace niwa::length {
using niwa::testfixtures::InternalEmptyLengthModel;
using std::cout;
using std::endl;

const std::string test_cases_observation_simple_tag_recapture_by_length =
    R"(
@observation observation1
type tag_recapture_by_length_for_growth
length_bins 3:10
plus_group true
years 2000
categories uni
selectivities maturity
time_step Annual
time_step_proportion 1.0
likelihood multinomial
table recaptured
2000 3 4 5 6 7 8 9 10
end_table

@growth_increment growth_model
type basic
l_alpha 5
l_beta  10
g_alpha 7
g_beta 1
min_sigma 2
distribution normal
length_weight allometric
cv 0.0
compatibility_option casal
)";

/**
 *
 */
TEST_F(InternalEmptyLengthModel, Observation_TagRecapture_By_Length_For_Growth) {
  AddConfigurationLine(testresources::models::length_based_unsexed_basic_with_length_plus, "TestResources/Models/UnsexedLengthBased.h", 32);
  AddConfigurationLine(test_cases_observation_simple_tag_recapture_by_length, __FILE__, 32);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_NEAR(407.32576555281048, obj_function.score(), 0.001);

  Observation* observation = model_->managers()->observation()->GetObservation("observation1");

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  ASSERT_EQ(1u, comparisons.size());

  unsigned year = 2000;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(8u, comparisons[year].size());
  EXPECT_EQ("uni", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(52, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(0.057692307692307696, comparisons[year][0].observed_);
  EXPECT_NEAR(79.856509079711401, comparisons[year][0].score_, 0.001);

  EXPECT_DOUBLE_EQ(52, comparisons[year][7].error_value_);
  EXPECT_DOUBLE_EQ(0.99720481393411942, comparisons[year][7].expected_);
  EXPECT_DOUBLE_EQ(0.19230769230769232, comparisons[year][7].observed_);
  EXPECT_NEAR(15.132403572010436, comparisons[year][7].score_, 0.001);
}

}  // namespace niwa::length

#endif /* TESTMODE */
