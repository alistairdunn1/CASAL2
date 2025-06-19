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
#include "TestResources/Models/TwoSexLengthComplex.h"
#include "TestResources/Models/UnsexedLengthBased.h"
#include "TestResources/TestFixtures/InternalEmptyLengthModel.h"

// Namespaces
namespace niwa::observations::length {
using niwa::testfixtures::InternalEmptyLengthModel;
using std::cout;
using std::endl;

/**
 * This unit test is specifically for ensuring we have the right values
 * being populated in the comparison object. Very carefully we're concerned
 * about the selectivities and the categories they're associated with.
 */
TEST_F(InternalEmptyLengthModel, Observation_TagRecapture_By_Length_for_growth_TwoSexComplex) {
  const std::string observation_definition =
      R"(
      @observation observation
      type tag_recapture_by_length_for_growth
      length_bins 3:10
      plus_group true
      years 2008
      categories immature.male+mature.male immature.female+mature.female
      selectivities logistic_one logistic_two logistic_three logistic_four
      time_step step_two
      time_step_proportion 1.0
      likelihood multinomial
      table recaptured
      2008 3 4 5 6 7 8 9 10 3 4 5 6 7 8 9 10
      end_table     
    )";

  AddConfigurationLine(testresources::models::two_sex_length_complex, "TwoSexLengthComplex.h", 23);
  AddConfigurationLine(observation_definition, __FILE__, 32);
  LoadConfiguration();

  model_->Start(RunMode::kTesting);
  model_->FullIteration();

  auto observation_ptr = model_->managers()->observation()->GetObservation("observation");
  ASSERT_NE(nullptr, observation_ptr);

  const vector<obs::Comparison>& comparisons = observation_ptr->comparisons(2008);
  ASSERT_EQ(16u, comparisons.size());

  // Check first comparison
  unsigned index = 0;
  EXPECT_EQ("immature.male+mature.male", comparisons[index].category_);
  ASSERT_EQ(2u, comparisons[index].selectivities_.size());
  auto sel_it = comparisons[index].selectivities_.begin();
  EXPECT_EQ("logistic_one", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_two", *sel_it);
  EXPECT_EQ(0u, comparisons[index].age_);
  EXPECT_DOUBLE_EQ(3.0, comparisons[index].length_);
  EXPECT_DOUBLE_EQ(4.7706895052684511e-09, comparisons[index].expected_);
  EXPECT_DOUBLE_EQ(0.028846153846153848, comparisons[index].observed_);
  EXPECT_DOUBLE_EQ(1.0, comparisons[index].process_error_);
  EXPECT_DOUBLE_EQ(104, comparisons[index].error_value_);
  EXPECT_DOUBLE_EQ(0.99047619047619051, comparisons[index].adjusted_error_);
  EXPECT_DOUBLE_EQ(9.9999999999999994e-12, comparisons[index].delta_);
  EXPECT_DOUBLE_EQ(0.53162107059655239, comparisons[index].score_);

  // Check second comparison
  index = 10;
  EXPECT_EQ("immature.female+mature.female", comparisons[index].category_);
  ASSERT_EQ(2u, comparisons[index].selectivities_.size());
  sel_it = comparisons[index].selectivities_.begin();
  EXPECT_EQ("logistic_four", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_three", *sel_it);
  EXPECT_EQ(0u, comparisons[index].age_);
  EXPECT_DOUBLE_EQ(5.0, comparisons[index].length_);
  EXPECT_DOUBLE_EQ(3.4638600294582146e-10, comparisons[index].expected_);
  EXPECT_DOUBLE_EQ(0.04807692307692308, comparisons[index].observed_);
  EXPECT_DOUBLE_EQ(1.0, comparisons[index].process_error_);
  EXPECT_DOUBLE_EQ(104, comparisons[index].error_value_);
  EXPECT_DOUBLE_EQ(0.99047619047619051, comparisons[index].adjusted_error_);
  EXPECT_DOUBLE_EQ(9.9999999999999994e-12, comparisons[index].delta_);
  EXPECT_DOUBLE_EQ(1.0116445897983106, comparisons[index].score_);
}

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
  EXPECT_NEAR(8.9853421051625126, obj_function.score(), 0.001);

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
  EXPECT_NEAR(1.4428123880214689, comparisons[year][0].score_, 0.001);

  EXPECT_DOUBLE_EQ(52, comparisons[year][7].error_value_);
  EXPECT_DOUBLE_EQ(0.99720481393411942, comparisons[year][7].expected_);
  EXPECT_DOUBLE_EQ(0.19230769230769232, comparisons[year][7].observed_);
  EXPECT_NEAR(-0.081492235327594101, comparisons[year][7].score_, 0.001);
}

}  // namespace niwa::observations::length

#endif /* TESTMODE */
