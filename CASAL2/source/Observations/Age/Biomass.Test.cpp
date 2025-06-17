/**
 * @file Biomass.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/06/10
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "Biomass.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Observations/Manager.h"
#include "TestResources/Models/TwoSexComplex.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"

namespace niwa::observations::age {
using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

/**
 * This unit test is specifically for ensuring we have the right values
 * being populated in the comparison object. Very carefully we're concerned
 * about the selectivities and the categories they're associated with.
 */
TEST_F(InternalEmptyModel, Observation_Biomass_TwoSexComplex) {
  const std::string observation_definition =
      R"(
        @observation observation
        type process_biomass
        catchability catchability_one
        time_step step_one
        process halfM
        categories immature.male+immature.female immature.female
        selectivities logistic_one logistic_two logistic_three
        likelihood lognormal
        years 1994:2003
        delta 1e-10
        table obs
        1994    1.50   1.45   0.35 
        1995    1.10   1.05   0.35
        1996    0.93   0.90   0.35
        1997    1.33   1.27   0.35
        1998    1.53   1.45   0.35
        1999    0.90   0.89   0.35
        2000    0.68   0.65   0.35
        2001    0.75   0.72   0.35
        2002    0.57   0.55   0.35
        2003    1.23   1.15   0.35
        end_table
    )";

  AddConfigurationLine(testresources::models::two_sex_complex, "TwoSexComplex.h", 23);
  AddConfigurationLine(observation_definition, __FILE__, 32);
  LoadConfiguration();

  model_->Start(RunMode::kTesting);
  model_->FullIteration();

  auto observation_ptr = model_->managers()->observation()->GetObservation("observation");
  ASSERT_NE(nullptr, observation_ptr);

  map<unsigned, vector<obs::Comparison> >& comparisons = observation_ptr->comparisons();
  ASSERT_EQ(10u, comparisons.size());

  // Check first comparison
  unsigned year  = 1994;
  unsigned index = 0;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(2u, comparisons[year].size());
  EXPECT_EQ("immature.male+immature.female", comparisons[year][index].category_);
  ASSERT_EQ(2u, comparisons[year][index].selectivities_.size());
  auto sel_it = comparisons[year][index].selectivities_.begin();
  EXPECT_EQ("logistic_one", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_two", *sel_it);
  EXPECT_EQ(0u, comparisons[year][index].age_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[year][index].length_);
  EXPECT_DOUBLE_EQ(-1.4908944284221839, comparisons[year][index].expected_);
  EXPECT_DOUBLE_EQ(1.5, comparisons[year][index].observed_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[year][index].process_error_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][index].error_value_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][index].adjusted_error_);
  EXPECT_DOUBLE_EQ(1e-10, comparisons[year][index].delta_);
  EXPECT_DOUBLE_EQ(9522.0485964286709, comparisons[year][index].score_);

  // // Check second comparison
  ++index;
  EXPECT_EQ("immature.female", comparisons[year][index].category_);
  ASSERT_EQ(1u, comparisons[year][index].selectivities_.size());
  EXPECT_EQ("logistic_three", *comparisons[year][index].selectivities_.begin());
  EXPECT_EQ(0u, comparisons[year][index].age_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[year][index].length_);
  EXPECT_DOUBLE_EQ(-1.0340977302112651, comparisons[year][index].expected_);
  EXPECT_DOUBLE_EQ(1.45, comparisons[year][index].observed_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[year][index].process_error_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][index].error_value_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][index].adjusted_error_);
  EXPECT_DOUBLE_EQ(1e-10, comparisons[year][index].delta_);
  EXPECT_DOUBLE_EQ(9360.4503073791784, comparisons[year][index].score_);
}

const std::string test_cases_observation_process_biomass =
    R"(
@model
min_age 2
max_age 25
age_plus t
start_year 1975
final_year 2002
base_weight_units kgs
initialisation_phases phase1
time_steps one two three

@categories
format sex
names male female
age_lengths age_size_male age_size_female

@initialisation_phase phase1
years 200
exclude_processes fishing

@time_step one
processes halfm fishing

@time_step two
processes halfm

@time_step three
processes recruitment ageing

@derived_quantity ssb
type biomass
categories male female
time_step_proportion 1.0
selectivities male_maturity female_maturity
time_step one

@ageing ageing
categories male female

@recruitment recruitment
type beverton_holt
categories male female
proportions 0.5 0.5
r0 5e6
age 2
steepness 0.9
recruitment_multipliers 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00
ssb ssb
standardise_years 1975:2001

@mortality halfm
type constant_rate
categories male female
relative_m_by_age [type=constant; c=1] halfm.one
m 0.20 0.20
time_step_proportions 0.5 0.5

@mortality fishing
type event_biomass
categories male female
years 1975:2002
catches 1191 1488 1288 2004 609 750 997 596 302 344 544 362 509 574 804 977 991 2454 2775 2898 4094 3760 3761 3673 3524 3700 3700 3700
U_max 0.9
selectivities observation.male observation.female
penalty event_mortality_penalty

@selectivity male_maturity
type logistic
a50 5
ato95 2

@selectivity female_maturity
type logistic
a50 5
ato95 2

@selectivity one
type constant
c 1

@age_length age_size_male
type von_bertalanffy
length_weight [type=none]
k 0.277
t0 0.11
linf 90.3

@age_length age_size_female
type von_bertalanffy
length_weight [type=none]
k 0.202
t0 -0.20
linf 113.4

@penalty event_mortality_penalty
type process
log_scale True
multiplier 10

@observation observation
type process_biomass
catchability [type=free; q=6.52606e-005]
time_step one
process halfm
categories male+female
selectivities male=[type=logistic; a50=9; ato95=4] female=[type=logistic; a50=9; ato95=4; alpha=0.7]
likelihood lognormal
years 1992:2001
delta 1e-10
table obs
1992    1.50   0.35
1993    1.10   0.35
1994    0.93   0.35
1995    1.33   0.35
1996    1.53   0.35
1997    0.90   0.35
1998    0.68   0.35
1999    0.75   0.35
2000    0.57   0.35
2001    1.23   0.35
end_table

@report DQ
type derived_quantity
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Observation_Process_Biomass) {
  AddConfigurationLine(test_cases_observation_process_biomass, __FILE__, 32);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(104784.87854324422, obj_function.score());

  Observation* observation = model_->managers()->observation()->GetObservation("observation");

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  ASSERT_EQ(10u, comparisons.size());

  unsigned year = 1992;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(-22.276816208066737, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.50, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(10651.526433300138, comparisons[year][0].score_);

  year = 1993;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(-22.273330558515898, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.1000000000000001, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(10518.701422461665, comparisons[year][0].score_);

  year = 1996;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(-22.260979027504177, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.53, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(10659.725504899203, comparisons[year][0].score_);

  year = 1998;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(-22.253873471029078, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(0.68000000000000005, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(10314.104407769524, comparisons[year][0].score_);

  year = 2001;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(-22.24770655278946, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.23, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(10565.926685658847, comparisons[year][0].score_);
}

const std::string test_cases_observation_biomass =
    R"(
@model
min_age 2
max_age 25
age_plus t
start_year 1975
final_year 2002
base_weight_units kgs
initialisation_phases phase1
time_steps one two three

@categories
format sex
names male female
age_lengths age_size_male age_size_female

@initialisation_phase phase1
years 200
exclude_processes fishing

@time_step one
processes halfm fishing halfm

@time_step two
processes recruitment

@time_step three
processes ageing

@derived_quantity ssb
type biomass
categories male female
time_step_proportion 1.0
selectivities male_maturity female_maturity
time_step one

@ageing ageing
categories male female

@recruitment recruitment
type beverton_holt
categories male female
proportions 0.5 0.5
r0 5e6
age 2
steepness 0.9
recruitment_multipliers 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00
ssb ssb
standardise_years 1975:2001

@mortality halfm
type constant_rate
categories male female
relative_m_by_age [type=constant; c=1] halfm.one
m 0.10 0.10
time_step_proportions 1.0

@mortality fishing
type event_biomass
categories male female
years 1975:2002
catches 1191 1488 1288 2004 609 750 997 596 302 344 544 362 509 574 804 977 991 2454 2775 2898 4094 3760 3761 3673 3524 3700 3700 3700
U_max 0.9
selectivities observation.male observation.female
penalty event_mortality_penalty

@selectivity male_maturity
type logistic
a50 5
ato95 2

@selectivity female_maturity
type logistic
a50 5
ato95 2

@selectivity one
type constant
c 1

@age_length age_size_male
type von_bertalanffy
length_weight [type=none]
k 0.277
t0 0.11
linf 90.3

@age_length age_size_female
type von_bertalanffy
length_weight [type=none]
k 0.202
t0 -0.20
linf 113.4

@penalty event_mortality_penalty
type process
log_scale True
multiplier 10

@observation observation
type biomass
catchability [type=free; q=6.52606e-005]
time_step one
categories male+female
selectivities male=[type=logistic; a50=9; ato95=4] female=[type=logistic; a50=9; ato95=4; alpha=0.7]
likelihood lognormal
years 1992:2001
time_step_proportion 1.0
delta 1e-10
table obs
1992    1.50   0.35
1993    1.10   0.35
1994    0.93   0.35
1995    1.33   0.35
1996    1.53   0.35
1997    0.90   0.35
1998    0.68   0.35
1999    0.75   0.35
2000    0.57   0.35
2001    1.23   0.35
end_table

@report DQ
type derived_quantity

)";

/**
 *
 */
TEST_F(InternalEmptyModel, Observation_Biomass) {
  AddConfigurationLine(test_cases_observation_biomass, __FILE__, 32);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(1494.5386400678647, obj_function.score());

  Observation* observation = model_->managers()->observation()->GetObservation("observation");

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  ASSERT_EQ(10u, comparisons.size());

  unsigned year = 1992;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(383.22351916713291, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.50, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(129.11208636163323, comparisons[year][0].score_);

  year = 1993;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(383.15132682728552, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.1000000000000001, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(144.24143138683561, comparisons[year][0].score_);

  year = 1996;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(382.90132456564459, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.53, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(128.13400466543285, comparisons[year][0].score_);

  year = 1998;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(382.78238573826928, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(0.68000000000000005, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(169.31120799784219, comparisons[year][0].score_);

  year = 2001;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(382.67525910913929, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.23, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(138.63223564386561, comparisons[year][0].score_);
}

}  // namespace niwa::observations::age
#endif  // TESTMODE