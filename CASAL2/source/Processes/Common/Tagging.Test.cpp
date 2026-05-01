/**
 * @file Tagging.Test.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 10/02/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * This file contains unit tests for the Tagging process (Age-based)
 */
#ifdef TESTMODE

// Headers
#include <iostream>

#include "Model/Model.h"
#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Partition/Partition.h"
#include "Processes/Manager.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"

// Namespaces
namespace niwa::processes::common {

using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

const std::string test_cases_process_tag_by_age =
    R"(
@model
start_year 1994
final_year 2008
min_age 1
max_age 12
age_plus t
base_weight_units kgs
initialisation_phases iphase1 iphase2
time_steps step_one=[processes=Recruitment] step_two=[processes=Tagging,Tagging_female,TagLoss] step_three=[processes=Ageing]

@categories
format stage.sex
names immature.male mature.male immature.female mature.female
age_lengths no_age_length*4

@age_length no_age_length
type none
length_weight no_length_weight

@length_weight no_length_weight
type none

@initialisation_phase iphase1
years 200

@initialisation_phase iphase2
years 1

@ageing Ageing
categories *

@Recruitment Recruitment
type constant
categories immature.male immature.female
proportions 0.5 0.5
R0 997386
age 1

@tag Tagging
type by_age
years 2008
from immature.male 
to mature.male
selectivities [type=constant; c=0.25] 
min_age 3
max_age 6
penalty [type=process]
table numbers
2008 1000 2000 3000 4000
end_table

@tag Tagging_female
type by_age
years 2008
from  immature.female
to  mature.female
selectivities [type=constant; c=0.4] 
min_age 3
max_age 6
penalty [type=process]
table numbers
2008 1000 2000 3000 4000
end_table

@process TagLoss
type tag_loss
categories mature.male mature.female
time_step_proportions 1.0
tag_loss_rate 0
selectivities loss_rate1=[type=constant; c=1] lossrate2=[type=constant; c=1]
tag_loss_type single
year 2008

@report DQ
type derived_quantity
)";

/**
 *
 */
TEST_F(InternalEmptyModel, Processes_Tag_By_Age) {
  AddConfigurationLine(test_cases_process_tag_by_age, __FILE__, 36);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  // Expected values: numbers * selectivity
  // Male: 1000*0.25=250, 2000*0.25=500, 3000*0.25=750, 4000*0.25=1000
  partition::Category& male = model_->partition().category("mature.male");
  EXPECT_DOUBLE_EQ(0.0, male.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[2]);
  EXPECT_DOUBLE_EQ(250.0, male.data_[3]);
  EXPECT_DOUBLE_EQ(500.0, male.data_[4]);
  EXPECT_DOUBLE_EQ(750.0, male.data_[5]);
  EXPECT_DOUBLE_EQ(1000.0, male.data_[6]);
  EXPECT_DOUBLE_EQ(0.0, male.data_[7]);

  // Female: 1000*0.4=400, 2000*0.4=800, 3000*0.4=1200, 4000*0.4=1600
  partition::Category& female = model_->partition().category("mature.female");
  EXPECT_DOUBLE_EQ(0.0, female.data_[0]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[1]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[2]);
  EXPECT_DOUBLE_EQ(400.0, female.data_[3]);
  EXPECT_DOUBLE_EQ(800.0, female.data_[4]);
  EXPECT_DOUBLE_EQ(1200.0, female.data_[5]);
  EXPECT_DOUBLE_EQ(1600.0, female.data_[6]);
  EXPECT_DOUBLE_EQ(0.0, female.data_[7]);
}

} /* namespace niwa::processes::common */
#endif /* TESTMODE */
