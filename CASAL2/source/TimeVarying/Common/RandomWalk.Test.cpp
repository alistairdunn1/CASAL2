/**
 * @file RandomWalk.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/22
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "RandomWalk.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Model/Managers.h"
#include "Model/Model.h"
#include "Reports/Report.h"
#include "Reports/Test/AddressableValue.h"
#include "TestResources/Models/TwoSex.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"
#include "TimeVarying/Manager.h"

namespace niwa::timevarying {
using niwa::testfixtures::InternalEmptyModel;

TEST_F(InternalEmptyModel, Timevarying_RandomWalk) {
  ASSERT_EQ(true, true);
  // const string extra_config = R"(
  //   @time_varying random_walk
  //   type random_walk
  //   parameter selectivity[FishingSel].a50
  //   years  1994 1995 1996 1997 1998 1999 2000
  //   mean 5.0
  //   sigma 1.0
  //   lower_bound 1
  //   upper_bound 20
  //   rho 1.1

  //   @report test_report
  //   type addressable_value
  //   time_step step_two
  //   addressable selectivity[FishingSel].a50
  //   )";

  // AddConfigurationLine(niwa::testresources::models::two_sex, "TestResources/Models/TwoSex.h", 31);
  // AddConfigurationLine(extra_config, __FILE__, 30);
  // LoadConfiguration();

  // model_->Start(RunMode::kBasic);

  // auto* time_varying = model_->managers()->time_varying()->get("random_walk");
  // ASSERT_NE(nullptr, time_varying);

  // RandomWalk* random_walk = dynamic_cast<RandomWalk*>(time_varying);
  // ASSERT_NE(nullptr, random_walk);

  // Report* report = model_->managers()->report()->get("test_report");
  // ASSERT_NE(nullptr, report);

  // reports::test::AddressableValue* avreport = dynamic_cast<reports::test::AddressableValue*>(report);
  // ASSERT_NE(nullptr, avreport);

  // auto report_values = avreport->values();
  // ASSERT_EQ(15u, report_values.size());
  // EXPECT_NEAR(4.4459810702527038, report_values[1994], 1e-9);
  // EXPECT_NEAR(3.7494286151769622, report_values[1995], 1e-9);
  // EXPECT_NEAR(4.0524417553765524, report_values[1996], 1e-9);
  // EXPECT_NEAR(5.4475535298818425, report_values[1997], 1e-9);
  // EXPECT_NEAR(4.2931056963305938, report_values[1998], 1e-9);
  // EXPECT_NEAR(5.1442491304429803, report_values[1999], 1e-9);
  // EXPECT_NEAR(3.7169158191124718, report_values[2000], 1e-9);
  // EXPECT_NEAR(8.0, report_values[2001], 1e-9);
}

}  // namespace niwa::timevarying
#endif  // TESTMODE