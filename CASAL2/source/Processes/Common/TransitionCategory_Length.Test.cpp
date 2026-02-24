/**
 * @file TransitionCategory.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/07
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "TransitionCategory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "TestResources/TestFixtures/InternalEmptyLengthModel.h"

namespace niwa::processes::length {
using niwa::testfixtures::InternalEmptyLengthModel;

/**
 *
 */
const std::string length_mortality_transition_category_basic =
    R"(
@model
type length
start_year 2000 
final_year 2000
length_bins  1:5
length_plus true
length_plus_group 6
base_weight_units tonnes
time_steps one

@categories
format sex
names male female tagged
growth_increment growth_model*3

@growth_increment growth_model
type basic
l_alpha 1
l_beta  3
g_alpha 5
g_beta 1
min_sigma 2
distribution normal
length_weight [type=none]
cv 0.1
compatibility_option casal2

@time_step one 
processes recruitment growth mortality transition

@recruitment recruitment
type constant
categories male female
proportions 0.6 0.4
length_bins 1
r0 100000

@process mortality
type mortality_constant_rate
m 0.6 0.4
relative_m_by_length maturity
categories male female
time_step_proportions 1.0  

@process growth
type growth
categories male female

@selectivity maturity
type logistic
a50 3
ato95 1

@selectivity one
type constant
c 1
)";

/*
 * Simple unit test
 */
TEST_F(InternalEmptyLengthModel, Processes_TransitionCategory) {
  const string transition_process = R"(
    @process transition
    type transition_category
    from male
    to tagged
    selectivities one
    proportions 0.1
  )";

  AddConfigurationLine(length_mortality_transition_category_basic, __FILE__, 29);
  AddConfigurationLine(transition_process, __FILE__, 92);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& male = model_->partition().category("male");
  EXPECT_NEAR(2147.7626967982337, male.data_[0], 1e-5);
  EXPECT_NEAR(3166.730839663448, male.data_[1], 1e-5);
  EXPECT_NEAR(4010.0196899998809, male.data_[2], 1e-5);
  EXPECT_NEAR(5213.5717411130263, male.data_[3], 1e-5);
  EXPECT_NEAR(17744.708325711686, male.data_[4], 1e-5);

  partition::Category& female = model_->partition().category("female");
  EXPECT_NEAR(1594.7359808890521, female.data_[0], 1e-5);
  EXPECT_NEAR(2434.9257951819186, female.data_[1], 1e-5);
  EXPECT_NEAR(3495.1296110602852, female.data_[2], 1e-5);
  EXPECT_NEAR(4705.6997736982248, female.data_[3], 1e-5);
  EXPECT_NEAR(16053.928757326928, female.data_[4], 1e-5);

  partition::Category& tagged = model_->partition().category("tagged");
  EXPECT_NEAR(238.64029964424819, tagged.data_[0], 1e-5);
  EXPECT_NEAR(351.8589821848276, tagged.data_[1], 1e-5);
  EXPECT_NEAR(445.5577433333201, tagged.data_[2], 1e-5);
  EXPECT_NEAR(579.28574901255854, tagged.data_[3], 1e-5);
  EXPECT_NEAR(1971.6342584124097, tagged.data_[4], 1e-5);
}

/*
 * Moving No fish
 */
TEST_F(InternalEmptyLengthModel, Processes_TransitionCategory_DoubleProportion) {
  const string transition_process = R"(
    @process transition
    type transition_category
    from male
    to tagged
    selectivities one
    proportions 0.2
  )";

  AddConfigurationLine(length_mortality_transition_category_basic, __FILE__, 29);
  AddConfigurationLine(transition_process, __FILE__, 92);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& male = model_->partition().category("male");
  EXPECT_NEAR(1909.1223971539853, male.data_[0], 1e-5);
  EXPECT_NEAR(2814.8718574786208, male.data_[1], 1e-5);
  EXPECT_NEAR(3564.4619466665608, male.data_[2], 1e-5);
  EXPECT_NEAR(4634.2859921004683, male.data_[3], 1e-5);
  EXPECT_NEAR(15773.074067299278, male.data_[4], 1e-5);

  partition::Category& female = model_->partition().category("female");
  EXPECT_NEAR(1594.7359808890521, female.data_[0], 1e-5);
  EXPECT_NEAR(2434.9257951819186, female.data_[1], 1e-5);
  EXPECT_NEAR(3495.1296110602852, female.data_[2], 1e-5);
  EXPECT_NEAR(4705.6997736982248, female.data_[3], 1e-5);
  EXPECT_NEAR(16053.928757326928, female.data_[4], 1e-5);

  partition::Category& tagged = model_->partition().category("tagged");
  EXPECT_NEAR(477.28059928849638, tagged.data_[0], 1e-5);
  EXPECT_NEAR(703.71796436965519, tagged.data_[1], 1e-5);
  EXPECT_NEAR(891.11548666664021, tagged.data_[2], 1e-5);
  EXPECT_NEAR(1158.5714980251171, tagged.data_[3], 1e-5);
  EXPECT_NEAR(3943.2685168248195, tagged.data_[4], 1e-5);

  // This should pass because 0.2 is 2x 0.1 so we're doing some extra checks here.
  EXPECT_NEAR(2.0 * 238.64029964424819, tagged.data_[0], 1e-5);
  EXPECT_NEAR(2.0 * 351.8589821848276, tagged.data_[1], 1e-5);
  EXPECT_NEAR(2.0 * 445.5577433333201, tagged.data_[2], 1e-5);
  EXPECT_NEAR(2.0 * 579.28574901255854, tagged.data_[3], 1e-5);
  EXPECT_NEAR(2.0 * 1971.6342584124097, tagged.data_[4], 1e-5);
}

}  // namespace niwa::processes::length
#endif  // TESTMODE