/**
 * @file Lognormal.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 24/09/2014
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

// Headers
#include <iostream>

#include "../../Estimates/Manager.h"
#include "../../Model/Models/Age.h"
#include "../../ObjectiveFunction/ObjectiveFunction.h"
#include "../../TestResources/Models/TwoSexNoEstimates.h"
#include "../../TestResources/TestFixtures/InternalEmptyModel.h"

// Namespaces
namespace niwa {
namespace estimates {

using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

const string estimate_student_t_standard_deviation =
    R"(
@estimate
parameter selectivity[FishingSel].a50
type students_t
lower_bound 1
upper_bound 20
mu 0
sigma 1.0
df 3
)";

const string estimate_student_t_case2 =
    R"(
@estimate
parameter selectivity[FishingSel].a50
type students_t
lower_bound 1
upper_bound 20
mu 5
sigma 2.0
df 5
)";

const string estimate_student_t_case3 =
    R"(
@estimate
parameter selectivity[FishingSel].a50
type students_t
lower_bound 1
upper_bound 20
mu 10
sigma 0.5
df 10
)";

const string estimate_student_t_case4 =
    R"(
@estimate
parameter selectivity[FishingSel].a50
type students_t
lower_bound 1
upper_bound 20
mu 2
sigma 3.0
df 2
)";

const string estimate_student_t_case5 =
    R"(
@estimate
parameter selectivity[FishingSel].a50
type students_t
lower_bound 1
upper_bound 20
mu 15
sigma 1.5
df 20
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Students_T_StandardDistribution) {
  AddConfigurationLine(testresources::models::two_sex_no_estimates, "TestResources/Models/TwoSexNoEstimates.h", 33);
  AddConfigurationLine(estimate_student_t_standard_deviation, __FILE__, 30);
  LoadConfiguration();

  model_->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(2566.4568865662127, obj_function.score());

  Estimate* estimate = model_->managers()->estimate()->GetEstimate("selectivity[FishingSel].a50");
  if (!estimate)
    LOG_FATAL() << "!estimate";
  EXPECT_DOUBLE_EQ(estimate->value(), 8.9808675586857447);

  // Check results
  estimate->set_value(1.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 0.57536414490356169);
  estimate->set_value(2.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 1.6945957207744069);
  estimate->set_value(3.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 2.7725887222397811);
  estimate->set_value(4.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 3.6916533809966614);
  estimate->set_value(5.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 4.4671844430141885);
}

/**
 *
 */
TEST_F(InternalEmptyModel, Students_T_AlternativeParameters_Mu5_Sigma2_Df5) {
  AddConfigurationLine(testresources::models::two_sex_no_estimates, "TestResources/Models/TwoSexNoEstimates.h", 33);
  AddConfigurationLine(estimate_student_t_case2, __FILE__, 44);
  LoadConfiguration();

  model_->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(2561.5511824374225, obj_function.score());

  Estimate* estimate = model_->managers()->estimate()->GetEstimate("selectivity[FishingSel].a50");
  if (!estimate)
    LOG_FATAL() << "!estimate";
  EXPECT_DOUBLE_EQ(estimate->value(), 8.979979833957195);

  // Check results
  estimate->set_value(1.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 1.7633599947063572);
  estimate->set_value(5.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 0.0);
  estimate->set_value(10.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 2.4327906486489863);
  estimate->set_value(15.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 5.3752784076841653);
  estimate->set_value(20.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 7.5165778109722083);
}

/**
 *
 */
TEST_F(InternalEmptyModel, Students_T_NarrowDistribution_Mu10_Sigma0p5_Df10) {
  AddConfigurationLine(testresources::models::two_sex_no_estimates, "TestResources/Models/TwoSexNoEstimates.h", 33);
  AddConfigurationLine(estimate_student_t_case3, __FILE__, 58);
  LoadConfiguration();

  model_->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(2560.8244427289137, obj_function.score());

  Estimate* estimate = model_->managers()->estimate()->GetEstimate("selectivity[FishingSel].a50");
  if (!estimate)
    LOG_FATAL() << "!estimate";
  EXPECT_DOUBLE_EQ(estimate->value(), 8.9932187899778739);

  // Check results
  estimate->set_value(8.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 4.0844100445772362);
  estimate->set_value(9.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 0.98921400254611924);
  estimate->set_value(10.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), -0.69314718055994529);
  estimate->set_value(11.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 0.98921400254611924);
  estimate->set_value(12.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 4.0844100445772362);
}

/**
 *
 */
TEST_F(InternalEmptyModel, Students_T_WideDistribution_Mu2_Sigma3_Df2) {
  AddConfigurationLine(testresources::models::two_sex_no_estimates, "TestResources/Models/TwoSexNoEstimates.h", 33);
  AddConfigurationLine(estimate_student_t_case4, __FILE__, 72);
  LoadConfiguration();

  model_->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(2562.2095329203171, obj_function.score());

  Estimate* estimate = model_->managers()->estimate()->GetEstimate("selectivity[FishingSel].a50");
  if (!estimate)
    LOG_FATAL() << "!estimate";
  EXPECT_DOUBLE_EQ(estimate->value(), 8.981693098306657);

  // Check results
  estimate->set_value(1.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 1.1526795099383855);
  estimate->set_value(5.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 1.5040773967762742);
  estimate->set_value(10.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 2.6149597780361979);
  estimate->set_value(15.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 3.4393491476265314);
  estimate->set_value(20.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 4.0430512678345503);
}

/**
 *
 */
TEST_F(InternalEmptyModel, Students_T_HighDf_Mu15_Sigma1p5_Df20) {
  AddConfigurationLine(testresources::models::two_sex_no_estimates, "TestResources/Models/TwoSexNoEstimates.h", 33);
  AddConfigurationLine(estimate_student_t_case5, __FILE__, 86);
  LoadConfiguration();

  model_->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(2566.105571496546, obj_function.score());

  Estimate* estimate = model_->managers()->estimate()->GetEstimate("selectivity[FishingSel].a50");
  if (!estimate)
    LOG_FATAL() << "!estimate";
  EXPECT_DOUBLE_EQ(estimate->value(), 8.9880290130499052);

  // Check results
  estimate->set_value(10.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 4.8237926308985584);
  estimate->set_value(12.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 2.22868067604771);
  estimate->set_value(15.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 0.40546510810816438);
  estimate->set_value(18.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 2.22868067604771);
  estimate->set_value(20.0);
  EXPECT_DOUBLE_EQ(estimate->GetScore(), 4.8237926308985584);
}

} /* namespace estimates */
} /* namespace niwa */
#endif
