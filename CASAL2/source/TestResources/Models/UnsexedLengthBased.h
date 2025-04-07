/**
 * @file UnsexedLengthBased.h
 * @author C.Marsh
 * @github https://github.com/Craig44
 * @date 2020
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 */
#ifndef TESTCASES_MODELS_LENGTHBASED_UNSEXED_
#define TESTCASES_MODELS_LENGTHBASED_UNSEXED_

#ifdef TESTMODE

#include <string>

namespace niwa {
namespace testresources {
namespace models {

/**
 *
 */
const std::string length_based_unsexed_basic =
    R"(
@model
type length
start_year 1986 
final_year 2012
projection_final_year 2021
length_bins  1:15
length_plus false
length_plus_group 15
base_weight_units tonnes
initialisation_phases Equilibrium_state
time_steps Annual

@categories 
format sex
names uni
growth_increment growth_model

@initialisation_phase Equilibrium_state
type iterative
years 200
convergence_years 200

@time_step Annual 
processes Recruit_BH growth  mortality

@process Nop
type null_process

@process Recruit_BH
type recruitment_beverton_holt
ssb_offset 1
standardise_years 1986:2010
initial_mean_length 5
initial_length_cv 0.40
recruitment_multipliers 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00  1.00 1.00 1.00  1.00 1.00 1.00
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

@selectivity double_normal
type double_normal
mu 12
sigma_l 2
sigma_r 8

@selectivity maturity
type logistic
a50 30 
ato95 5

@selectivity One
type constant
c 1
)";

const std::string length_based_unsexed_basic_with_length_plus =
    R"(
@model
type length
start_year 1986 
final_year 2012
projection_final_year 2021
length_bins  1:15
length_plus true
length_plus_group 15
base_weight_units tonnes
initialisation_phases Equilibrium_state
time_steps Annual

@categories 
format sex
names uni
growth_increment growth_model

@initialisation_phase Equilibrium_state
type iterative
years 200
convergence_years 200

@time_step Annual 
processes Recruit_BH growth  mortality

@process Nop
type null_process

@process Recruit_BH
type recruitment_beverton_holt
ssb_offset 1
standardise_years 1986:2010
initial_mean_length 5
initial_length_cv 0.40
recruitment_multipliers 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00  1.00 1.00 1.00  1.00 1.00 1.00
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

@selectivity double_normal
type double_normal
mu 12
sigma_l 2
sigma_r 8

@selectivity maturity
type logistic
a50 30 
ato95 5

@selectivity One
type constant
c 1
)";

}  // namespace models
}  // namespace testresources
}  // namespace niwa

#endif /* TESTMODE */
#endif /* TESTCASES_MODELS_LENGTHBASED_UNSEXED_ */
