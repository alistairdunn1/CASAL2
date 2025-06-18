/**
 * @file TagRecaptureByAge.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/29
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "TagRecaptureByAge.h"

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
 * Basic unit test for the TagRecaptureByAge observation.
 */
TEST_F(InternalEmptyModel, Observation_TagRecapture_By_Age_TwoSexComplex) {
  const std::string observation_definition =
      R"(
      @observation observation
      type tag_recapture_by_age
      min_age 3
      max_age 10
      plus_group false
      years 2000
      categories immature.male+immature.female mature.male
      selectivities logistic_one logistic_two logistic_three
      tagged_categories immature.male mature.female+mature.male
      tagged_selectivities logistic_four logistic_five logistic_six
      time_step step_one
      detection 0.9
      time_step_proportion 1.0
      likelihood binomial
      table scanned
      2000 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120
      end_table 
      table recaptured
      2000 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120
      end_table
      )";
  AddConfigurationLine(testresources::models::two_sex_complex, "TwoSexComplex.h", 23);
  AddConfigurationLine(observation_definition, __FILE__, 36);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_NEAR(-147050.30450957277, obj_function.score(), 0.001);

  auto observation_ptr = model_->managers()->observation()->GetObservation("observation");
  ASSERT_NE(nullptr, observation_ptr);

  map<unsigned, vector<obs::Comparison> >& comparisons = observation_ptr->comparisons();
  ASSERT_EQ(1u, comparisons.size());

  unsigned year  = 2000;
  unsigned index = 0;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(16u, comparisons[year].size());
  EXPECT_EQ("immature.male", comparisons[year][index].category_);
  ASSERT_EQ(3u, comparisons[year][index].selectivities_.size());
  auto sel_it = comparisons[year][index].selectivities_.begin();
  EXPECT_EQ("logistic_four", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_one", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_two", *sel_it);
  EXPECT_DOUBLE_EQ(10120, comparisons[year][index].error_value_);
  EXPECT_DOUBLE_EQ(2.7645925245995282, comparisons[year][index].expected_);
  EXPECT_DOUBLE_EQ(1.0, comparisons[year][index].observed_);
  EXPECT_NEAR(-10290.959741335237, comparisons[year][index].score_, 0.001);

  index = 10;
  EXPECT_EQ("mature.female+mature.male", comparisons[year][index].category_);
  ASSERT_EQ(3u, comparisons[year][index].selectivities_.size());
  sel_it = comparisons[year][index].selectivities_.begin();
  EXPECT_EQ("logistic_five", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_six", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_three", *sel_it);
  EXPECT_DOUBLE_EQ(10120, comparisons[year][index].error_value_);
  EXPECT_DOUBLE_EQ(2.7767646502866312, comparisons[year][index].expected_);
  EXPECT_DOUBLE_EQ(1.0, comparisons[year][index].observed_);
  EXPECT_NEAR(-10335.418925677797, comparisons[year][index].score_, 0.001);
}

/**
 * Basic unit test for the TagRecaptureByAge observation.
 */
TEST_F(InternalEmptyModel, Observation_TagRecapture_By_Age) {
  const std::string test_cases_observation_simple_tag_recapture_by_age =
      R"(
      @observation observation1
      type tag_recapture_by_age
      min_age 3
      max_age 10
      plus_group false
      years 2000
      categories male female
      selectivities male_maturity female_maturity
      tagged_categories male female
      tagged_selectivities male_maturity female_maturity
      time_step one
      detection 0.9
      time_step_proportion 1.0
      likelihood binomial
      table scanned
      2000 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120
      end_table 
      table recaptured
      2000 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120
      end_table
      )";
  AddConfigurationLine(testresources::models::test_cases_models_casal_complex_1, __FILE__, __LINE__);
  AddConfigurationLine(test_cases_observation_simple_tag_recapture_by_age, __FILE__, 31);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_NEAR(17059.974695315232, obj_function.score(), 0.001);

  Observation* observation = model_->managers()->observation()->GetObservation("observation1");

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  ASSERT_EQ(1u, comparisons.size());

  unsigned year = 2000;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(16u, comparisons[year].size());
  EXPECT_EQ("male", comparisons[year][0].category_);
  ASSERT_EQ(1u, comparisons[year][0].selectivities_.size());
  EXPECT_EQ("male_maturity", *comparisons[year][0].selectivities_.begin());
  EXPECT_DOUBLE_EQ(10120, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(0.9, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.0, comparisons[year][0].observed_);
  EXPECT_NEAR(1066.248418457202, comparisons[year][0].score_, 0.001);

  EXPECT_EQ("female", comparisons[year][10].category_);
  ASSERT_EQ(1u, comparisons[year][0].selectivities_.size());
  EXPECT_EQ("female_maturity", *comparisons[year][10].selectivities_.begin());
  EXPECT_DOUBLE_EQ(10120, comparisons[year][10].error_value_);
  EXPECT_DOUBLE_EQ(0.9, comparisons[year][10].expected_);
  EXPECT_DOUBLE_EQ(1.0, comparisons[year][10].observed_);
  EXPECT_NEAR(1066.248418457202, comparisons[year][10].score_, 0.001);
}

/**
 * Basic unit test for the TagRecaptureByAge observation.
 */
TEST_F(InternalEmptyModel, Observation_TagRecapture_By_Age_SingleScannedValue) {
  const std::string test_cases_observation_simple_tag_recapture_by_age =
      R"(
      @observation observation1
      type tag_recapture_by_age
      min_age 3
      max_age 10
      plus_group false
      years 2000
      categories male female
      selectivities male_maturity female_maturity
      tagged_categories male female
      tagged_selectivities male_maturity female_maturity
      time_step one
      detection 0.9
      time_step_proportion 1.0
      likelihood binomial
      table recaptured
      2000 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120 10120
      end_table
      table scanned
      2000 10120 
      end_table 
      )";
  AddConfigurationLine(testresources::models::test_cases_models_casal_complex_1, __FILE__, __LINE__);
  AddConfigurationLine(test_cases_observation_simple_tag_recapture_by_age, __FILE__, 31);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_NEAR(17059.974695315232, obj_function.score(), 0.001);

  Observation* observation = model_->managers()->observation()->GetObservation("observation1");

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  ASSERT_EQ(1u, comparisons.size());

  unsigned year = 2000;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(16u, comparisons[year].size());
  EXPECT_EQ("male", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(10120, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(0.9, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.0, comparisons[year][0].observed_);
  EXPECT_NEAR(1066.248418457202, comparisons[year][0].score_, 0.001);

  EXPECT_EQ("female", comparisons[year][10].category_);
  EXPECT_DOUBLE_EQ(10120, comparisons[year][10].error_value_);
  EXPECT_DOUBLE_EQ(0.9, comparisons[year][10].expected_);
  EXPECT_DOUBLE_EQ(1.0, comparisons[year][10].observed_);
  EXPECT_NEAR(1066.248418457202, comparisons[year][10].score_, 0.001);
}

}  // namespace niwa::observations::age

#endif /* TESTMODE */
