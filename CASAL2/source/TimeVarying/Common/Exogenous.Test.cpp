/**
 * @file Exogenous.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/07
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "Exogenous.h"

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

TEST_F(InternalEmptyModel, Timevarying_Exogenous) {
  const string extra_config = R"(
  @time_varying exogenous
  type exogenous
  parameter selectivity[FishingSel].a50
  years  1994 1995 1996
  exogenous_variable 3 5 8
  a 0.5
  
  @report test_report
  type addressable_value
  time_step step_two
  addressable selectivity[FishingSel].a50
  )";

  AddConfigurationLine(niwa::testresources::models::two_sex, "TestResources/Models/TwoSex.h", 31);
  AddConfigurationLine(extra_config, __FILE__, 27);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  auto* time_varying = model_->managers()->time_varying()->get("exogenous");
  ASSERT_NE(nullptr, time_varying);

  Exogenous* varying = dynamic_cast<Exogenous*>(time_varying);
  ASSERT_NE(nullptr, varying);

  EXPECT_NEAR(varying->mean_value(), 5.333333333333333, 1e-9);

  auto values = varying->values_by_year();
  ASSERT_EQ(3u, values.size());
  EXPECT_NEAR(values[1994], 3, 1e-5);
  EXPECT_NEAR(values[1995], 5, 1e-5);
  EXPECT_NEAR(values[1996], 8, 1e-5);

  Report* report = model_->managers()->report()->get("test_report");
  ASSERT_NE(nullptr, report);

  reports::test::AddressableValue* avreport = dynamic_cast<reports::test::AddressableValue*>(report);
  ASSERT_NE(nullptr, avreport);

  auto report_values = avreport->values();
  ASSERT_EQ(15u, report_values.size());
  EXPECT_NEAR(report_values[1994], 6.8333333333333339, 1e-5);
  EXPECT_NEAR(report_values[1995], 7.8333333333333339, 1e-5);
  EXPECT_NEAR(report_values[1996], 9.3333333333333339, 1e-5);
  EXPECT_NEAR(report_values[1997], 8, 1e-5);
}

}  // namespace niwa::timevarying
#endif  // TESTMODE