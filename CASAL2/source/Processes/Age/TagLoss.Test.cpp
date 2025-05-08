/**
 * @file TagLoss.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/08
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "TagLoss.h"

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
TEST_F(InternalEmptyModel, Processes_TagLoss) {
  const string tag_loss = R"(
    @process tag_loss
    type tag_loss
    categories tagged
    time_step_proportions 1.0
    tag_loss_rate 0.25
    selectivities loss_rate1=[type=constant; c=1]
    tag_loss_type single
    year 2002
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
TEST_F(InternalEmptyModel, Processes_TagLoss_MultipleYears) {
  const string tag_loss = R"(
    @process tag_loss
    type tag_loss
    categories tagged
    time_step_proportions 1.0
    tag_loss_rate 0.25
    selectivities loss_rate1=[type=constant; c=1]
    tag_loss_type single
    year 2000
  )";

  AddConfigurationLine(test_cases_process_tag_loss, __FILE__, 24);
  AddConfigurationLine(tag_loss, __FILE__, 64);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& tagged = model_->partition().category("tagged");
  EXPECT_DOUBLE_EQ(77880.078307140488, tagged.data_[0]);
  EXPECT_DOUBLE_EQ(60653.065971263342, tagged.data_[1]);
  EXPECT_DOUBLE_EQ(47236.655274101475, tagged.data_[2]);
  EXPECT_DOUBLE_EQ(36787.944117144238, tagged.data_[3]);
  EXPECT_DOUBLE_EQ(28650.479686019014, tagged.data_[4]);
}

/*
 * Sample unit test
 */
TEST_F(InternalEmptyModel, Processes_TagLoss_Double) {
  const string tag_loss = R"(
    @process tag_loss
    type tag_loss
    categories tagged
    time_step_proportions 1.0
    tag_loss_rate 0.25
    selectivities loss_rate1=[type=constant; c=1]
    tag_loss_type double
    year 2002
  )";

  AddConfigurationLine(test_cases_process_tag_loss, __FILE__, 24);
  AddConfigurationLine(tag_loss, __FILE__, 64);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& tagged = model_->partition().category("tagged");
  EXPECT_DOUBLE_EQ(100000, tagged.data_[0]);
  EXPECT_DOUBLE_EQ(100000, tagged.data_[1]);
  EXPECT_DOUBLE_EQ(100000, tagged.data_[2]);
  EXPECT_DOUBLE_EQ(100000, tagged.data_[3]);
  EXPECT_DOUBLE_EQ(100000, tagged.data_[4]);
}

/*
 * Sample unit test
 */
TEST_F(InternalEmptyModel, Processes_TagLoss_Double_MultipleYears) {
  const string tag_loss = R"(
    @process tag_loss
    type tag_loss
    categories tagged
    time_step_proportions 1.0
    tag_loss_rate 0.25
    selectivities loss_rate1=[type=constant; c=1]
    tag_loss_type double
    year 2000
  )";

  AddConfigurationLine(test_cases_process_tag_loss, __FILE__, 24);
  AddConfigurationLine(tag_loss, __FILE__, 64);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& tagged = model_->partition().category("tagged");
  EXPECT_DOUBLE_EQ(88866.337151053871, tagged.data_[0]);
  EXPECT_DOUBLE_EQ(84518.187825382454, tagged.data_[1]);
  EXPECT_DOUBLE_EQ(84518.187825382454, tagged.data_[2]);
  EXPECT_DOUBLE_EQ(84518.187825382454, tagged.data_[3]);
  EXPECT_DOUBLE_EQ(84518.187825382454, tagged.data_[4]);
}

}  // namespace niwa::processes::age
#endif  // TESTMODE