/**
 * @file MortalityInstantaneousLength.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/07
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 * @description Length-based tests for consolidated MortalityInstantaneous
 */
#ifdef TESTMODE

// Headers
#include <iomanip>

#include "Model/Model.h"
#include "MortalityInstantaneous.h"
#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Partition/Partition.h"
#include "Processes/Manager.h"
#include "TestResources/TestFixtures/InternalEmptyLengthModel.h"

// Namespaces
namespace niwa {
namespace processes {
namespace common {

using niwa::testfixtures::InternalEmptyLengthModel;

const std::string test_cases_process_mortality_instantaneous_length =
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
processes Recruit_BH growth mortality

@process Recruit_BH
type recruitment_beverton_holt
ssb_offset 1
standardise_years 1986:2010
initial_mean_length 5
initial_length_cv 0.40
recruitment_multipliers 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00  1.00 1.00 1.00 1.00 1.00 1.00
b0 150000
categories uni
proportions 1
steepness 0.75
ssb SSB

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

@selectivity maturity
type logistic
a50 30 
ato95 5

@selectivity One
type constant
c 1

@growth_increment growth_model
type basic
l_alpha 5
l_beta  10
g_alpha 7
g_beta 1
min_sigma 2
distribution normal
length_weight allometric
cv 0.0
compatibility_option casal
)";

const std::string test_cases_process_mortality_instantaneous_simple_length =
    R"(
@process mortality
type mortality_instantaneous
time_step_proportions 1
m 0.0798
relative_m_by_length One
categories uni
table catches
year FishingWest FishingEest
1986 282000 80000
1987 387000 122000
1988 385000 189000
1989 386000 418000
1990 309000 689000
1991 409000 503000
1992 718000 1087000
1993 656000 1996000
1994 368000 2912000
1995 597000 2903000
1996 1353000 2483000
1997 1475000 1820000
1998 1424000 1124000
1999 1169000 3339000
2000 1155000 2130000
2001 1208000 1700000
2002 454000 1058000
2003 497000 718000
2004 687000 1983000
2005 2585000 1434000
2006 184000 255000
2007 270000 683000
2008 259000 901000
2009 1069000 832000
2010 231000 159000
2011 822000 118000
2012 800000 150000
end_table

table method
method  category selectivity u_max time_step penalty
FishingWest   uni   westFSel 0.7 Annual none
FishingEest  uni   eastFSel 0.7 Annual none
end_table
)";

/**
 *
 */
TEST_F(InternalEmptyLengthModel, Processes_Mortality_Instantaneous_Length_Simple) {
  AddConfigurationLine(test_cases_process_mortality_instantaneous_length, __FILE__, 31);
  AddConfigurationLine(test_cases_process_mortality_instantaneous_simple_length, __FILE__, 136);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  vector<Double> expected = {14098614.78110135905,       129088683.48653472960,       1189099952.11722612381,      9685073663.45943832397,      66569971137.62071990967,
                             364014552498.40484619141,   1507426425073.91552734375,   4747903089668.97753906250,   12533326352844.74414062500,  31846363858248.38281250000,
                             81685851138416.75000000000, 192300662561015.87500000000, 325901473229625.81250000000, 291223789948903.75000000000, 199268453971695.59375000000};

  partition::Category& stock = model_->partition().category("uni");
  ASSERT_EQ(stock.data_.size(), expected.size());

  for (unsigned i = 0; i < stock.data_.size(); ++i) {
    EXPECT_NEAR(expected[i], stock.data_[i], 1e-9) << " with i = " << i;
  }
}

}  // namespace common
} /* namespace processes */
} /* namespace niwa */
#endif /* TESTMODE */
