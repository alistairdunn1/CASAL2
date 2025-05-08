/**
 * @file MortalityInitialisationEventBiomass.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/08
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "MortalityInitialisationEventBiomass.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestResources/TestFixtures/InternalEmptyModel.h"

namespace niwa::processes::age {
using niwa::testfixtures::InternalEmptyModel;

const std::string test_case_mortality_initialisation_event =
    R"(
@model
start_year 2000
final_year 2002
min_age 1
max_age 5
base_weight_units kgs
age_plus f
initialisation_phases iphase
time_steps step_one

@categories
format sex
names male female
age_lengths no_age_length*2

@age_length no_age_length
type none
length_weight [type=none]

@initialisation_phase iphase
type iterative
years 2

@time_step step_one
processes ageing recruitment mortality

@ageing ageing
categories *

@recruitment recruitment
type constant
categories *
proportions 0.6 0.4
r0 100000
age 1

@selectivity sel_male
type logistic
a50 11.99
ato95 5.25

@selectivity sel_female
type logistic
a50 16.92
ato95 7.68

@selectivity one
type constant
c 1
)";

/*
 * Sample unit test
 */
TEST_F(InternalEmptyModel, Process_MortalityInitialisationEventBiomass) {
  const string morality_process = R"(
    @mortality mortality
    type initialisation_event_biomass
    categories male
    catch 555
    U_max 0.9
    selectivities one
  )";

  AddConfigurationLine(test_case_mortality_initialisation_event, __FILE__, 24);
  AddConfigurationLine(morality_process, __FILE__, __LINE__);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& male = model_->partition().category("male");
  EXPECT_NEAR(60000, male.data_[0], 1e-5);
  EXPECT_NEAR(60000, male.data_[1], 1e-5);
  EXPECT_NEAR(60000, male.data_[2], 1e-5);
  EXPECT_NEAR(59721.210599020473, male.data_[3], 1e-5);
  EXPECT_NEAR(59168.789400979527, male.data_[4], 1e-5);

  partition::Category& female = model_->partition().category("female");
  EXPECT_NEAR(40000, female.data_[0], 1e-5);
  EXPECT_NEAR(40000, female.data_[1], 1e-5);
  EXPECT_NEAR(40000, female.data_[2], 1e-5);
  EXPECT_NEAR(40000, female.data_[3], 1e-5);
  EXPECT_NEAR(40000, female.data_[4], 1e-5);
}

/**
 * Test when we run across sexes
 */
TEST_F(InternalEmptyModel, Process_MortalityInitialisationEventBiomass_Across_Categories) {
  const string morality_process = R"(
    @mortality mortality
    type initialisation_event_biomass
    categories male female
    catch 555
    U_max 0.9
    selectivities one one
  )";

  AddConfigurationLine(test_case_mortality_initialisation_event, __FILE__, 24);
  AddConfigurationLine(morality_process, __FILE__, __LINE__);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& male = model_->partition().category("male");
  EXPECT_NEAR(60000, male.data_[0], 1e-5);
  EXPECT_NEAR(60000, male.data_[1], 1e-5);
  EXPECT_NEAR(60000, male.data_[2], 1e-5);
  EXPECT_NEAR(59833.036676778058, male.data_[3], 1e-5);
  EXPECT_NEAR(59500.963323221942, male.data_[4], 1e-5);

  partition::Category& female = model_->partition().category("female");
  EXPECT_NEAR(40000, female.data_[0], 1e-5);
  EXPECT_NEAR(40000, female.data_[1], 1e-5);
  EXPECT_NEAR(40000, female.data_[2], 1e-5);
  EXPECT_NEAR(39888.691117852039, female.data_[3], 1e-5);
  EXPECT_NEAR(39667.308882147961, female.data_[4], 1e-5);
}

}  // namespace niwa::processes::age
#endif  // TESTMODE