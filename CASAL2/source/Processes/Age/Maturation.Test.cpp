/**
 * @file Maturation.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/03/25
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "Model/Factory.h"
#include "Model/Managers.h"
#include "Model/Model.h"
#include "Partition/Partition.h"
#include "Selectivities/Manager.h"
#include "Selectivities/Selectivity.h"
#include "TestResources/TestFixtures/BasicModel.h"
#include "TimeSteps/Manager.h"

namespace niwa::processes::age {
using niwa::testfixtures::BasicModel;
using std::cout;
using std::endl;

/**
 *
 */
TEST_F(BasicModel, Processes_Maturation_Final_Year_Interpolation) {
  this->add_process_ageing(__FILE__, __LINE__, {"*"});
  this->add_process_recruitment_constant(__FILE__, __LINE__);
  this->add_time_step(__FILE__, __LINE__, {"ageing", "recruitment", "maturation"});

  // Maturation process
  base::Object* process = model_->factory().CreateObject(PARAM_PROCESS, PARAM_MATURATION);

  vector<string> from_categories = {"immature.male", "immature.female"};
  vector<string> to_categories   = {"mature.male", "mature.female"};
  vector<string> years           = {"1994"};   //, "1995", "1996", "1997", "1998", "1999", "2000"};
  vector<string> rate            = {"0.065"};  //, "0.065", "0.065", "0.065", "0.065", "0.065", "0.065"};
  process->parameters().Add(PARAM_LABEL, "maturation", __FILE__, __LINE__);
  process->parameters().Add(PARAM_TYPE, "maturation", __FILE__, __LINE__);
  process->parameters().Add(PARAM_FROM, from_categories, __FILE__, __LINE__);
  process->parameters().Add(PARAM_TO, to_categories, __FILE__, __LINE__);
  process->parameters().Add(PARAM_YEARS, years, __FILE__, __LINE__);
  process->parameters().Add(PARAM_RATES, rate, __FILE__, __LINE__);
  process->parameters().Add(PARAM_SELECTIVITIES, "constant_one", __FILE__, __LINE__);
  process->parameters().Add(PARAM_MISSING_YEARS_METHOD, PARAM_FINAL_YEAR, __FILE__, __LINE__);

  model_->Start(RunMode::kTesting);

  partition::Category& immature_male   = model_->partition().category("immature.male");
  partition::Category& immature_female = model_->partition().category("immature.female");
  partition::Category& mature_male     = model_->partition().category("mature.male");
  partition::Category& mature_female   = model_->partition().category("mature.female");

  // Verify blank partition
  for (unsigned i = 0; i < immature_male.data_.size(); ++i) EXPECT_DOUBLE_EQ(0.0, immature_male.data_[i]) << " where i = " << i << "; age = " << i + immature_male.min_age_;
  for (unsigned i = 0; i < immature_female.data_.size(); ++i) EXPECT_DOUBLE_EQ(0.0, immature_female.data_[i]) << " where i = " << i << "; age = " << i + immature_female.min_age_;
  for (unsigned i = 0; i < mature_male.data_.size(); ++i) EXPECT_DOUBLE_EQ(0.0, mature_male.data_[i]) << " where i = " << i << "; age = " << i + mature_male.min_age_;
  for (unsigned i = 0; i < mature_female.data_.size(); ++i) EXPECT_DOUBLE_EQ(0.0, mature_female.data_[i]) << " where i = " << i << "; age = " << i + mature_female.min_age_;

  // Run the model
  model_->FullIteration();

  // Validate our results
  double immature_male_value   = 60000;
  double mature_male_value     = 0;
  double immature_female_value = 40000;
  double mature_female_value   = 0;

  vector<double> expected_immature_male   = {56100, 52453.5, 49044, 45856.2, 42875.5, 40088.6, 37482.8, 35046.5, 32768.4, 30638.5, 28647, 26784.9, 25043.9, 23416.1, 21894};
  vector<double> expected_mature_male     = {3900, 7546.5, 10956, 14143.8, 17124.5, 19911.4, 22517.2, 24953.5, 27231.6, 29361.5, 31353, 33215.1, 34956.1, 36583.9, 38106};
  vector<double> expected_immature_female = {37400, 34969, 32696, 30570.8, 28583.7, 26725.7, 24988.6, 23364.3, 21845.6, 20425.7, 19098, 17856.6, 16695.9, 15610.7, 14596};
  vector<double> expected_mature_female   = {2600, 5031, 7303.99, 9429.23, 11416.3, 13274.3, 15011.4, 16635.7, 18154.4, 19574.3, 20902, 22143.4, 23304.1, 24389.3, 25404};

  for (unsigned i = 0; i < immature_male.data_.size() && i < 15; ++i) {
    double male_amount = immature_male_value * 0.065;
    immature_male_value -= male_amount;
    mature_male_value += male_amount;

    EXPECT_DOUBLE_EQ(immature_male_value, immature_male.data_[i]) << " where i = " << i << "; age = " << i + immature_male.min_age_;
    EXPECT_DOUBLE_EQ(mature_male_value, mature_male.data_[i]) << " where i = " << i << "; age = " << i + mature_male.min_age_;
    EXPECT_NEAR(expected_immature_male[i], immature_male.data_[i], 1e-1) << " where i = " << i << "; age = " << i + immature_male.min_age_;
    EXPECT_NEAR(expected_mature_male[i], mature_male.data_[i], 1e-1) << " where i = " << i << "; age = " << i + mature_male.min_age_;

    double female_amount = immature_female_value * 0.065;
    immature_female_value -= female_amount;
    mature_female_value += female_amount;
    EXPECT_DOUBLE_EQ(immature_female_value, immature_female.data_[i]) << " where i = " << i << "; age = " << i + immature_female.min_age_;
    EXPECT_DOUBLE_EQ(mature_female_value, mature_female.data_[i]) << " where i = " << i << "; age = " << i + mature_female.min_age_;
    EXPECT_NEAR(expected_immature_female[i], immature_female.data_[i], 1e-1) << " where i = " << i << "; age = " << i + immature_female.min_age_;
    EXPECT_NEAR(expected_mature_female[i], mature_female.data_[i], 1e-1) << " where i = " << i << "; age = " << i + mature_female.min_age_;
  }
}

}  // namespace niwa::processes::age
#endif  // TESTMODE