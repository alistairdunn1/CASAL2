/**
 * @file AddressableInputLoader.Test.cpp
 * @author GitHub Copilot
 * @date 2024/05/17
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * Unit tests for the AddressableInputLoader class
 */
#ifdef TESTMODE

// Headers
#include "../AddressableInputLoader/AddressableInputLoader.h"

#include <iostream>

#include "../GlobalConfiguration/GlobalConfiguration.h"
#include "../Logging/Logging.h"
#include "../Model/Model.h"
#include "../ObjectiveFunction/ObjectiveFunction.h"
#include "../TestResources/Models/TwoSex.h"
#include "../TestResources/TestFixtures/InternalEmptyModel.h"

// Namespaces
namespace niwa {

using niwa::testfixtures::InternalEmptyModel;
using std::cout;
using std::endl;

/**
 * Test fixture for AddressableInputLoader tests
 */
class AddressableInputLoaderTest : public InternalEmptyModel {
public:
  AddressableInputLoaderTest()          = default;
  virtual ~AddressableInputLoaderTest() = default;

  void LoadTwoSexModel() {
    AddConfigurationLine(testresources::models::two_sex, __FILE__, 35);
    LoadConfiguration();
  }
};

/**
 * Test the AddressableInputLoader with the TwoSex model
 * modifying the R0 parameter
 */
TEST_F(AddressableInputLoaderTest, TwoSex_R0_Override) {
  LoadTwoSexModel();

  model_->Start(RunMode::kTesting);

  // Get the AddressableInputLoader
  AddressableInputLoader* loader = model_->managers()->addressable_input_loader();

  // Add different values for R0
  loader->AddValue("process[Recruitment].R0", 500000.0);
  loader->AddValue("process[Recruitment].R0", 1000000.0);
  loader->AddValue("process[Recruitment].R0", 2000000.0);

  loader->AddValue("catchability[CPUEq].q", 0.000153139);
  loader->AddValue("catchability[CPUEq].q", 0.000153139);
  loader->AddValue("catchability[CPUEq].q", 0.000153139);

  loader->AddValue("selectivity[FishingSel].a50", 8);
  loader->AddValue("selectivity[FishingSel].a50", 8);
  loader->AddValue("selectivity[FishingSel].a50", 8);

  loader->AddValue("selectivity[FishingSel].ato95", 3);
  loader->AddValue("selectivity[FishingSel].ato95", 3);
  loader->AddValue("selectivity[FishingSel].ato95", 3);

  // Set force overwrites
  // model_->global_configuration().set_force_overwrite_of_addressables(true);

  // Run for the first value - R0 = 500,000
  loader->LoadValues(0);
  model_->FullIteration();
  ObjectiveFunction& obj_function1 = model_->objective_function();
  obj_function1.CalculateScore();
  Double score1 = obj_function1.score();

  // Run for the second value - R0 = 1,000,000
  loader->LoadValues(1);
  model_->FullIteration();
  ObjectiveFunction& obj_function2 = model_->objective_function();
  obj_function2.CalculateScore();
  Double score2 = obj_function2.score();

  // Run for the third value - R0 = 2,000,000
  loader->LoadValues(2);
  model_->FullIteration();
  ObjectiveFunction& obj_function3 = model_->objective_function();
  obj_function3.CalculateScore();
  Double score3 = obj_function3.score();

  // The scores should be different for different R0 values
  EXPECT_NE(score1, score2);
  EXPECT_NE(score2, score3);
  EXPECT_NE(score1, score3);

  // Scores should follow a pattern where higher R0 means lower objective function
  // because the model fits better with more fish
  EXPECT_NEAR(score1, 2497.02, 0.01);
  EXPECT_NEAR(score2, 2699.27, 0.01);
  EXPECT_NEAR(score3, 3078.34, 0.01);
}

/**
 * Test the AddressableInputLoader with the TwoSex model
 * modifying the M parameter (natural mortality)
 */
TEST_F(AddressableInputLoaderTest, TwoSex_a50_Override) {
  LoadTwoSexModel();

  model_->Start(RunMode::kTesting);

  // Get the AddressableInputLoader
  AddressableInputLoader* loader = model_->managers()->addressable_input_loader();

  // Add different values for a50
  loader->AddValue("process[Recruitment].R0", 997386);
  loader->AddValue("process[Recruitment].R0", 997386);
  loader->AddValue("process[Recruitment].R0", 997386);

  loader->AddValue("catchability[CPUEq].q", 0.000153139);
  loader->AddValue("catchability[CPUEq].q", 0.000153139);
  loader->AddValue("catchability[CPUEq].q", 0.000153139);

  loader->AddValue("selectivity[FishingSel].a50", 12);
  loader->AddValue("selectivity[FishingSel].a50", 8);
  loader->AddValue("selectivity[FishingSel].a50", 6);

  loader->AddValue("selectivity[FishingSel].ato95", 3);
  loader->AddValue("selectivity[FishingSel].ato95", 3);
  loader->AddValue("selectivity[FishingSel].ato95", 3);

  // Set force overwrites
  // model_->global_configuration().set_force_overwrite_of_addressables(true);

  // Run for the first value - a50 = 12
  loader->LoadValues(0);
  model_->FullIteration();
  ObjectiveFunction& obj_function1 = model_->objective_function();
  obj_function1.CalculateScore();
  Double score1 = obj_function1.score();

  // Run for the second value - a50 = 8
  loader->LoadValues(1);
  model_->FullIteration();
  ObjectiveFunction& obj_function2 = model_->objective_function();
  obj_function2.CalculateScore();
  Double score2 = obj_function2.score();

  // Run for the third value - a50 = 6
  loader->LoadValues(2);
  model_->FullIteration();
  ObjectiveFunction& obj_function3 = model_->objective_function();
  obj_function3.CalculateScore();
  Double score3 = obj_function3.score();

  // The scores should be different for different R0 values
  EXPECT_NE(score1, score2);
  EXPECT_NE(score2, score3);
  EXPECT_NE(score1, score3);

  // Scores should follow a pattern where higher R0 means lower objective function
  // because the model fits better with more fish
  EXPECT_NEAR(score1, 3882.06, 0.01);
  EXPECT_NEAR(score2, 2698.13, 0.01);
  EXPECT_NEAR(score3, 3602.68, 0.01);
}

/**
 * Test the AddressableInputLoader with the TwoSex model
 * modifying both R0 and M parameters simultaneously
 */
TEST_F(AddressableInputLoaderTest, TwoSex_R0_And_M_Override) {
  LoadTwoSexModel();

  model_->Start(RunMode::kTesting);

  // Get the AddressableInputLoader
  AddressableInputLoader* loader = model_->managers()->addressable_input_loader();

  // Add different values for a50
  loader->AddValue("process[Recruitment].R0", 500000.0);
  loader->AddValue("process[Recruitment].R0", 1000000.0);
  loader->AddValue("process[Recruitment].R0", 2000000.0);

  loader->AddValue("catchability[CPUEq].q", 0.000153139);
  loader->AddValue("catchability[CPUEq].q", 0.000153139);
  loader->AddValue("catchability[CPUEq].q", 0.000153139);

  loader->AddValue("selectivity[FishingSel].a50", 12);
  loader->AddValue("selectivity[FishingSel].a50", 8);
  loader->AddValue("selectivity[FishingSel].a50", 6);

  loader->AddValue("selectivity[FishingSel].ato95", 3);
  loader->AddValue("selectivity[FishingSel].ato95", 3);
  loader->AddValue("selectivity[FishingSel].ato95", 3);

  // Set force overwrites
  // model_->global_configuration().set_force_overwrite_of_addressables(true);

  // Run for the first value - a50 = 12
  loader->LoadValues(0);
  model_->FullIteration();
  ObjectiveFunction& obj_function1 = model_->objective_function();
  obj_function1.CalculateScore();
  Double score1 = obj_function1.score();

  // Run for the second value - a50 = 8
  loader->LoadValues(1);
  model_->FullIteration();
  ObjectiveFunction& obj_function2 = model_->objective_function();
  obj_function2.CalculateScore();
  Double score2 = obj_function2.score();

  // Run for the third value - a50 = 6
  loader->LoadValues(2);
  model_->FullIteration();
  ObjectiveFunction& obj_function3 = model_->objective_function();
  obj_function3.CalculateScore();
  Double score3 = obj_function3.score();

  // The scores should be different for different R0 values
  EXPECT_NE(score1, score2);
  EXPECT_NE(score2, score3);
  EXPECT_NE(score1, score3);

  // Scores should follow a pattern where higher R0 means lower objective function
  // because the model fits better with more fish
  EXPECT_NEAR(score1, 3648.91, 0.01);
  EXPECT_NEAR(score2, 2699.26, 0.01);
  EXPECT_NEAR(score3, 4024.07, 0.01);
}

/**
 * Test the AddressableInputLoader's ability to return addressable labels
 */
TEST_F(AddressableInputLoaderTest, GetAddressableLabels) {
  LoadTwoSexModel();

  // Get the AddressableInputLoader
  AddressableInputLoader* loader = model_->managers()->addressable_input_loader();

  // Add some addressable values
  loader->AddValue("process[Recruitment].R0", 500000.0);
  loader->AddValue("process[halfM].M{1}", 0.05);

  // Get the addressable labels
  vector<string> labels = loader->GetAddressableLabels();

  // Check that the right number of labels were returned
  EXPECT_EQ(labels.size(), 2);

  // Check that the expected labels are present
  bool found_r0 = false;
  bool found_m  = false;

  for (auto label : labels) {
    if (label == "process[Recruitment].R0")
      found_r0 = true;
    else if (label == "process[halfM].M{1}")
      found_m = true;
  }

  EXPECT_TRUE(found_r0);
  EXPECT_TRUE(found_m);
}

/**
 * Test the AddressableInputLoader's GetValueCount method
 */
TEST_F(AddressableInputLoaderTest, GetValueCount) {
  LoadTwoSexModel();

  // Get the AddressableInputLoader
  AddressableInputLoader* loader = model_->managers()->addressable_input_loader();

  // Initially there should be no values
  EXPECT_EQ(loader->GetValueCount(), 0);

  // Add one set of values
  loader->AddValue("process[Recruitment].R0", 500000.0);
  loader->AddValue("process[halfM].M{1}", 0.05);

  // Should now have one set
  EXPECT_EQ(loader->GetValueCount(), 1);

  // Add another set
  loader->AddValue("process[Recruitment].R0", 1000000.0);
  loader->AddValue("process[halfM].M{1}", 0.065);

  // Should now have two sets
  EXPECT_EQ(loader->GetValueCount(), 2);
}

/**
 * Test the AddressableInputLoader's GetValues method
 */
TEST_F(AddressableInputLoaderTest, GetValues) {
  LoadTwoSexModel();

  // Get the AddressableInputLoader
  AddressableInputLoader* loader = model_->managers()->addressable_input_loader();

  // Add some values
  loader->AddValue("process[Recruitment].R0", 500000.0);
  loader->AddValue("process[halfM].M{1}", 0.05);

  // Add another set
  loader->AddValue("process[Recruitment].R0", 1000000.0);
  loader->AddValue("process[halfM].M{1}", 0.065);

  // Get the first set of values
  map<string, Double> values1 = loader->GetValues(0);

  // Check the values
  EXPECT_EQ(values1["process[Recruitment].R0"], 500000.0);
  EXPECT_EQ(values1["process[halfM].M{1}"], 0.05);

  // Get the second set of values
  map<string, Double> values2 = loader->GetValues(1);

  // Check the values
  EXPECT_EQ(values2["process[Recruitment].R0"], 1000000.0);
  EXPECT_EQ(values2["process[halfM].M{1}"], 0.065);
}

} /* namespace niwa */

#endif /* TESTMODE */
