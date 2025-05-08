/**
 * @file TagLossEmpirical.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/08
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "TagLossEmpirical.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestResources/TestFixtures/InternalEmptyModel.h"

namespace niwa::processes::age {
using niwa::testfixtures::InternalEmptyModel;

const std::string test_cases_process_tag_loss =
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
names tagged
age_lengths no_age_length

@age_length no_age_length
type none
length_weight [type=none]

@initialisation_phase iphase1
years 2

@time_step step_one
processes ageing recruitment tag_loss

@ageing ageing
categories *

@Recruitment recruitment
type constant
categories tagged
proportions 1.0
R0 100000
age 1
)";

/*
 * Sample unit test
 */
TEST_F(InternalEmptyModel, Processes_TagLossEmpirical) {
  const string tag_loss = R"(
    @process tag_loss
    type tag_loss_empirical
    categories tagged
    time_step_proportions 1.0
    tag_loss_rate 0.25
    selectivities loss_rate1=[type=constant; c=1]
    year 2002
    years_at_liberty 0
  )";

  AddConfigurationLine(test_cases_process_tag_loss, __FILE__, 24);
  AddConfigurationLine(tag_loss, __FILE__, 64);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& tagged = model_->partition().category("tagged");
  EXPECT_DOUBLE_EQ(77880.078307140488, tagged.data_[0]);
  EXPECT_DOUBLE_EQ(77880.078307140488, tagged.data_[1]);
  EXPECT_DOUBLE_EQ(77880.078307140488, tagged.data_[2]);
  EXPECT_DOUBLE_EQ(77880.078307140488, tagged.data_[3]);
  EXPECT_DOUBLE_EQ(77880.078307140488, tagged.data_[4]);
}

/*
 * Sample unit test
 */
TEST_F(InternalEmptyModel, Processes_TagLossEmpirical_MultipleYears) {
  const string tag_loss = R"(
    @process tag_loss
    type tag_loss_empirical
    categories tagged
    time_step_proportions 1.0
    tag_loss_rate 0.25
    selectivities loss_rate1=[type=constant; c=1]
    year 2000
    years_at_liberty 1 2
  )";

  AddConfigurationLine(test_cases_process_tag_loss, __FILE__, 24);
  AddConfigurationLine(tag_loss, __FILE__, 64);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& tagged = model_->partition().category("tagged");
  EXPECT_DOUBLE_EQ(77880.078307140488, tagged.data_[0]);
  EXPECT_DOUBLE_EQ(60653.065971263342, tagged.data_[1]);
  EXPECT_DOUBLE_EQ(60653.065971263342, tagged.data_[2]);
  EXPECT_DOUBLE_EQ(60653.065971263342, tagged.data_[3]);
  EXPECT_DOUBLE_EQ(60653.065971263342, tagged.data_[4]);
}

}  // namespace niwa::processes::age
#endif  // TESTMODE