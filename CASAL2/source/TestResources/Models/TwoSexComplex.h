/**
 * @file TwoSexComplex.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/06/18
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifndef TESTCASES_MODELS_TWOSEX_COMPLEX_H_
#define TESTCASES_MODELS_TWOSEX_COMPLEX_H_

#ifdef TESTMODE

#include <string>

namespace niwa::testresources::models {

/**
 *
 */
const std::string two_sex_complex =
    R"(
@model
start_year 1994
final_year 2008
projection_final_year 2012
base_weight_units kgs
min_age 1
max_age 50
age_plus t
length_bins 1:25
length_plus t
initialisation_phases iphase1 iphase2
time_steps step_one step_two

@categories
format stage.sex
names immature.male mature.male immature.female mature.female
age_lengths age_size_male age_size_male age_size_female age_size_female

@age_length age_size_male
type von_bertalanffy
length_weight [type=none]
k 0.277
t0 0.11
linf 90.3
cv_first 1
cv_last 9

@age_length age_size_female
type von_bertalanffy
length_weight [type=none]
k 0.202
t0 -0.20
linf 113.4
cv_first 1
cv_last 9

@initialisation_phase iphase1
years 200
exclude_processes fishing
convergence_years 200

@initialisation_phase iphase2
years 1
exclude_processes fishing

@time_step step_one
processes recruitment maturation halfM fishing halfM

@time_step step_two
processes ageing

@ageing ageing
categories *

@Recruitment recruitment
type constant
categories stage=immature
proportions 0.6 0.4
R0 997386
age 1

@mortality halfM
type constant_rate
categories *
M 0.065 0.065 0.065 0.065
selectivities logistic_one logistic_two logistic_three logistic_four
time_step_proportions 1.0 

@mortality fishing
type event
categories *
years           1998         1999         2000         2001         2002         2003         2004          2005          2006          2007
catches  1849.153714 14442.000000 28323.203463 24207.464203 47279.000000 58350.943094 82875.872790 115974.547730 113852.472257 119739.517172
U_max 0.99
selectivities logistic_one logistic_two logistic_three logistic_four
penalty event_mortality_penalty

@process maturation
type transition_category
from stage=immature
to stage=mature
proportions 0.8 0.75
selectivities logistic_one logistic_two

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

@penalty event_mortality_penalty
type process
log_scale False
multiplier 10

@catchability catchability_one
type free
q 0.000153139

@catchability catchability_two
type free
q 0.000176921

@catchability catchability_three
type free
q 0.000254921

@ageing_error ageing_error_one
type off_by_one
p1 0.15
p2 0.25
k 5
)";

}  // namespace niwa::testresources::models

#endif /* TESTMODE */
#endif /* TESTCASES_MODELS_TWOSEX_COMPLEX_H_ */
