/**
 * @file Growth.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/07
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "Growth.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "TestResources/TestFixtures/InternalEmptyLengthModel.h"

namespace niwa::processes::length {
using niwa::testfixtures::InternalEmptyLengthModel;

/**
 *
 */
const std::string length_growth_basic =
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
names male female
growth_increment growth_model*2

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
processes recruitment growth mortality

@recruitment recruitment
type constant
categories *
proportions 0.6 0.4
length_bins 1
r0 100000

@process growth
type growth
categories *

@process mortality
type mortality_constant_rate
m 0.5 0.5
relative_m_by_length maturity
categories male female
time_step_proportions 1.0

@selectivity maturity
type logistic
a50 3
ato95 1
)";

/*
 * Sample unit test
 */
TEST_F(InternalEmptyLengthModel, Processes_Growth) {
  AddConfigurationLine(length_growth_basic, __FILE__, 29);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& male = model_->partition().category("male");
  EXPECT_NEAR(2389.251784, male.data_[0], 1e-5);
  EXPECT_NEAR(3584.865085, male.data_[1], 1e-5);
  EXPECT_NEAR(4833.138828, male.data_[2], 1e-5);
  EXPECT_NEAR(6394.464189, male.data_[3], 1e-5);
  EXPECT_NEAR(21789.610800, male.data_[4], 1e-5);

  partition::Category& female = model_->partition().category("female");
  EXPECT_NEAR(1592.834522, female.data_[0], 1e-5);
  EXPECT_NEAR(2389.910057, female.data_[1], 1e-5);
  EXPECT_NEAR(3222.092552, female.data_[2], 1e-5);
  EXPECT_NEAR(4262.976126, female.data_[3], 1e-5);
  EXPECT_NEAR(14526.407200, female.data_[4], 1e-5);
}

}  // namespace niwa::processes::length
#endif  // TESTMODE