/**
 * @file Estimate.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 29/09/2014
 * @section LICENSE
 *
 * Copyright NIWA Science �2014 - www.niwa.co.nz
 *
 * @section DESCRIPTION
 *
 * << Add Description >>
 */
#ifdef TESTMODE

// Headers
#include <iostream>

#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Estimates/Manager.h"
#include "Model/Model.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"
#include "TestResources/Models/TwoSexNoEstimates.h"

// Namespaces
namespace isam {
namespace estimates {

using std::cout;
using std::endl;
using isam::testfixtures::InternalEmptyModel;

const string estimate_single_target =
R"(
@estimate e1
parameter selectivity[FishingSel].a50
lower_bound 1
upper_bound 20
type beta
mu 0.3
sigma 0.05
a 0
b 10
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Estimates_Single_Target) {
  AddConfigurationLine(testresources::models::two_sex_no_estimates, "TestResources/Models/TwoSexNoEstimates.h", 28);
  AddConfigurationLine(estimate_single_target, __FILE__, 34);
  LoadConfiguration();

  ModelPtr model = Model::Instance();
  model->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = ObjectiveFunction::Instance();
  EXPECT_DOUBLE_EQ(1726.6295023192379, obj_function.score());

  EstimatePtr estimate = estimates::Manager::Instance().GetEstimate("selectivity[FishingSel].a50");
  if (!estimate)
    LOG_ERROR("!estimate");
  EXPECT_DOUBLE_EQ(estimate->value(), 7.2724038656178385);

  // Check results
  estimate->set_value(1.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -2476.5137933614251);
  estimate->set_value(2.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -2367.250113991935);
  estimate->set_value(3.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -2230.4867585646953);
  estimate->set_value(4.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -2066.4915312599851);
  estimate->set_value(5.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -1868.5574163359895);
}

/**
 *
 */
const string estimate_multiple_defined_targets_vector =
R"(
@estimate e1
parameter selectivity[FishingSel].a50
lower_bound 1
upper_bound 20
type beta
mu 0.3
sigma 0.05
a 0
b 10
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Estimates_Multiple_Defined_Targets_Vector) {
  AddConfigurationLine(testresources::models::two_sex_no_estimates, "TestResources/Models/TwoSexNoEstimates.h", 28);
  AddConfigurationLine(estimate_multiple_defined_targets_vector, __FILE__, 80);
  LoadConfiguration();

  ModelPtr model = Model::Instance();
  model->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = ObjectiveFunction::Instance();
  EXPECT_DOUBLE_EQ(1726.6295023192379, obj_function.score());

  EstimatePtr estimate = estimates::Manager::Instance().GetEstimate("selectivity[FishingSel].a50");
  if (!estimate)
    LOG_ERROR("!estimate");
  EXPECT_DOUBLE_EQ(estimate->value(), 7.2724038656178385);

  // Check results
  estimate->set_value(1.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -2476.5137933614251);
  estimate->set_value(2.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -2367.250113991935);
  estimate->set_value(3.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -2230.4867585646953);
  estimate->set_value(4.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -2066.4915312599851);
  estimate->set_value(5.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -1868.5574163359895);
}

/**
 *
 */
const string estimate_multiple_defined_targets_unsigned_map =
R"(
@estimate process[Fishing].catches(2000:2002)
type lognormal
lower_bound 28000 24000 47000
upper_bound 29000 25000 48000
mu 3 5 7
cv 4 6 8
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Estimates_Multiple_Defined_Targets_Unsigned_Map) {
  AddConfigurationLine(testresources::models::two_sex_no_estimates, "TestResources/Models/TwoSexNoEstimates.h", 28);
  AddConfigurationLine(estimate_multiple_defined_targets_unsigned_map, __FILE__, 126);
  LoadConfiguration();

  ModelPtr model = Model::Instance();
  model->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = ObjectiveFunction::Instance();
  EXPECT_DOUBLE_EQ(2764.0521558776445, obj_function.score());

  EstimatePtr estimate = estimates::Manager::Instance().GetEstimate("process[Fishing].catches(2000)");
  if (!estimate)
    LOG_ERROR("!estimate");
  EXPECT_DOUBLE_EQ(estimate->value(), 28323.203463000002);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 29.966307896614428);

  estimate = estimates::Manager::Instance().GetEstimate("process[Fishing].catches(2001)");
  if (!estimate)
    LOG_ERROR("!estimate");
  EXPECT_DOUBLE_EQ(estimate->value(), 24207.464203);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 24.757322431641612);

  estimate = estimates::Manager::Instance().GetEstimate("process[Fishing].catches(2002)");
  if (!estimate)
    LOG_ERROR("!estimate");
  EXPECT_DOUBLE_EQ(estimate->value(), 47279);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 25.007985625485446);

}

/**
 *
 */
const string estimate_multiple_defined_targets_string_map =
R"(
@estimate recruitment.proportions
type lognormal
parameter process[Recruitment].proportions(immature.male,immature.female)
lower_bound 0.4 0.4
upper_bound 0.6 0.6
mu 1 2
cv 3 4
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Estimates_Multiple_Defined_Targets_String_Map) {
  AddConfigurationLine(testresources::models::two_sex_no_estimates, "TestResources/Models/TwoSexNoEstimates.h", 28);
  AddConfigurationLine(estimate_multiple_defined_targets_string_map, __FILE__, 167);
  LoadConfiguration();

  ModelPtr model = Model::Instance();
  model->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = ObjectiveFunction::Instance();
  EXPECT_DOUBLE_EQ(2682.979986309937, obj_function.score());

  EstimatePtr estimate = estimates::Manager::Instance().GetEstimate("process[Recruitment].proportions(immature.male)");
  if (!estimate)
    LOG_ERROR("!estimate");
  EXPECT_DOUBLE_EQ(estimate->value(), 0.5);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -0.64756858783643167);

  estimate = estimates::Manager::Instance().GetEstimate("process[Recruitment].proportions(immature.female)");
  if (!estimate)
    LOG_ERROR("!estimate");
  EXPECT_DOUBLE_EQ(estimate->value(), 0.5);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -0.69298502612944257);
}

} /* namespace estimates */
} /* namespace isam */
#endif
