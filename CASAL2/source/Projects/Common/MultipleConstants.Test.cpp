/**
 * @file MultipleConstants.Test.cpp
 * @author Your Name
 * @date 2024-03-25
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 */
#ifdef TESTMODE

// Headers
#include "MultipleConstants.h"

#include <iostream>

#include "../../AddressableInputLoader/AddressableInputLoader.h"
#include "../../DerivedQuantities/Manager.h"
#include "../../Model/Models/Age.h"
#include "../../ObjectiveFunction/ObjectiveFunction.h"
#include "../../Projects/Manager.h"
#include "../../Reports/Manager.h"
#include "../../Reports/Report.h"
#include "../../Reports/Test/ObjectiveFunction.h"
#include "../../TestResources/Models/TwoSex.h"
#include "../../TestResources/TestFixtures/InternalEmptyModel.h"

// Namespaces
namespace niwa::projects {

using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

/*
 * Input projection with multiple constants for different -i values
 */
const string multiple_constants_recruitment_r0_baseline = R"(
@project future_ycs
type multiple_values
parameter process[Recruitment].r0
years 1994:2012
table values
1994 1995 1996 1997 1998 1999 2000 2001 2002 2003 2004 2005 2006 2007 2008 2009 2010 2011 2012
500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000
500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000
500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000 500000
end_table

@report test_objective_function
type test_objective_function
)";

/**
 * This unit test is going to check that everything compiles and loads perfectly in to the model.
 * We're not varying the process[Recruitment].r0 value at all because we want to get a baseline
 * objective function value to use.
 */
TEST_F(InternalEmptyModel, Projects_MultipleConstants_Recruitment_R0_BaseLine) {
  AddConfigurationLine(testresources::models::two_sex, __FILE__, __LINE__);
  AddConfigurationLine(multiple_constants_recruitment_r0_baseline, __FILE__, 39);
  LoadConfiguration();

  // model_->Start(RunMode::kTesting);

  AddressableInputLoader* loader = model_->managers()->addressable_input_loader();

  // Add different values for R0
  loader->AddValue("process[Recruitment].r0", 500000.0);
  loader->AddValue("process[Recruitment].r0", 1000000.0);
  loader->AddValue("process[Recruitment].r0", 2000000.0);

  loader->AddValue("catchability[CPUEq].q", 0.000153139);
  loader->AddValue("catchability[CPUEq].q", 0.000153139);
  loader->AddValue("catchability[CPUEq].q", 0.000153139);

  loader->AddValue("selectivity[FishingSel].a50", 8);
  loader->AddValue("selectivity[FishingSel].a50", 8);
  loader->AddValue("selectivity[FishingSel].a50", 8);

  loader->AddValue("selectivity[FishingSel].ato95", 3);
  loader->AddValue("selectivity[FishingSel].ato95", 3);
  loader->AddValue("selectivity[FishingSel].ato95", 3);

  model_->Start(RunMode::kProjection);

  niwa::reports::test::ObjectiveFunction* report = dynamic_cast<niwa::reports::test::ObjectiveFunction*>(model_->managers()->report()->get("test_objective_function"));
  if (report == nullptr)
    LOG_FATAL() << "!report";

  auto scores = report->scores();
  ASSERT_EQ(3u, scores.size());
  EXPECT_NEAR(2497.0178025982541, scores[0], 1e-9);
  EXPECT_NEAR(2297.5614212170476, scores[1], 1e-9);
  EXPECT_NEAR(2688.5998404835541, scores[2], 1e-9);
}

/*
 * Input projection with multiple constants for different -i values
 */
const string multiple_constants_recruitment_r0_change_step_inputs = R"(
  @project future_ycs
  type multiple_values
  parameter process[Recruitment].r0
  years 2009:2012
  table values
  2009 2010 2011 2012
  500000 500000 500000 500000
  1000000 1000000 1000000 1000000
  2000000 2000000 2000000 2000000
  end_table
  
  @report test_objective_function
  type test_objective_function
  )";

/**
 * In this unit test, we're making the values that were already being used.
 */
TEST_F(InternalEmptyModel, Projects_MultipleConstants_Recruitment_R0_ChangeStepInputs) {
  AddConfigurationLine(testresources::models::two_sex, __FILE__, __LINE__);
  AddConfigurationLine(multiple_constants_recruitment_r0_change_step_inputs, __FILE__, 39);
  LoadConfiguration();

  // model_->Start(RunMode::kTesting);

  AddressableInputLoader* loader = model_->managers()->addressable_input_loader();

  // Add different values for R0
  loader->AddValue("process[Recruitment].r0", 500000.0);
  loader->AddValue("process[Recruitment].r0", 1000000.0);
  loader->AddValue("process[Recruitment].r0", 2000000.0);

  loader->AddValue("catchability[CPUEq].q", 0.000153139);
  loader->AddValue("catchability[CPUEq].q", 0.000153139);
  loader->AddValue("catchability[CPUEq].q", 0.000153139);

  loader->AddValue("selectivity[FishingSel].a50", 8);
  loader->AddValue("selectivity[FishingSel].a50", 8);
  loader->AddValue("selectivity[FishingSel].a50", 8);

  loader->AddValue("selectivity[FishingSel].ato95", 3);
  loader->AddValue("selectivity[FishingSel].ato95", 3);
  loader->AddValue("selectivity[FishingSel].ato95", 3);

  model_->Start(RunMode::kProjection);

  niwa::reports::test::ObjectiveFunction* report = dynamic_cast<niwa::reports::test::ObjectiveFunction*>(model_->managers()->report()->get("test_objective_function"));
  if (report == nullptr)
    LOG_FATAL() << "!report";

  auto scores = report->scores();
  ASSERT_EQ(3u, scores.size());
  EXPECT_NEAR(2497.0178025982541, scores[0], 1e-9);
  EXPECT_NEAR(2699.2673782776801, scores[1], 1e-9);
  EXPECT_NEAR(3078.3356358777787, scores[2], 1e-9);
}

}  // namespace niwa::projects

#endif /* ifdef TESTMODE */
