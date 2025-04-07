/**
 * @file AnnualShift.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/07
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "AnnualShift.h"

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

TEST_F(InternalEmptyModel, Timevarying_Annual_Shift) {
  const string extra_config = R"(
  @time_varying annual_shift
  type annual_shift
  parameter selectivity[FishingSel].a50
  values 1.0 1.1 1.2
  years  1994 1995 1996
  a 8
  b 0
  c 0

  @report test_report
  type addressable_value
  time_step step_two
  addressable selectivity[FishingSel].a50
  )";

  AddConfigurationLine(niwa::testresources::models::two_sex, "TestResources/Models/TwoSex.h", 31);
  AddConfigurationLine(extra_config, __FILE__, 27);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  auto* time_varying = model_->managers()->time_varying()->get("annual_shift");
  ASSERT_NE(nullptr, time_varying);

  AnnualShift* shift = dynamic_cast<AnnualShift*>(time_varying);
  ASSERT_NE(nullptr, shift);

  auto values = shift->values_by_year();
  ASSERT_EQ(3u, values.size());
  EXPECT_NEAR(values[1994], 7.2, 1e-5);
  EXPECT_NEAR(values[1995], 8, 1e-5);
  EXPECT_NEAR(values[1996], 8.8, 1e-5);

  Report* report = model_->managers()->report()->get("test_report");
  ASSERT_NE(nullptr, report);

  reports::test::AddressableValue* avreport = dynamic_cast<reports::test::AddressableValue*>(report);
  ASSERT_NE(nullptr, avreport);

  auto report_values = avreport->values();
  ASSERT_EQ(15u, report_values.size());
  EXPECT_NEAR(report_values[1994], 7.2, 1e-5);
  EXPECT_NEAR(report_values[1995], 8, 1e-5);
  EXPECT_NEAR(report_values[1996], 8.8, 1e-5);
  EXPECT_NEAR(report_values[1997], 8, 1e-5);
}

}  // namespace niwa::timevarying
#endif  // TESTMODE