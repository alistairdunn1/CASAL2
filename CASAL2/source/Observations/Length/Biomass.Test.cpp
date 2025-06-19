/**
 * @file Biomass.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/06/19
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "Biomass.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Observations/Manager.h"
#include "TestResources/Models/TwoSexLengthComplex.h"
#include "TestResources/TestFixtures/InternalEmptyLengthModel.h"

namespace niwa::observations::length {
using niwa::testfixtures::InternalEmptyLengthModel;
using std::cout;
using std::endl;

/**
 * This unit test is specifically for ensuring we have the right values
 * being populated in the comparison object. Very carefully we're concerned
 * about the selectivities and the categories they're associated with.
 */
TEST_F(InternalEmptyLengthModel, Observation_Length_Biomass_TwoSexComplex) {
  const std::string observation_definition =
      R"(
        @observation observation
        type biomass
        catchability catchability_one
        time_step step_one
        categories immature.male+immature.female immature.female
        selectivities logistic_one logistic_two logistic_three
        likelihood lognormal
        time_step_proportion 1.0
        years 2008
        table obs
        2008 22.50 11.25 0.2
        end_table
    )";

  AddConfigurationLine(testresources::models::two_sex_length_complex, "TwoSexLengthComplex.h", 23);
  AddConfigurationLine(observation_definition, __FILE__, 36);
  LoadConfiguration();

  model_->Start(RunMode::kTesting);
  model_->FullIteration();

  auto observation_ptr = model_->managers()->observation()->GetObservation("observation");
  ASSERT_NE(nullptr, observation_ptr);

  const vector<obs::Comparison>& comparisons = observation_ptr->comparisons(2008);
  ASSERT_EQ(2u, comparisons.size());

  // Check first comparison
  unsigned index = 0;
  EXPECT_EQ("immature.male+immature.female", comparisons[index].category_);
  ASSERT_EQ(2u, comparisons[index].selectivities_.size());
  auto sel_it = comparisons[index].selectivities_.begin();
  EXPECT_EQ("logistic_one", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_two", *sel_it);
  EXPECT_EQ(0u, comparisons[index].age_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].length_);
  EXPECT_DOUBLE_EQ(0.050482317544881139, comparisons[index].expected_);
  EXPECT_DOUBLE_EQ(22.5, comparisons[index].observed_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].process_error_);
  EXPECT_DOUBLE_EQ(0.2, comparisons[index].error_value_);
  EXPECT_DOUBLE_EQ(0.2, comparisons[index].adjusted_error_);
  EXPECT_DOUBLE_EQ(9.9999999999999994e-12, comparisons[index].delta_);
  EXPECT_DOUBLE_EQ(475.7473154422774, comparisons[index].score_);

  // Check second comparison
  ++index;
  EXPECT_EQ("immature.female", comparisons[index].category_);
  ASSERT_EQ(1u, comparisons[index].selectivities_.size());
  EXPECT_EQ("logistic_three", *comparisons[index].selectivities_.begin());
  EXPECT_EQ(0u, comparisons[index].age_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].length_);
  EXPECT_DOUBLE_EQ(0.029176901613811885, comparisons[index].expected_);
  EXPECT_DOUBLE_EQ(11.25, comparisons[index].observed_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].process_error_);
  EXPECT_DOUBLE_EQ(0.2, comparisons[index].error_value_);
  EXPECT_DOUBLE_EQ(0.2, comparisons[index].adjusted_error_);
  EXPECT_DOUBLE_EQ(9.9999999999999994e-12, comparisons[index].delta_);
  EXPECT_DOUBLE_EQ(453.40731192931418, comparisons[index].score_);
}

}  // namespace niwa::observations::length
#endif  // TESTMODE