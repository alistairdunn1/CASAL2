/**
 * @file ProportionsMigrating.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/11
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "ProportionsMigrating.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Observations/Manager.h"
#include "TestResources/Models/TwoSex.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"

namespace niwa::observations::age {

using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

const string proportions_migrating_twosex_basic =
    R"(
@observation simple
type process_proportions_migrating
years 1995
ageing_error Ageing_error
time_step step_one
process maturation
plus_group true
min_age 4
max_age 9
likelihood lognormal
categories immature.male+immature.female
table obs
1995 0.64 0.58 0.65 0.66 0.71 0.60
end_table
table error_values
1995 0.25
end_table

@ageing_error Ageing_error
type none
)";

TEST_F(InternalEmptyModel, Observation_Proportions_Migrating) {
  AddConfigurationLine(testresources::models::two_sex, "TestResources/Models/TwoSex.h", 27);
  AddConfigurationLine(proportions_migrating_twosex_basic, __FILE__, 33);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(8208.368462085693, obj_function.score());

  Observation* observation = model_->managers()->observation()->GetObservation("simple");
  ASSERT_NE(nullptr, observation);

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  ASSERT_EQ(1u, comparisons.size());

  unsigned year = 1995;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(6u, comparisons[year].size());
  EXPECT_EQ("immature.male+immature.female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.25, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(0.64000000000000001, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(5406.0316452198822, comparisons[year][0].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][1].category_);
  EXPECT_DOUBLE_EQ(0.25, comparisons[year][1].error_value_);
  EXPECT_DOUBLE_EQ(0.050000000000000072, comparisons[year][1].expected_);
  EXPECT_DOUBLE_EQ(0.57999999999999996, comparisons[year][1].observed_);
  EXPECT_DOUBLE_EQ(49.377643984164834, comparisons[year][1].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][2].category_);
  EXPECT_DOUBLE_EQ(0.25, comparisons[year][2].error_value_);
  EXPECT_DOUBLE_EQ(0.076996987399869016, comparisons[year][2].expected_);
  EXPECT_DOUBLE_EQ(0.65000000000000002, comparisons[year][2].observed_);
  EXPECT_DOUBLE_EQ(37.203346885139254, comparisons[year][2].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][3].category_);
  EXPECT_DOUBLE_EQ(0.25, comparisons[year][3].error_value_);
  EXPECT_DOUBLE_EQ(0.17044039423368937, comparisons[year][3].expected_);
  EXPECT_DOUBLE_EQ(0.66000000000000003, comparisons[year][3].observed_);
  EXPECT_DOUBLE_EQ(14.3999480768684, comparisons[year][3].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][4].category_);
  EXPECT_DOUBLE_EQ(0.25, comparisons[year][4].error_value_);
  EXPECT_DOUBLE_EQ(0.3126219116078422, comparisons[year][4].expected_);
  EXPECT_DOUBLE_EQ(0.70999999999999996, comparisons[year][4].observed_);
  EXPECT_DOUBLE_EQ(4.565446631807597, comparisons[year][4].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][5].category_);
  EXPECT_DOUBLE_EQ(0.25, comparisons[year][5].error_value_);
  EXPECT_DOUBLE_EQ(0.56850113967958016, comparisons[year][5].expected_);
  EXPECT_DOUBLE_EQ(0.59999999999999998, comparisons[year][5].observed_);
  EXPECT_DOUBLE_EQ(-1.3430017715739275, comparisons[year][5].score_);
}

}  // namespace niwa::observations::age
#endif  // TESTMODE