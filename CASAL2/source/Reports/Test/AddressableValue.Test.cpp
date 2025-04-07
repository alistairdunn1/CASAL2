/**
 * @file AddressableValue.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/07
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "AddressableValue.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Model/Managers.h"
#include "Model/Model.h"
#include "TestResources/Models/TwoSex.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"

namespace niwa::reports::test {
using niwa::testfixtures::InternalEmptyModel;

TEST_F(InternalEmptyModel, Report_Test_AddressableValue) {
  const string addressable_value = R"(
  @report test_report
  type addressable_value
  time_step step_two
  addressable process[Recruitment].r0
  )";

  AddConfigurationLine(niwa::testresources::models::two_sex, "TestResources/Models/TwoSex.h", 31);
  AddConfigurationLine(addressable_value, __FILE__, 23);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  Report* report = model_->managers()->report()->get("test_report");
  ASSERT_NE(nullptr, report);

  AddressableValue* avreport = dynamic_cast<AddressableValue*>(report);
  ASSERT_NE(nullptr, avreport);

  auto values = avreport->values();
  ASSERT_EQ(15u, values.size());
  EXPECT_EQ(values[1996], 997386);
}

}  // namespace niwa::reports::test
#endif  // TESTMODE