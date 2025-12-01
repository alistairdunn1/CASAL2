/**
 * @file HarvestStrategyConstantU.Test.cpp
 * @author A Dunn
 * @date 01/04/2024
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 */
#ifdef TESTMODE

// Headers
#include "HarvestStrategyRampU.h"

#include <iostream>

#include "../../DerivedQuantities/Manager.h"
#include "../../Model/Models/Age.h"
#include "../../ObjectiveFunction/ObjectiveFunction.h"
#include "../../Projects/Manager.h"
#include "../../TestResources/Models/CasalComplex1.h"
#include "../../TestResources/TestFixtures/InternalEmptyModel.h"
#include "../../Utilities/RandomNumberGenerator.h"

// Namespaces
namespace niwa {
namespace projects {

using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

const string HarvestStrategyRampU_config =
    R"(
@project future_catch
type harvest_strategy_ramp_u
parameter process[fishing].catches
years 2001:2012
biomass_index ssb
reference_index ssb
u 0.0 0.05 0.1
reference_points 0.1 0.2 0.4
min_delta 0.0
max_delta 0.0
biomass_index_lag_years 1
current_catch 3700
multiplier 1.0
first_year 2003

@project future_ycs
type constant
parameter process[recruitment].recruitment_multipliers
years 2001:2012
values 0.478482 0.640663 0.640091 0.762361 0.560125 0.651637 0.764833 0.645498 0.678341 1.234 1.0423 1.4352
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Projects_HarvestStrategyRampU) {
  AddConfigurationLine(testresources::models::test_cases_models_casal_complex_1, "TestResources/Models/CasalComplex1.h", 28);
  AddConfigurationLine(HarvestStrategyRampU_config, __FILE__, 35);
  LoadConfiguration();
  model_->Start(RunMode::kProjection);

  Project* project = model_->managers()->project()->GetProject("future_catch");
  if (!project)
    LOG_FATAL() << "!project";

  DerivedQuantity* dq = model_->managers()->derived_quantity()->GetDerivedQuantity("ssb");
  if (!dq)
    LOG_FATAL() << "!dq";

  // test the values have been set to the ramp U catch value
  map<unsigned, Double>& values     = project->projected_parameters();
  vector<unsigned>       proj_years = {2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012};
  vector<double>         expected   = {3700.0,
                                       3700.0,
                                       1427066.1546919504,
                                       1331193.4549297791,
                                       1198659.3204704768,
                                       1053465.8860748436,
                                       936674.5625356012,
                                       853233.06229152402,
                                       789894.73295612459,
                                       742908.23955774086,
                                       715110.90327180922,
                                       706283.78341380821};
  for (unsigned yr_ndx = 0; yr_ndx < proj_years.size(); ++yr_ndx) {
    EXPECT_DOUBLE_EQ(expected[yr_ndx], values[proj_years[yr_ndx]]);
  }
}

}  // namespace projects
} /* namespace niwa */

#endif /* ifdef TESTMODE */
