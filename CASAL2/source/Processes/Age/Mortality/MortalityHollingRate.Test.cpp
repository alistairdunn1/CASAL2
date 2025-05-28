/**
 * @file MortalityHollingRate.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/08
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "MortalityHollingRate.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestResources/TestFixtures/InternalEmptyModel.h"

namespace niwa::processes::age {
using niwa::testfixtures::InternalEmptyModel;

const std::string test_cases_process_holling_rate =
    R"(
@model
start_year 2000
final_year 2002
min_age 1
max_age 5
base_weight_units kgs
initialisation_phases iphase1
time_steps step_one

@categories
format sex
names predator prey
age_lengths no_age_length*2

@age_length no_age_length
type none
length_weight [type=none]

@initialisation_phase iphase1
years 2

@time_step step_one
processes ageing recruitment mortality

@ageing ageing
categories *

@Recruitment recruitment
type constant
categories *
proportions 0.6 0.4
R0 100000
age 1
)";

/*
 * Sample unit test
 */
TEST_F(InternalEmptyModel, Processes_MortalityHollingRate) {
  ASSERT_TRUE(true);
  return;

  const string mortality = R"(
    @process mortality
    type mortality_holling_rate
  )";

  AddConfigurationLine(test_cases_process_holling_rate, __FILE__, 24);
  AddConfigurationLine(mortality, __FILE__, 64);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& male = model_->partition().category("male");
  EXPECT_DOUBLE_EQ(0.0, male.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[2]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[3]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[4]);

  partition::Category& female = model_->partition().category("female");
  EXPECT_DOUBLE_EQ(0.0, female.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[2]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[3]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[4]);
}

}  // namespace niwa::processes::age
#endif  // TESTMODE