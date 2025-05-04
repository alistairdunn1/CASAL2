/**
 * @file TagRecaptureByLength.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/02
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "TagRecaptureByLength.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Observations/Manager.h"
#include "TestResources/Models/CasalComplex1.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"

// Namespaces
namespace niwa::age {
using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

const std::string test_cases_observation_simple_tag_recapture_by_length =
    R"(
@observation observation1
type tag_recapture_by_length
length_bins 3:10
plus_group true
years 2000
categories male
selectivities male_maturity
tagged_categories male
tagged_selectivities male_maturity
time_step two
detection 0.9
time_step_proportion 1.0
likelihood binomial
table scanned
2000 10 11 12 13 14 15 16 17
end_table 
table recaptured
2000 3 4 5 6 7 8 9 10
end_table
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Observation_TagRecapture_By_Length) {
  AddConfigurationLine(testresources::models::test_cases_models_casal_complex_1, "TestResources/Models/CasalComplex1.h", 109);
  AddConfigurationLine(test_cases_observation_simple_tag_recapture_by_length, __FILE__, 32);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_NEAR(1049.2385219236039, obj_function.score(), 0.001);

  Observation* observation = model_->managers()->observation()->GetObservation("observation1");

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  ASSERT_EQ(1u, comparisons.size());

  unsigned year = 2000;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(8u, comparisons[year].size());
  EXPECT_EQ("male", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(10, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(0.29999999999999999, comparisons[year][0].observed_);
  EXPECT_NEAR(73.277257867700513, comparisons[year][0].score_, 0.001);

  EXPECT_DOUBLE_EQ(17, comparisons[year][7].error_value_);
  EXPECT_DOUBLE_EQ(0.90000000000000002, comparisons[year][7].expected_);
  EXPECT_DOUBLE_EQ(0.58823529411764708, comparisons[year][7].observed_);
  EXPECT_NEAR(7.2962012915342811, comparisons[year][7].score_, 0.001);
}

}  // namespace niwa::age

#endif /* TESTMODE */
