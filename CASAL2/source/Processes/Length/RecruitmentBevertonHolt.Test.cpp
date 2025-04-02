/**
 * @file RecruitmentBevertonHolt.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/02
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "RecruitmentBevertonHolt.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Model/Model.h"
#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Partition/Partition.h"
#include "Processes/Manager.h"
#include "TestResources/TestFixtures/InternalEmptyLengthModel.h"

namespace niwa::processes::length {
using niwa::testfixtures::InternalEmptyLengthModel;
using std::cout;
using std::endl;

const std::string test_cases_process_recruitment_bh =
    R"(
@model
type length
start_year 1986 
final_year 1987
projection_final_year 2021
length_bins  1:15
length_plus true
length_plus_group 15
base_weight_units tonnes
initialisation_phases Equilibrium_state
time_steps Annual step2

@categories 
format sex
names uni
growth_increment growth_model

@initialisation_phase Equilibrium_state
type iterative
years 200
convergence_years 200
exclude_processes growth

@time_step Annual 
processes Recruit_BH mortality

@time_step step2
processes growth

@process Recruit_BH
type recruitment_beverton_holt
ssb_offset 1
standardise_years 1986:1987
initial_mean_length 5
initial_length_cv 0.40
recruitment_multipliers 1.00 1.00 
b0 1500
categories uni
proportions 1
steepness 0.75
ssb SSB

@process mortality
type mortality_constant_rate
m 0.2
relative_m_by_length One
categories uni
time_step_proportions 1.0

@process growth
type growth
categories uni

@growth_increment growth_model
type basic
l_alpha 5
l_beta  10
g_alpha 7
g_beta 1
min_sigma 2
distribution normal
length_weight allometric
cv 0.1
compatibility_option casal2

@length_weight allometric
type basic
a 0.000000000373
b 3.145
units tonnes

@derived_quantity SSB
type biomass
categories uni
selectivities maturity
time_step Annual
time_step_proportion 0.5

@selectivity maturity
type logistic
a50 30 
ato95 5

@selectivity One
type constant
c 1
)";

TEST_F(InternalEmptyLengthModel, Processes_Recruitment_BevertonHolt) {
  AddConfigurationLine(test_cases_process_recruitment_bh, __FILE__, 30);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& uni = model_->partition().category("uni");
  EXPECT_NEAR(3352210.7833991447, uni.data_[0], 1e-5);
  EXPECT_NEAR(150621504.15927425, uni.data_[1], 1e-5);
  EXPECT_NEAR(4320943348.662369, uni.data_[2], 1e-5);
  EXPECT_NEAR(73808962359.278091, uni.data_[3], 1e-5);
  EXPECT_NEAR(755102736602.484009, uni.data_[4], 1e-5);
  EXPECT_NEAR(4891892712791.377930, uni.data_[5], 1e-5);
  EXPECT_NEAR(22831710926190.597656, uni.data_[6], 1e-5);
  EXPECT_NEAR(89078913993865.140625, uni.data_[7], 1e-5);
  EXPECT_NEAR(293505469395302.250000, uni.data_[8], 1e-5);
  EXPECT_NEAR(727000632389128.250000, uni.data_[9], 1e-5);
  EXPECT_NEAR(1245343507250031.500000, uni.data_[10], 1e-5);
  EXPECT_NEAR(1456280648525284.500000, uni.data_[11], 1e-5);
  EXPECT_NEAR(1199380133603353.000000, uni.data_[12], 1e-5);
  EXPECT_NEAR(730274095336281.250000, uni.data_[13], 1e-5);
  EXPECT_NEAR(518632433449280.625000, uni.data_[14], 1e-5);
}

const std::string test_cases_process_recruitment_bh_modified =
    R"(
@model
type length
start_year 1986 
final_year 1987
projection_final_year 2021
length_bins  1:15
length_plus true
length_plus_group 15
base_weight_units tonnes
initialisation_phases Equilibrium_state
time_steps Annual step2

@categories 
format sex
names uni
growth_increment growth_model

@initialisation_phase Equilibrium_state
type iterative
years 200
convergence_years 200
exclude_processes growth

@time_step Annual 
processes Recruit_BH mortality

@time_step step2
processes growth

@process Recruit_BH
type recruitment_beverton_holt
ssb_offset 1
standardise_years 1986:1987
initial_mean_length 3
initial_length_cv 0.20
recruitment_multipliers 1.00 1.00 
b0 1700
categories uni
proportions 1
steepness 0.66
ssb SSB

@process mortality
type mortality_constant_rate
m 0.2
relative_m_by_length One
categories uni
time_step_proportions 1.0

@process growth
type growth
categories uni

@growth_increment growth_model
type basic
l_alpha 5
l_beta  10
g_alpha 7
g_beta 1
min_sigma 2
distribution normal
length_weight allometric
cv 0.1
compatibility_option casal2

@length_weight allometric
type basic
a 0.000000000373
b 3.145
units tonnes

@derived_quantity SSB
type biomass
categories uni
selectivities maturity
time_step Annual
time_step_proportion 0.5

@selectivity maturity
type logistic
a50 30 
ato95 5

@selectivity One
type constant
c 1
)";

TEST_F(InternalEmptyLengthModel, Processes_Recruitment_BevertonHolt_Modified) {
  AddConfigurationLine(test_cases_process_recruitment_bh_modified, __FILE__, 144);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  partition::Category& uni = model_->partition().category("uni");
  EXPECT_NEAR(21863936395.999607, uni.data_[0], 1e-5);
  EXPECT_NEAR(5086774282869.0645, uni.data_[1], 1e-5);
  EXPECT_NEAR(127151982730395.89, uni.data_[2], 1e-5);
  EXPECT_NEAR(1027900096909477.6, uni.data_[3], 1e-5);
  EXPECT_NEAR(6231964902776545, uni.data_[4], 1e-5);
  EXPECT_NEAR(30880904819496964, uni.data_[5], 1e-5);
  EXPECT_NEAR(1.3346371263371518e+17, uni.data_[6], 1e-5);
  EXPECT_NEAR(5.4476532242291648e+17, uni.data_[7], 1e-5);
  EXPECT_NEAR(1.9759199808727982e+18, uni.data_[8], 1e-5);
  EXPECT_NEAR(5.4352472240778004e+18, uni.data_[9], 1e-5);
  EXPECT_NEAR(1.028406767775436e+19, uni.data_[10], 1e-5);
  EXPECT_NEAR(1.3186365685033587e+19, uni.data_[11], 1e-5);
  EXPECT_NEAR(1.1839818117959377e+19, uni.data_[12], 1e-5);
  EXPECT_NEAR(7.8290701064306606e+18, uni.data_[13], 1e-5);
  EXPECT_NEAR(6.2344119526705951e+18, uni.data_[14], 1e-5);
}

}  // namespace niwa::processes::length
#endif  // TESTMODE