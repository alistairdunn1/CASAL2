/**
 * @file Process.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/07
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Observations/Manager.h"
#include "Process.h"
#include "TestResources/Models/TwoSex.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"

namespace niwa {
namespace observations {
namespace age {

using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

const string proportions_by_category_twosex_basic =
    R"(
@observation simple
type proportions_by_category
categories immature.male+immature.female
target_categories immature.male+immature.female+mature.male+mature.female
delta 1e-5
years 1995 
time_step step_one
time_step_proportion 1.0 
selectivities Maturation 
target_selectivities One 
plus_group true
min_age 1
max_age 50
likelihood binomial
table obs
1995 0 0 0 0 0 0 0.125 0 0 0.047619 0.0344828 0 0.0192308 0.0958904 0.0989011 0.196721 0.176471 0.291304 0.324561 0.402878 0.546703 0.604494 0.725314 0.796774 0.856094 0.901714 0.939333 0.960422 0.966992 0.988498 0.984941 0.991223 0.993568 0.996481 0.99837 0.997366 0.999401 0.998588 1 1 1 1 1 1 1 1 1 1 1 1 
end_table
table error_values
1995 1 1 2 2 4 8 8 12 25 21 29 41 52 73 91 122 187 230 228 278 364 445 557 620 681 875 989 1137 1333 1478 1793 1823 2021 1989 1840 1898 1670 1416 1185 907 695 478 291 166 96 61 31 14 13 5 
end_table
)";

TEST_F(InternalEmptyModel, Proportions_By_Category) {
  AddConfigurationLine(testresources::models::two_sex, "TestResources/Models/TwoSex.h", 27);
  AddConfigurationLine(proportions_by_category_twosex_basic, __FILE__, 33);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(345189.28808143677, obj_function.score());

  Observation* observation = model_->managers()->observation()->GetObservation("simple");
  ASSERT_NE(nullptr, observation);

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  ASSERT_EQ(1u, comparisons.size());

  unsigned year = 1995;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(50u, comparisons[year].size());
  EXPECT_EQ("immature.male+immature.female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(1, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][0].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][1].category_);
  EXPECT_DOUBLE_EQ(1, comparisons[year][1].error_value_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][1].expected_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][1].observed_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][1].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][2].category_);
  EXPECT_DOUBLE_EQ(2, comparisons[year][2].error_value_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][2].expected_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][2].observed_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][2].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][3].category_);
  EXPECT_DOUBLE_EQ(2, comparisons[year][3].error_value_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][3].expected_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][3].observed_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][3].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][4].category_);
  EXPECT_DOUBLE_EQ(4, comparisons[year][4].error_value_);
  EXPECT_DOUBLE_EQ(21.05263157894737, comparisons[year][4].expected_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][4].observed_);
  EXPECT_DOUBLE_EQ(104.0968492931022, comparisons[year][4].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][5].category_);
  EXPECT_DOUBLE_EQ(8, comparisons[year][5].error_value_);
  EXPECT_DOUBLE_EQ(14.811517054756051, comparisons[year][5].expected_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[year][5].observed_);
  EXPECT_DOUBLE_EQ(205.21084153036333, comparisons[year][5].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][10].category_);
  EXPECT_DOUBLE_EQ(29, comparisons[year][10].error_value_);
  EXPECT_DOUBLE_EQ(33.671074862718044, comparisons[year][10].expected_);
  EXPECT_DOUBLE_EQ(0.034482800000000001, comparisons[year][10].observed_);
  EXPECT_DOUBLE_EQ(735.4615921642926, comparisons[year][10].score_);

  EXPECT_EQ("immature.male+immature.female", comparisons[year][20].category_);
  EXPECT_DOUBLE_EQ(364, comparisons[year][20].error_value_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][20].expected_);
  EXPECT_DOUBLE_EQ(0.54670300000000005, comparisons[year][20].observed_);
  EXPECT_DOUBLE_EQ(2181.4624551408647, comparisons[year][20].score_);
}

}  // namespace age
}  // namespace observations
}  // namespace niwa
#endif  // TESTMODE