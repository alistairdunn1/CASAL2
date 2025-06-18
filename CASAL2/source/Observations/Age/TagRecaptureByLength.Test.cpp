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
#include "TestResources/Models/TwoSexComplex.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"

// Namespaces
namespace niwa::observations::age {
using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

/**
 * This unit test is specifically for ensuring we have the right values
 * being populated in the comparison object. Very carefully we're concerned
 * about the selectivities and the categories they're associated with.
 */
TEST_F(InternalEmptyModel, Observation_TagRecapture_By_Length_TwoSexComplex) {
  const std::string observation_definition =
      R"(
      @observation observation
      type tag_recapture_by_length
      length_bins 3:10
      plus_group true
      years 2008
      categories immature.male+mature.male immature.female
      selectivities logistic_one logistic_two logistic_three
      tagged_categories immature.male+mature.male immature.female
      tagged_selectivities logistic_four logistic_five logistic_six
      time_step step_two
      detection 0.9
      time_step_proportion 1.0
      likelihood binomial
      table scanned
      2008 10 11 12 13 14 15 16 17 10 11 12 13 14 15 16 17
      end_table 
      table recaptured
      2008 3 4 5 6 7 8 9 10 3 4 5 6 7 8 9 10
      end_table      
    )";

  AddConfigurationLine(testresources::models::two_sex_complex, "TwoSexComplex.h", 23);
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
  ASSERT_EQ(4u, comparisons[index].selectivities_.size());
  auto sel_it = comparisons[index].selectivities_.begin();
  EXPECT_EQ("logistic_five", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_four", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_one", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_two", *sel_it);
  EXPECT_EQ(0u, comparisons[index].age_);
  EXPECT_DOUBLE_EQ(3.0, comparisons[index].length_);
  EXPECT_DOUBLE_EQ(1.5381594919600015, comparisons[index].expected_);
  EXPECT_DOUBLE_EQ(0.3, comparisons[index].observed_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].process_error_);
  EXPECT_DOUBLE_EQ(10.0, comparisons[index].error_value_);
  EXPECT_DOUBLE_EQ(10.0, comparisons[index].adjusted_error_);
  EXPECT_DOUBLE_EQ(9.9999999999999994e-12, comparisons[index].delta_);
  EXPECT_DOUBLE_EQ(344.18165071370964, comparisons[index].score_);

  // Check second comparison
  index = 10;
  EXPECT_EQ("immature.female", comparisons[index].category_);
  ASSERT_EQ(2u, comparisons[index].selectivities_.size());
  sel_it = comparisons[index].selectivities_.begin();
  EXPECT_EQ("logistic_six", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_three", *sel_it);
  EXPECT_EQ(0u, comparisons[index].age_);
  EXPECT_DOUBLE_EQ(5.0, comparisons[index].length_);
  EXPECT_DOUBLE_EQ(0.35643372249872296, comparisons[index].expected_);
  EXPECT_DOUBLE_EQ(0.41666666666666669, comparisons[index].observed_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].process_error_);
  EXPECT_DOUBLE_EQ(12, comparisons[index].error_value_);
  EXPECT_DOUBLE_EQ(12, comparisons[index].adjusted_error_);
  EXPECT_DOUBLE_EQ(9.9999999999999994e-12, comparisons[index].delta_);
  EXPECT_DOUBLE_EQ(1.5685852818311794, comparisons[index].score_);
}

/**
 *
 */
TEST_F(InternalEmptyModel, Observation_TagRecapture_By_Length) {
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

}  // namespace niwa::observations::age

#endif /* TESTMODE */
