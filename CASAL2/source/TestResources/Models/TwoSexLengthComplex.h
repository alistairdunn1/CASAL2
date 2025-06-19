/**
 * @file TwoSexComplex.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/06/18
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifndef TESTCASES_MODELS_TWOSEX_LENGTH_COMPLEX_H_
#define TESTCASES_MODELS_TWOSEX_LENGTH_COMPLEX_H_

#ifdef TESTMODE

#include <string>

namespace niwa::testresources::models {

/**
 *
 */
const std::string two_sex_length_complex =
    R"(
@model
type length
start_year 1994
final_year 2008
projection_final_year 2012
base_weight_units tonnes
length_bins 1:25
length_plus t
initialisation_phases iphase1 iphase2
time_steps step_one step_two

@categories
format stage.sex
names immature.male mature.male immature.female mature.female
age_lengths age_size_male age_size_male age_size_female age_size_female
growth_increment growth_model_male*2 growth_model_female*2

@age_length age_size_male
type von_bertalanffy
length_weight length_weight_male
k 0.277
t0 0.11
linf 90.3
cv_first 1
cv_last 9

@length_weight length_weight_male
type basic
a 0.000000000373
b 3.145
units tonnes

@age_length age_size_female
type von_bertalanffy
length_weight length_weight_female
k 0.202
t0 -0.20
linf 113.4
cv_first 1
cv_last 9

@length_weight length_weight_female
type basic
a 0.000000000373
b 3.379
units tonnes

@growth_increment growth_model_male
type basic
l_alpha 15
l_beta  35
g_alpha 7
g_beta 1
min_sigma 2
distribution normal
length_weight length_weight_male
cv 0.0
compatibility_option casal2

@growth_increment growth_model_female
type basic
l_alpha 20
l_beta  40
g_alpha 10
g_beta 1
min_sigma 2
distribution normal
length_weight length_weight_female
cv 0.0
compatibility_option casal

@initialisation_phase iphase1
years 200
convergence_years 200

@initialisation_phase iphase2
years 1

@time_step step_one
processes recruitment growth maturation

@time_step step_two
processes mortality

@process recruitment
type recruitment_beverton_holt
ssb_offset 1
standardise_years 1994:2008
initial_mean_length 5
initial_length_cv 0.40
recruitment_multipliers 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0
b0 1500
categories immature.male immature.female
proportions 0.6 0.4
steepness 0.75
ssb SSB

@process mortality
type mortality_constant_rate
m 0.2
relative_m_by_length logistic_one logistic_two logistic_three logistic_four
categories *
time_step_proportions 1.0

@process growth
type growth
categories *

@process maturation
type transition_category
from stage=immature
to stage=mature
proportions 0.8 0.75
selectivities logistic_one logistic_two

@derived_quantity SSB
type biomass
categories *
selectivities logistic_one logistic_two logistic_three logistic_four
time_step step_one
time_step_proportion 0.5

@selectivity constant_one
type constant
c 1

@selectivity logistic_one
type logistic
a50 9
ato95 4
alpha 0.7

@selectivity logistic_two
type logistic
a50 9
ato95 4

@selectivity logistic_three
type logistic
a50 8
ato95 3

@selectivity logistic_four
type logistic
a50 5
ato95 2

@selectivity logistic_five
type logistic
a50 7
ato95 4

@selectivity logistic_six
type logistic
a50 9
ato95 1

@selectivity logistic_producing_one
type logistic_producing
L 5
H 30
a50 8
ato95 3

@selectivity logistic_producing_two
type logistic_producing
L 6
H 25
a50 7
ato95 4

@catchability catchability_one
type free
q 0.000153139

@catchability catchability_two
type free
q 0.000176921

@catchability catchability_three
type free
q 0.000254921
)";

}  // namespace niwa::testresources::models

#endif /* TESTMODE */
#endif /* TESTCASES_MODELS_TWOSEX_LENGTH_COMPLEX_H_ */
