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
#include "TestResources/TestFixtures/InternalEmptyModel.h"

// Namespaces
namespace niwa::age {
using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

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

/**
 *
 */
TEST_F(InternalEmptyModel, Observation_TagRecapture_By_Age) {
  AddConfigurationLine(testresources::models::test_cases_models_casal_complex_1, __FILE__, __LINE__);
  AddConfigurationLine(test_cases_observation_simple_tag_recapture_by_age, __FILE__, 31);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_NEAR(17068.119540329037, obj_function.score(), 0.001);

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
  EXPECT_DOUBLE_EQ(0.89998594853167813, comparisons[year][10].expected_);
  EXPECT_DOUBLE_EQ(1.0, comparisons[year][10].observed_);
  EXPECT_NEAR(1066.4064206455373, comparisons[year][10].score_, 0.001);
}

}  // namespace niwa::age

#endif /* TESTMODE */
