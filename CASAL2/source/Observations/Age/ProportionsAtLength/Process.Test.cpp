/**
 * @file Process.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/01
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "Process.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Observations/Manager.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"

namespace niwa {
namespace observations {
namespace age {

using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

const std::string test_cases_process_proportions_at_length =
    R"(
@model
start_year 1990
final_year 1997
min_age 1
max_age 4
age_plus true
base_weight_units kgs
initialisation_phases iphase1
time_steps init step1 step2 step3
length_bins 0 20 40 60 80 110
length_plus true

@categories
format stock
names stock
age_lengths age_size

@initialisation_phase iphase1
type iterative
years 100

@time_step init
processes [type=null_process]

@time_step step1
processes Recruitment instant_mort

@time_step step2
processes instant_mort

@time_step step3
processes  Ageing instant_mort

@process Recruitment
type recruitment_beverton_holt
categories stock
proportions 1
r0 4.04838e+006
standardise_years  1990 1991 1992 1993 1994 1995 1996 1997
recruitment_multipliers      1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00
steepness 0.9
ssb biomass_t1
age 1

@process Ageing
type ageing
categories stock

@process instant_mort
type mortality_instantaneous
m 0.19
time_step_proportions 0.42 0.25 0.33
relative_m_by_age One
categories stock
table catches
year FishingWest FishingEest
1991  309000  689000
1992  409000  503000
1993  718000  1087000
1994  656000  1996000
1995  368000  2912000
1996  597000  2903000
1997  1353000 2483000
end_table

table method
method       category  selectivity u_max   time_step penalty
FishingWest   stock     westFSel    0.7     step1     none
FishingEest   stock     eastFSel    0.7     step1     none
end_table

@derived_quantity biomass_t1
type biomass
time_step step1
categories stock
time_step_proportion 1.0
selectivities MaturationSel

@selectivity One
type constant
c 1

@selectivity MaturationSel
type all_values_bounded
l 2
h 4
v 0.50 0.75 0.97

@selectivity westFSel
type double_normal
mu 6
sigma_l 3
sigma_r 10
alpha 1.0

@selectivity eastFSel
type double_normal
mu 6
sigma_l 3
sigma_r 10
alpha 1.0

@selectivity chatTANSel
type double_normal
mu 6
sigma_l 3
sigma_r 10
alpha 1.0

@age_length age_size
type von_bertalanffy
by_length True
k  0.5
t0 -0.21
Linf 88.3
cv_first 0.1
length_weight size_weight3

@length_weight size_weight3
type basic
units kgs
a 2.0e-6
b 3.288

@observation process_observation
type process_proportions_at_length
likelihood multinomial
time_step step1
process instant_mort
process_proportion 0.5
categories stock
years 1990 1992 1993 1994 1995
selectivities chatTANSel
delta 1e-5
table obs
1990    0 0.2   0.3   0.1   0.2   0.2
1992    0 0.12  0.25  0.28  0.25  0.1
1993    0 0.0   0.05  0.05  0.10  0.80
1994    0 0.05  0.1   0.05  0.05  0.75
1995    0 0.3   0.4   0.2   0.05  0.05
end_table
table error_values
1990 25
1992 37
1993 31
1994 34
1995 22
end_table

@report DQ
type derived_quantity
)";

TEST_F(InternalEmptyModel, Process_Proportions_At_Length) {
  AddConfigurationLine(test_cases_process_proportions_at_length, __FILE__, 31);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(712.32380371700344, obj_function.score());

  Observation* observation = model_->managers()->observation()->GetObservation("process_observation");

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  ASSERT_EQ(5u, comparisons.size());

  unsigned year = 1992;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(6u, comparisons[year].size());
  EXPECT_EQ("stock", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(37, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(1.2961743194744091e-08, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(0, comparisons[year][0].score_);

  EXPECT_EQ("stock", comparisons[year][1].category_);
  EXPECT_DOUBLE_EQ(37, comparisons[year][1].error_value_);
  EXPECT_DOUBLE_EQ(0.023459035089163638, comparisons[year][1].expected_);
  EXPECT_DOUBLE_EQ(0.12, comparisons[year][1].observed_);
  EXPECT_DOUBLE_EQ(20.522606705368126, comparisons[year][1].score_);

  EXPECT_EQ("stock", comparisons[year][2].category_);
  EXPECT_DOUBLE_EQ(37, comparisons[year][2].error_value_);
  EXPECT_DOUBLE_EQ(0.084723891892122677, comparisons[year][2].expected_);
  EXPECT_DOUBLE_EQ(0.25, comparisons[year][2].observed_);
  EXPECT_DOUBLE_EQ(36.200331845656038, comparisons[year][2].score_);

  EXPECT_EQ("stock", comparisons[year][3].category_);
  EXPECT_DOUBLE_EQ(37, comparisons[year][3].error_value_);
  EXPECT_DOUBLE_EQ(0.59494554977885616, comparisons[year][3].expected_);
  EXPECT_DOUBLE_EQ(0.28000000000000003, comparisons[year][3].observed_);
  EXPECT_DOUBLE_EQ(21.33693773831784, comparisons[year][3].score_);

  EXPECT_EQ("stock", comparisons[year][4].category_);
  EXPECT_DOUBLE_EQ(37, comparisons[year][4].error_value_);
  EXPECT_DOUBLE_EQ(0.2968606612184741, comparisons[year][4].expected_);
  EXPECT_DOUBLE_EQ(0.25, comparisons[year][4].observed_);
  EXPECT_DOUBLE_EQ(24.602078412298997, comparisons[year][4].score_);

  EXPECT_EQ("stock", comparisons[year][5].category_);
  EXPECT_DOUBLE_EQ(37, comparisons[year][5].error_value_);
  EXPECT_DOUBLE_EQ(1.0849059640325116e-05, comparisons[year][5].expected_);
  EXPECT_DOUBLE_EQ(0.1, comparisons[year][5].observed_);
  EXPECT_DOUBLE_EQ(45.032704104380088, comparisons[year][5].score_);
}

// Test the process_proportions_at_length observation with bespoke length bins
const std::string process_proportions_bespoke_length_bins =
    R"(
@model
min_age 1
max_age 30
age_plus true
base_weight_units tonnes
initialisation_phases Equilibrium_phase
time_steps Jul_Jan Feb_Jun
length_bins 10 20 30 40 50 60 70 80 90 100 110 120
length_plus true
start_year 1975
final_year 2018
projection_final_year 2022

@categories
format sex
names male female
age_lengths age_len_m age_len_f

@initialisation_phase Equilibrium_phase
type Derived

@time_step Jul_Jan
processes Ageing Recruitment Instantaneous_Mortality

@time_step Feb_Jun
processes Instantaneous_Mortality

@process Recruitment
type recruitment_beverton_holt
categories male female
proportions 0.5 0.5
b0 60000
standardise_years          1975:2014
recruitment_multipliers 1.00 1.00 1.00 1.00 1.92 1.11 0.78 0.71 1.00 0.38 0.89 0.66 1.08 0.84 1.06 1.06 1.19 1.31 1.71 0.93 1.92 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00
steepness 0.8
ssb SSB
age 1

@process Ageing
type ageing
categories male female

@process Instantaneous_Mortality
type mortality_instantaneous
m 0.19
time_step_proportions 0.58 0.42
relative_m_by_age One
categories *
table catches
year    fisheryF
1975    120
1990    1897
2018    1033
end_table

table method
method    category  selectivity  u_max  time_step  penalty
fisheryF  *         fisheryFSel  0.7    Jul_Jan    fisheryCatchMustBeTaken
end_table

@derived_quantity SSB
type biomass
time_step Jul_Jan
categories male female
time_step_proportion 0.5
time_step_proportion_method weighted_sum
selectivities maturity_sel_m maturity_sel_f

@selectivity maturity_sel_m
type all_values_bounded
l 2
h 15
v 0.01 0.03    0.09    0.22    0.46    0.71    0.88    0.96    0.98    0.99    1.00    1.00    1.00    1.00

@selectivity maturity_sel_f
type all_values_bounded
l 2
h 15
v 0.01 0.02    0.05    0.11    0.23    0.43    0.64    0.81    0.91    0.96    0.98    0.99    1.00    1.00

@selectivity fisheryFSel
type logistic
a50 4
ato95 3
alpha 1

@selectivity One
type constant
c 1.0

@age_length age_len_m
type schnute
compatibility_option casal
by_length true
time_step_proportions 0.0 0.33
y1 22.3
y2 89.8
tau1 1
tau2 20
a 0.249
b 1.243
cv_first 0.1
distribution normal
length_weight len_wt_m

@age_length age_len_f
type schnute
compatibility_option casal
by_length true
time_step_proportions 0.0 0.33
y1 22.9
y2 109.9
tau1 1
tau2 20
a 0.147
b 1.457
cv_first 0.1
distribution normal
length_weight len_wt_f

@length_weight len_wt_m
type basic
units tonnes
a 2.13e-9
b 3.281

@length_weight len_wt_f
type basic
units tonnes
a 1.83e-9
b 3.314

@penalty fisheryCatchMustBeTaken
type process
log_scale True
multiplier 1000

@observation process_len_obs
type process_proportions_at_length
time_step Jul_Jan
process Instantaneous_Mortality
process_proportion 0.4
categories male+female
selectivities fisheryFSel
likelihood multinomial
delta 1e-11
length_bins 10	30	50	60	70	80	90	100	110	120
plus_group true
years 1990 
table obs
1990	0.002458274	0.027817311	0.035580282	0.020442489	0.073489455	0.064562039	0.129512227	0.340147496	0.172208565	0.133781861
end_table
table error_values
1990 19
end_table
)";

TEST_F(InternalEmptyModel, Process_Proportions_At_Length_Bespoke_Length_Bins) {
  AddConfigurationLine(process_proportions_bespoke_length_bins, __FILE__, 31);
  LoadConfiguration();
  model_->Start(RunMode::kBasic);

  Observation*                             observation = model_->managers()->observation()->GetObservation("process_len_obs");
  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();

  ASSERT_EQ(1u, comparisons.size());  // only one year of data

  unsigned year = 1990;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(10u, comparisons[year].size());  // 10 length bins

  EXPECT_EQ("male+female", comparisons[year][0].category_);

  // Check that the expected values sum to 1.0 (as they should be proportions)
  double expected_sum = 0.0;
  for (unsigned i = 0; i < comparisons[year].size(); ++i) {
    expected_sum += comparisons[year][i].expected_;
  }
  EXPECT_NEAR(1.0, expected_sum, 1e-9);
}

// Test for wrong length bins configuration
const std::string process_proportions_error_test =
    R"(
@model
min_age 1
max_age 30
age_plus true
base_weight_units tonnes
initialisation_phases Equilibrium_phase
time_steps Jul_Jan Feb_Jun
length_bins 10 20 30 40 50 60 70 80 90 100 110 120
length_plus true
start_year 1975
final_year 2018
projection_final_year 2022

@categories
format sex
names male female
age_lengths age_len_m age_len_f

@initialisation_phase Equilibrium_phase
type Derived

@time_step Jul_Jan
processes Ageing Recruitment Instantaneous_Mortality

@time_step Feb_Jun
processes Instantaneous_Mortality

@process Recruitment
type recruitment_beverton_holt
categories male female
proportions 0.5 0.5
b0 60000
standardise_years          1975:2014
recruitment_multipliers 1.00 1.00 1.00 1.00 1.92 1.11 0.78 0.71 1.00 0.38 0.89 0.66 1.08 0.84 1.06 1.06 1.19 1.31 1.71 0.93 1.92 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00
steepness 0.8
ssb SSB
age 1

@process Ageing
type ageing
categories male female

@process Instantaneous_Mortality
type mortality_instantaneous
m 0.19
time_step_proportions 0.58 0.42
relative_m_by_age One
categories *
table catches
year    fisheryF
1975    120
1990    1897
2018    1033
end_table

table method
method    category  selectivity  u_max  time_step  penalty
fisheryF  *         fisheryFSel  0.7    Jul_Jan    fisheryCatchMustBeTaken
end_table

@derived_quantity SSB
type biomass
time_step Jul_Jan
categories male female
time_step_proportion 0.5
time_step_proportion_method weighted_sum
selectivities maturity_sel_m maturity_sel_f

@selectivity maturity_sel_m
type all_values_bounded
l 2
h 15
v 0.01 0.03    0.09    0.22    0.46    0.71    0.88    0.96    0.98    0.99    1.00    1.00    1.00    1.00

@selectivity maturity_sel_f
type all_values_bounded
l 2
h 15
v 0.01 0.02    0.05    0.11    0.23    0.43    0.64    0.81    0.91    0.96    0.98    0.99    1.00    1.00

@selectivity fisheryFSel
type logistic
a50 4
ato95 3
alpha 1

@selectivity One
type constant
c 1.0

@age_length age_len_m
type schnute
compatibility_option casal
by_length true
time_step_proportions 0.0 0.33
y1 22.3
y2 89.8
tau1 1
tau2 20
a 0.249
b 1.243
cv_first 0.1
distribution normal
length_weight len_wt_m

@age_length age_len_f
type schnute
compatibility_option casal
by_length true
time_step_proportions 0.0 0.33
y1 22.9
y2 109.9
tau1 1
tau2 20
a 0.147
b 1.457
cv_first 0.1
distribution normal
length_weight len_wt_f

@length_weight len_wt_m
type basic
units tonnes
a 2.13e-9
b 3.281

@length_weight len_wt_f
type basic
units tonnes
a 1.83e-9
b 3.314

@penalty fisheryCatchMustBeTaken
type process
log_scale True
multiplier 1000

@observation process_len_obs
type process_proportions_at_length
time_step Jul_Jan
process Instantaneous_Mortality
process_proportion 0.4
categories male+female
selectivities fisheryFSel
likelihood multinomial
delta 1e-11
length_bins 10 31 50 60 70 80 90 100 110 120  # Incorrect - 31 is not in model length bins
plus_group true
years 1990 
table obs
1990	0.002458274	0.027817311	0.035580282	0.020442489	0.073489455	0.064562039	0.129512227	0.340147496	0.172208565	0.133781861
end_table
table error_values
1990 19
end_table
)";

TEST_F(InternalEmptyModel, Process_Proportions_At_Length_Error_Test) {
  AddConfigurationLine(process_proportions_error_test, __FILE__, 31);
  LoadConfiguration();
  EXPECT_THROW(model_->Start(RunMode::kBasic), std::string);
}

}  // namespace age
}  // namespace observations
}  // namespace niwa
#endif  // TESTMODE