/**
 * @file MultiSelectivity.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/03/13
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "MultiSelectivity.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "../../Selectivities/Manager.h"
#include "../../Selectivities/Selectivity.h"
#include "../../TestResources/MockClasses/Managers.h"
#include "../../TestResources/MockClasses/Model.h"
#include "../../TestResources/TestFixtures/InternalEmptyModel.h"
#include "InverseLogistic.h"
#include "Logistic.h"

namespace niwa {
using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;
using ::testing::_;
using ::testing::Invoke;

class MockSelectivitymanager : public selectivities::Manager {
public:
  MockSelectivitymanager()  = default;
  ~MockSelectivitymanager() = default;
};

const string basic_model = R"(
@model
min_age 5
max_age 10
start_year 1990
final_year 1999
time_steps step1

@categories
format sex
names uni

@time_step step1
processes [type=ageing; categories=*]

@selectivity inverse_logistic
type inverse_logistic
a50 2
ato95 7

@selectivity logistic
type logistic
a50 2
ato95 7

@selectivity multi
type multi_selectivity
years 1990 1991 1992
selectivity_labels inverse_logistic inverse_logistic inverse_logistic
default_selectivity logistic
projection_selectivity logistic
)";

TEST_F(InternalEmptyModel, MultiSelectivity) {
  AddConfigurationLine(basic_model, __FILE__, 40);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);
  niwa::selectivities::Manager* selectivity_manager = model_->managers()->selectivity();
  ASSERT_NE(selectivity_manager, nullptr);

  auto* inverse_logistic = selectivity_manager->GetSelectivity("inverse_logistic");
  ASSERT_NE(inverse_logistic, nullptr);

  auto* logistic = selectivity_manager->GetSelectivity("logistic");
  ASSERT_NE(logistic, nullptr);

  auto* multi = selectivity_manager->GetSelectivity("multi");
  ASSERT_NE(multi, nullptr);

  model_->set_current_year(1990);
  EXPECT_DOUBLE_EQ(0.15675987867112151, inverse_logistic->GetAgeResult(6, nullptr));  // At model->min_age()
  EXPECT_DOUBLE_EQ(0.10878902775125598, inverse_logistic->GetAgeResult(7, nullptr));
  EXPECT_DOUBLE_EQ(0.074206130458434716, inverse_logistic->GetAgeResult(8, nullptr));
  EXPECT_DOUBLE_EQ(0.15675987867112151, multi->GetAgeResult(6, nullptr));  // At model->min_age()
  EXPECT_DOUBLE_EQ(0.10878902775125598, multi->GetAgeResult(7, nullptr));
  EXPECT_DOUBLE_EQ(0.074206130458434716, multi->GetAgeResult(8, nullptr));

  model_->set_current_year(1993);
  EXPECT_DOUBLE_EQ(0.84324012132887849, logistic->GetAgeResult(6, nullptr));  // At model->min_age()
  EXPECT_DOUBLE_EQ(0.89121097224874402, logistic->GetAgeResult(7, nullptr));
  EXPECT_DOUBLE_EQ(0.92579386954156528, logistic->GetAgeResult(8, nullptr));
  EXPECT_DOUBLE_EQ(0.84324012132887849, multi->GetAgeResult(6, nullptr));  // At model->min_age()
  EXPECT_DOUBLE_EQ(0.89121097224874402, multi->GetAgeResult(7, nullptr));
  EXPECT_DOUBLE_EQ(0.92579386954156528, multi->GetAgeResult(8, nullptr));
}

}  // namespace niwa
#endif  // TESTMODE