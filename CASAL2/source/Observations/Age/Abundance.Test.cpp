/**
 * @file Abundance.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/06/10
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "Abundance.h"

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
TEST_F(InternalEmptyModel, Observation_Abundance_TwoSexComplex) {
  const std::string observation_definition =
      R"(
        @observation observation
        type abundance
        catchability catchability_one
        time_step step_one
        categories immature.male+immature.female immature.female
        selectivities logistic_one logistic_two logistic_three
        likelihood lognormal
        time_step_proportion 1.0
        years 2008
        table obs
        2008 22.50 11.25 0.2
        end_table
    )";

  AddConfigurationLine(testresources::models::two_sex_complex, "TwoSexComplex.h", 23);
  AddConfigurationLine(observation_definition, __FILE__, 32);
  LoadConfiguration();

  model_->Start(RunMode::kTesting);
  model_->FullIteration();

  auto observation_ptr = model_->managers()->observation()->GetObservation("observation");
  ASSERT_NE(nullptr, observation_ptr);

  const vector<obs::Comparison>& comparisons = observation_ptr->comparisons(2008);
  ASSERT_EQ(2u, comparisons.size());

  // Check first comparison
  unsigned index = 0;
  EXPECT_EQ("immature.male+immature.female", comparisons[index].category_);
  ASSERT_EQ(2u, comparisons[index].selectivities_.size());
  auto sel_it = comparisons[index].selectivities_.begin();
  EXPECT_EQ("logistic_one", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_two", *sel_it);
  EXPECT_EQ(0u, comparisons[index].age_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].length_);
  EXPECT_DOUBLE_EQ(114.07815198571596, comparisons[index].expected_);
  EXPECT_DOUBLE_EQ(22.5, comparisons[index].observed_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].process_error_);
  EXPECT_DOUBLE_EQ(0.2, comparisons[index].error_value_);
  EXPECT_DOUBLE_EQ(0.2, comparisons[index].adjusted_error_);
  EXPECT_DOUBLE_EQ(9.9999999999999994e-12, comparisons[index].delta_);
  EXPECT_DOUBLE_EQ(31.170031954974714, comparisons[index].score_);

  // Check second comparison
  ++index;
  EXPECT_EQ("immature.female", comparisons[index].category_);
  ASSERT_EQ(1u, comparisons[index].selectivities_.size());
  EXPECT_EQ("logistic_three", *comparisons[index].selectivities_.begin());
  EXPECT_EQ(0u, comparisons[index].age_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].length_);
  EXPECT_DOUBLE_EQ(59.505395164278127, comparisons[index].expected_);
  EXPECT_DOUBLE_EQ(11.25, comparisons[index].observed_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].process_error_);
  EXPECT_DOUBLE_EQ(0.2, comparisons[index].error_value_);
  EXPECT_DOUBLE_EQ(0.2, comparisons[index].adjusted_error_);
  EXPECT_DOUBLE_EQ(9.9999999999999994e-12, comparisons[index].delta_);
  EXPECT_DOUBLE_EQ(32.923790582157153, comparisons[index].score_);
}

const std::string test_cases_observation_abundance =
    R"(
@model
min_age 1
max_age 20
age_plus t
start_year 1994
final_year 2008
base_weight_units kgs
time_steps step_one

@categories
format stage.sex
names immature.male mature.male immature.female mature.female
age_lengths no_age_length*4

@age_length no_age_length
type none
length_weight no_length_weight

@length_weight no_length_weight
type none

@time_step step_one
processes ageing recruitment mortality

@ageing ageing
categories immature.male immature.female

@recruitment recruitment
type constant
categories immature.male immature.female
proportions 0.6 0.4
r0 100000
age 1

@mortality mortality
type constant_rate
categories immature.male immature.female mature.male mature.female
m 0.065
relative_m_by_age constant_one
time_step_proportions 1.0

@selectivity constant_one
type constant
c 1

@catchability catchability
type free
q 0.000153139

@observation abundance
type abundance
catchability catchability
time_step step_one
categories immature.male+immature.female immature.female
selectivities constant_one constant_one constant_one
likelihood lognormal
time_step_proportion 1.0
years 2008
table obs
2008 22.50 11.25 0.2
end_table

@report DQ
type derived_quantity

)";

TEST_F(InternalEmptyModel, Observation_Abundance) {
  AddConfigurationLine(test_cases_observation_abundance, __FILE__, 32);
  LoadConfiguration();

  model_->Start(RunMode::kTesting);
  model_->FullIteration();

  const vector<obs::Comparison>& comparisons = model_->managers()->observation()->GetObservation("abundance")->comparisons(2008);
  ASSERT_EQ(2u, comparisons.size());

  EXPECT_EQ("immature.male+immature.female", comparisons[0].category_);
  EXPECT_DOUBLE_EQ(0.2, comparisons[0].error_value_);
  EXPECT_DOUBLE_EQ(142.01537476494462, comparisons[0].expected_);
  EXPECT_DOUBLE_EQ(22.5, comparisons[0].observed_);
  EXPECT_DOUBLE_EQ(40.738892086047329, comparisons[0].score_);

  EXPECT_EQ("immature.female", comparisons[1].category_);
  EXPECT_DOUBLE_EQ(0.2, comparisons[1].error_value_);
  EXPECT_DOUBLE_EQ(56.806149905977861, comparisons[1].expected_);
  EXPECT_DOUBLE_EQ(11.25, comparisons[1].observed_);
  EXPECT_DOUBLE_EQ(31.002921785658106, comparisons[1].score_);
}

const std::string test_cases_observation_process_abundance =
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
type process_abundance
catchability [type=free; q=6.52606e-005]
delta 1e-10
time_step two
process halfm
categories male+female
selectivities male=[type=logistic; a50=9; ato95=4] female=[type=logistic; a50=9; ato95=4; alpha=0.7]
likelihood lognormal
years 1992:2001
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
TEST_F(InternalEmptyModel, Observation_Process_Abundance) {
  AddConfigurationLine(test_cases_observation_process_abundance, __FILE__, 32);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(104357.9915848384, obj_function.score());

  Observation* observation = model_->managers()->observation()->GetObservation("observation");

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  ASSERT_EQ(10u, comparisons.size());

  unsigned year = 1992;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(-20.151984669493967, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.50, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(10608.527215563498, comparisons[year][0].score_);

  year = 1993;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(-20.148188402164081, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.1000000000000001, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(10475.957800116032, comparisons[year][0].score_);

  year = 1996;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(-20.135041918475192, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.53, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(10616.654030879328, comparisons[year][0].score_);

  year = 1998;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(-20.12878746564024, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(0.68000000000000005, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(10271.741196680667, comparisons[year][0].score_);

  year = 2001;
  ASSERT_FALSE(comparisons.find(year) == comparisons.end());
  ASSERT_EQ(1u, comparisons[year].size());
  EXPECT_EQ("male+female", comparisons[year][0].category_);
  EXPECT_DOUBLE_EQ(0.35, comparisons[year][0].error_value_);
  EXPECT_DOUBLE_EQ(-20.123154162672272, comparisons[year][0].expected_);
  EXPECT_DOUBLE_EQ(1.23, comparisons[year][0].observed_);
  EXPECT_DOUBLE_EQ(10523.04778953198, comparisons[year][0].score_);
}

}  // namespace niwa::observations::age
#endif  // TESTMODE