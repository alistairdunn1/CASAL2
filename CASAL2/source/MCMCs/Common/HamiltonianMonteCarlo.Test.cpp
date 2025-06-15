/**
 * @file HamiltonianMonteCarlo.Test.cpp
 * @author Scott Rasmussen (scott@zaita.com)
 * @brief
 * @version 0.1
 * @date 2021-05-10
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifdef TESTMODE
#ifndef USE_AUTODIFF

// Headers
#include <iomanip>
#include <iostream>

#include "../../DerivedQuantities/Manager.h"
#include "../../MCMCs/MCMC.h"
#include "../../MCMCs/Manager.h"
#include "../../MPD/MPD.Mock.h"
#include "../../Model/Managers.h"
#include "../../Model/Model.h"
#include "../../ObjectiveFunction/ObjectiveFunction.h"
#include "../../TestResources/MockClasses/Model.h"
#include "../../TestResources/TestCases/CasalComplex1.h"
#include "../../TestResources/TestCases/TwoSexModel.h"
#include "../../TestResources/TestFixtures/BaseThreaded.h"
#include "../../TestResources/TestFixtures/BasicModel.h"
#include "../MCMC.Mock.h"
#include "../MCMC.h"
#include "../Manager.h"

// Namespaces
namespace niwa {

using niwa::MockMPD;
using std::cout;
using std::endl;
using ::testing::NiceMock;

class HamiltonianMonteCarloThreadedModel : public testfixtures::BaseThreaded {};

/**
 * Notes for these unit tests.
 *
 * Because we're using an arctan transformation in the deltadiff minimiser we end up with quite
 * different values between Operating Systems. Each of these unit tests will #ifdef the Operating
 * System so we can check the values between them.
 *
 * Arguably, all answers are correct. The difference starts off <1e-16 but when we use this to inform
 * the gradient it does change the direction of the leap frog slightly. After some jumps we end up
 * in a place that will fail a unit test, even though it's arguably correct.
 *
 * Second note:
 *  The code I use to print the results
 */
//  {
//   cout << std::setprecision(16);
//   for (unsigned i = 0; i < chain.size(); i += 10) cout << "EXPECT_DOUBLE_EQ(chain[" << i << "].score_, " << chain[i].score_ << ");" << endl;
// }

/**
 * @brief Test the HMC MCMC algorithm with the TwoSex model doing Five Iterations
 *
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Five_Iteration_With_TwoSex) {
  string ammended_definition = testcases::test_cases_two_sex_model_population;
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  boost::replace_all(ammended_definition, "threads 1", "threads 4");
  AddConfigurationLine(ammended_definition, __FILE__, __LINE__);

  string mcmc_definition = R"(
    @mcmc my_mcmc
    type hamiltonian
    length 5
    start 0
    step_size 0.02
    keep 1
    leapfrog_delta 1e-5
    leapfrog_steps 3
  )";
  AddConfigurationLine(mcmc_definition, __FILE__, __LINE__);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kMCMC));
  auto model = runner_->model();
  auto mcmc  = model->managers()->mcmc()->active_mcmc();
  ASSERT_TRUE(mcmc != nullptr);

  auto chain = mcmc->chain();
  ASSERT_EQ(5u, chain.size());

#ifdef __GITHUB_ACTIONS_WINDOWS
  EXPECT_DOUBLE_EQ(chain[0].score_, 1978.1596979263302);
  EXPECT_DOUBLE_EQ(chain[1].score_, 1978.1594543486492);
  EXPECT_DOUBLE_EQ(chain[2].score_, 1978.1591024197189);
  EXPECT_DOUBLE_EQ(chain[3].score_, 1978.158885029025);
  EXPECT_DOUBLE_EQ(chain[4].score_, 1978.1585957112677);
#elif _WIN64
  EXPECT_DOUBLE_EQ(chain[0].score_, 1979.302049961175);
  EXPECT_DOUBLE_EQ(chain[1].score_, 1979.3017644429353);
  EXPECT_DOUBLE_EQ(chain[2].score_, 1979.301741240226);
  EXPECT_DOUBLE_EQ(chain[3].score_, 1979.3013843614176);
  EXPECT_DOUBLE_EQ(chain[4].score_, 1979.3011473702381);
#elif __linux__
  EXPECT_DOUBLE_EQ(chain[0].score_, 1979.31910941548);
  EXPECT_DOUBLE_EQ(chain[1].score_, 1979.318837290028);
  EXPECT_DOUBLE_EQ(chain[2].score_, 1979.318332128152);
  EXPECT_DOUBLE_EQ(chain[3].score_, 1979.318110301643);
  EXPECT_DOUBLE_EQ(chain[4].score_, 1979.317733207503);
#endif
}

/**
 * @brief Construct a new test f object
 *
 */
TEST_F(HamiltonianMonteCarloThreadedModel, TwentyFive_Iteration_With_TwoSex) {
  string ammended_definition = testcases::test_cases_two_sex_model_population;
  boost::replace_all(ammended_definition, "threads 1", "threads 2");
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  AddConfigurationLine(ammended_definition, __FILE__, __LINE__);

  string mcmc_definition = R"(
    @mcmc my_mcmc
    type hamiltonian
    length 25
    start 0
    step_size 0.02
    keep 1
    leapfrog_delta 1e-7
    leapfrog_steps 3
  )";
  AddConfigurationLine(mcmc_definition, __FILE__, __LINE__);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kMCMC));
  auto model = runner_->model();
  auto mcmc  = model->managers()->mcmc()->active_mcmc();
  ASSERT_TRUE(mcmc != nullptr);

  auto chain = mcmc->chain();
  ASSERT_EQ(25u, chain.size());

#ifdef __GITHUB_ACTIONS_WINDOWS
  EXPECT_DOUBLE_EQ(chain[0].score_, 1978.1596979263302);
  EXPECT_DOUBLE_EQ(chain[1].score_, 1978.1594543486492);
  EXPECT_DOUBLE_EQ(chain[2].score_, 1978.1591024197189);
  EXPECT_DOUBLE_EQ(chain[3].score_, 1978.158885029025);
  EXPECT_DOUBLE_EQ(chain[4].score_, 1978.1585957112677);
  EXPECT_DOUBLE_EQ(chain[5].score_, 1978.1596846974744);
  EXPECT_DOUBLE_EQ(chain[6].score_, 1978.159681979555);
  EXPECT_DOUBLE_EQ(chain[7].score_, 1978.1596860517557);
  EXPECT_DOUBLE_EQ(chain[8].score_, 1978.1596836710382);
  EXPECT_DOUBLE_EQ(chain[9].score_, 1978.1596801530534);
  EXPECT_DOUBLE_EQ(chain[10].score_, 1978.1596780478726);
  EXPECT_DOUBLE_EQ(chain[11].score_, 1978.1596750398071);
  EXPECT_DOUBLE_EQ(chain[12].score_, 1978.1596735360283);
  EXPECT_DOUBLE_EQ(chain[13].score_, 1978.1596708806944);
  EXPECT_DOUBLE_EQ(chain[14].score_, 1978.1596532774574);
  EXPECT_DOUBLE_EQ(chain[15].score_, 1978.1596508221869);
  EXPECT_DOUBLE_EQ(chain[16].score_, 1978.1596468698986);
  EXPECT_DOUBLE_EQ(chain[17].score_, 1978.1596445580994);
  EXPECT_DOUBLE_EQ(chain[18].score_, 1978.1596413674554);
  EXPECT_DOUBLE_EQ(chain[19].score_, 1978.1596394417122);
  EXPECT_DOUBLE_EQ(chain[20].score_, 1978.1596366501219);
  EXPECT_DOUBLE_EQ(chain[21].score_, 1978.1596362696796);
  EXPECT_DOUBLE_EQ(chain[22].score_, 1978.1596338815984);
  EXPECT_DOUBLE_EQ(chain[23].score_, 1978.1596302247012);
  EXPECT_DOUBLE_EQ(chain[24].score_, 1978.159628060758);
#elif _WIN64
  EXPECT_DOUBLE_EQ(chain[0].score_, 1979.3020499611750438);
  EXPECT_DOUBLE_EQ(chain[1].score_, 1979.3020470915430451);
  EXPECT_DOUBLE_EQ(chain[2].score_, 1979.3020466862337798);
  EXPECT_DOUBLE_EQ(chain[3].score_, 1979.302043067030354);
  EXPECT_DOUBLE_EQ(chain[4].score_, 1979.3020404979447449);
  EXPECT_DOUBLE_EQ(chain[5].score_, 1979.3020351838388251);
  EXPECT_DOUBLE_EQ(chain[6].score_, 1979.3020313736797107);
  EXPECT_DOUBLE_EQ(chain[7].score_, 1979.3020055990491528);
  EXPECT_DOUBLE_EQ(chain[8].score_, 1979.3020004714137485);
  EXPECT_DOUBLE_EQ(chain[9].score_, 1979.3020006346400805);
  EXPECT_DOUBLE_EQ(chain[10].score_, 1979.3019944718671468);
  EXPECT_DOUBLE_EQ(chain[11].score_, 1979.3019930402597311);
  EXPECT_DOUBLE_EQ(chain[12].score_, 1979.3019832154277537);
  EXPECT_DOUBLE_EQ(chain[13].score_, 1979.3019805764106422);
  EXPECT_DOUBLE_EQ(chain[14].score_, 1979.3019825897238206);
  EXPECT_DOUBLE_EQ(chain[15].score_, 1979.3019794722013103);
  EXPECT_DOUBLE_EQ(chain[16].score_, 1979.3019780008462476);
  EXPECT_DOUBLE_EQ(chain[17].score_, 1979.3019738738180422);
  EXPECT_DOUBLE_EQ(chain[18].score_, 1979.3019707086896233);
  EXPECT_DOUBLE_EQ(chain[19].score_, 1979.3019640178647478);
  EXPECT_DOUBLE_EQ(chain[20].score_, 1979.3019596693270614);
  EXPECT_DOUBLE_EQ(chain[21].score_, 1979.3019707050871148);
  EXPECT_DOUBLE_EQ(chain[22].score_, 1979.3019688745027906);
  EXPECT_DOUBLE_EQ(chain[23].score_, 1979.3019645214221782);
  EXPECT_DOUBLE_EQ(chain[24].score_, 1979.301961154108767);
#elif __linux__
  EXPECT_DOUBLE_EQ(chain[0].score_, 1979.31910941548);
  EXPECT_DOUBLE_EQ(chain[1].score_, 1979.31910667998);
  EXPECT_DOUBLE_EQ(chain[2].score_, 1979.319101457041);
  EXPECT_DOUBLE_EQ(chain[3].score_, 1979.319099189367);
  EXPECT_DOUBLE_EQ(chain[4].score_, 1979.319095221592);
  EXPECT_DOUBLE_EQ(chain[5].score_, 1979.319093918872);
  EXPECT_DOUBLE_EQ(chain[6].score_, 1979.319090717183);
  EXPECT_DOUBLE_EQ(chain[7].score_, 1979.319102191839);
  EXPECT_DOUBLE_EQ(chain[8].score_, 1979.319099816649);
  EXPECT_DOUBLE_EQ(chain[9].score_, 1979.319095325929);
  EXPECT_DOUBLE_EQ(chain[10].score_, 1979.31909364716);
  EXPECT_DOUBLE_EQ(chain[11].score_, 1979.319090029647);
  EXPECT_DOUBLE_EQ(chain[12].score_, 1979.319090616442);
  EXPECT_DOUBLE_EQ(chain[13].score_, 1979.319087741402);
  EXPECT_DOUBLE_EQ(chain[14].score_, 1979.319081412827);
  EXPECT_DOUBLE_EQ(chain[15].score_, 1979.319078851945);
  EXPECT_DOUBLE_EQ(chain[16].score_, 1979.319074174579);
  EXPECT_DOUBLE_EQ(chain[17].score_, 1979.319072148768);
  EXPECT_DOUBLE_EQ(chain[18].score_, 1979.319068518904);
  EXPECT_DOUBLE_EQ(chain[19].score_, 1979.319068194450);
  EXPECT_DOUBLE_EQ(chain[20].score_, 1979.319065299380);
  EXPECT_DOUBLE_EQ(chain[21].score_, 1979.319054701787);
  EXPECT_DOUBLE_EQ(chain[22].score_, 1979.319051302200);
  EXPECT_DOUBLE_EQ(chain[23].score_, 1979.319052394348);
  EXPECT_DOUBLE_EQ(chain[24].score_, 1979.319049774818);
#endif
}

/**
 * @brief Construct a new test f object
 *
 */
TEST_F(HamiltonianMonteCarloThreadedModel, OneHundred_Iteration_With_TwoSex) {
  string ammended_definition = testcases::test_cases_two_sex_model_population;
  boost::replace_all(ammended_definition, "threads 1", "threads 4");
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  AddConfigurationLine(ammended_definition, __FILE__, __LINE__);

  string mcmc_definition = R"(
    @mcmc my_mcmc
    type hamiltonian
    length 100
    start 0
    step_size 0.02
    keep 1
    leapfrog_delta 1e-7
    leapfrog_steps 3
  )";
  AddConfigurationLine(mcmc_definition, __FILE__, __LINE__);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kMCMC));
  auto model = runner_->model();
  auto mcmc  = model->managers()->mcmc()->active_mcmc();
  ASSERT_TRUE(mcmc != nullptr);

  auto chain = mcmc->chain();
  ASSERT_EQ(100u, chain.size());

#ifdef __GITHUB_ACTIONS_WINDOWS
  EXPECT_DOUBLE_EQ(chain[0].score_, 1978.1596979263302);
  EXPECT_DOUBLE_EQ(chain[5].score_, 1978.1596846974744);
  EXPECT_DOUBLE_EQ(chain[10].score_, 1978.1596780478726);
  EXPECT_DOUBLE_EQ(chain[15].score_, 1978.1596508221869);
  EXPECT_DOUBLE_EQ(chain[20].score_, 1978.1596366501219);
  EXPECT_DOUBLE_EQ(chain[25].score_, 1978.1596251309056);
  EXPECT_DOUBLE_EQ(chain[30].score_, 1978.1596132203067);
  EXPECT_DOUBLE_EQ(chain[35].score_, 1978.1595973230317);
  EXPECT_DOUBLE_EQ(chain[40].score_, 1978.1595861670035);
  EXPECT_DOUBLE_EQ(chain[45].score_, 1978.1595893486199);
  EXPECT_DOUBLE_EQ(chain[50].score_, 1978.1595718393637);
  EXPECT_DOUBLE_EQ(chain[55].score_, 1978.1595585429895);
  EXPECT_DOUBLE_EQ(chain[60].score_, 1978.1595565815057);
  EXPECT_DOUBLE_EQ(chain[65].score_, 1978.1595430804334);
  EXPECT_DOUBLE_EQ(chain[70].score_, 1978.1593216923202);
  EXPECT_DOUBLE_EQ(chain[75].score_, 1978.1593072362828);
  EXPECT_DOUBLE_EQ(chain[80].score_, 1978.1592986732212);
  EXPECT_DOUBLE_EQ(chain[85].score_, 1978.1592945103787);
  EXPECT_DOUBLE_EQ(chain[90].score_, 1978.1592828048135);
  EXPECT_DOUBLE_EQ(chain[95].score_, 1978.1592670578009);
#elif _WIN64
  EXPECT_DOUBLE_EQ(chain[0].score_, 1979.3020499611750438);
  EXPECT_DOUBLE_EQ(chain[5].score_, 1979.3020351838388251);
  EXPECT_DOUBLE_EQ(chain[10].score_, 1979.3019944718671468);
  EXPECT_DOUBLE_EQ(chain[15].score_, 1979.3019794722013103);
  EXPECT_DOUBLE_EQ(chain[20].score_, 1979.3019596693270614);
  EXPECT_DOUBLE_EQ(chain[25].score_, 1979.3019537755765214);
  EXPECT_DOUBLE_EQ(chain[30].score_, 1979.3019440268008111);
  EXPECT_DOUBLE_EQ(chain[35].score_, 1979.301900354884765);
  EXPECT_DOUBLE_EQ(chain[40].score_, 1979.3018696129702221);
  EXPECT_DOUBLE_EQ(chain[45].score_, 1979.3018447067643137);
  EXPECT_DOUBLE_EQ(chain[50].score_, 1979.3018210052966879);
  EXPECT_DOUBLE_EQ(chain[55].score_, 1979.3018254737555708);
  EXPECT_DOUBLE_EQ(chain[60].score_, 1979.3018293503657787);
  EXPECT_DOUBLE_EQ(chain[65].score_, 1979.3018150599191358);
  EXPECT_DOUBLE_EQ(chain[70].score_, 1979.3017831024715179);
  EXPECT_DOUBLE_EQ(chain[75].score_, 1979.3017541797662489);
  EXPECT_DOUBLE_EQ(chain[80].score_, 1979.3017360959611324);
  EXPECT_DOUBLE_EQ(chain[85].score_, 1979.3017082443243453);
  EXPECT_DOUBLE_EQ(chain[90].score_, 1979.3017402898069577);
  EXPECT_DOUBLE_EQ(chain[95].score_, 1979.3017260680524032);
#elif __linux__
  EXPECT_DOUBLE_EQ(chain[0].score_, 1979.31910941548);
  EXPECT_DOUBLE_EQ(chain[5].score_, 1979.319093918872);
  EXPECT_DOUBLE_EQ(chain[10].score_, 1979.31909364716);
  EXPECT_DOUBLE_EQ(chain[15].score_, 1979.319078851945);
  EXPECT_DOUBLE_EQ(chain[20].score_, 1979.319065299380);
  EXPECT_DOUBLE_EQ(chain[25].score_, 1979.319044240832);
  EXPECT_DOUBLE_EQ(chain[30].score_, 1979.319036663623);
  EXPECT_DOUBLE_EQ(chain[35].score_, 1979.319034464054);
  EXPECT_DOUBLE_EQ(chain[40].score_, 1979.318998174726);
  EXPECT_DOUBLE_EQ(chain[45].score_, 1979.318943927063);
  EXPECT_DOUBLE_EQ(chain[50].score_, 1979.318915747170);
  EXPECT_DOUBLE_EQ(chain[55].score_, 1979.318904312885);
  EXPECT_DOUBLE_EQ(chain[60].score_, 1979.318878527860);
  EXPECT_DOUBLE_EQ(chain[65].score_, 1979.318864735373);
  EXPECT_DOUBLE_EQ(chain[70].score_, 1979.318853724756);
  EXPECT_DOUBLE_EQ(chain[75].score_, 1979.318835980401);
  EXPECT_DOUBLE_EQ(chain[80].score_, 1979.318821098520);
  EXPECT_DOUBLE_EQ(chain[85].score_, 1979.318808220300);
  EXPECT_DOUBLE_EQ(chain[90].score_, 1979.318785249656);
  EXPECT_DOUBLE_EQ(chain[95].score_, 1979.318774356487);
#endif
}

/**
 * @brief Construct a new test f object
 *
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Five_Iteration_With_CasalComplexOne) {
  string ammended_definition = testcases::test_cases_casal_complex_1;
  boost::replace_all(ammended_definition, "threads 1", "threads 8");
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  AddConfigurationLine(ammended_definition, __FILE__, __LINE__);

  string mcmc_definition = R"(
    @mcmc my_mcmc
    type hamiltonian
    length 5
    start 0
    step_size 0.02
    keep 1
    leapfrog_delta 1e-5
    leapfrog_steps 3
  )";
  AddConfigurationLine(mcmc_definition, __FILE__, __LINE__);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kMCMC));
  auto model = runner_->model();
  auto mcmc  = model->managers()->mcmc()->active_mcmc();
  ASSERT_TRUE(mcmc != nullptr);

  auto chain = mcmc->chain();
  ASSERT_EQ(5u, chain.size());

#ifdef __GITHUB_ACTIONS_WINDOWS
  EXPECT_DOUBLE_EQ(chain[0].score_, 487.52063897234791);
  EXPECT_DOUBLE_EQ(chain[1].score_, 487.53527535556577);
  EXPECT_DOUBLE_EQ(chain[2].score_, 487.58671049861869);
  EXPECT_DOUBLE_EQ(chain[3].score_, 487.58822511748838);
  EXPECT_DOUBLE_EQ(chain[4].score_, 487.58822511748838);
#elif _WIN64
  EXPECT_DOUBLE_EQ(chain[0].score_, 487.52063897213412247);
  EXPECT_DOUBLE_EQ(chain[1].score_, 487.53527528011829872);
  EXPECT_DOUBLE_EQ(chain[2].score_, 487.67106078210798614);
  EXPECT_DOUBLE_EQ(chain[3].score_, 487.67273570961077667);
  EXPECT_DOUBLE_EQ(chain[4].score_, 487.67273570961077667);
#elif __linux__
  // EXPECT_NEAR(chain[0].score_, 487.520638972034, 1e-7);
  // EXPECT_NEAR(chain[1].score_, 487.5352752521567, 1e-7);
  // EXPECT_NEAR(chain[2].score_, 487.6219696273498, 1e-7);
  // EXPECT_NEAR(chain[3].score_, 487.6233820105855, 1e-7);
  // EXPECT_NEAR(chain[4].score_, 487.6233820105855, 1e-7);
#endif
}

/**
 * @brief Construct a new test f object
 *
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Five_Iteration_With_CasalComplexOne_LeapFrog_Steps_Ten) {
  string ammended_definition = testcases::test_cases_casal_complex_1;
  boost::replace_all(ammended_definition, "threads 1", "threads 4");
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  AddConfigurationLine(ammended_definition, __FILE__, __LINE__);

  string mcmc_definition = R"(
    @mcmc my_mcmc
    type hamiltonian
    length 5
    start 0
    step_size 0.02
    keep 1
    leapfrog_delta 1e-5
    leapfrog_steps 10
  )";
  AddConfigurationLine(mcmc_definition, __FILE__, __LINE__);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kMCMC));
  auto model = runner_->model();
  auto mcmc  = model->managers()->mcmc()->active_mcmc();
  ASSERT_TRUE(mcmc != nullptr);

  auto chain = mcmc->chain();
  ASSERT_EQ(5u, chain.size());

#ifdef __GITHUB_ACTIONS_WINDOWS
  EXPECT_DOUBLE_EQ(chain[0].score_, 487.52063897234791);
  EXPECT_DOUBLE_EQ(chain[1].score_, 487.53965019178696);
  EXPECT_DOUBLE_EQ(chain[2].score_, 487.59638031932241);
  EXPECT_DOUBLE_EQ(chain[3].score_, 487.60283826625414);
  EXPECT_DOUBLE_EQ(chain[4].score_, 487.60283826625414);
#elif _WIN64
  EXPECT_DOUBLE_EQ(chain[0].score_, 487.52063897213412247);
  EXPECT_DOUBLE_EQ(chain[1].score_, 487.53964985454666703);
  EXPECT_DOUBLE_EQ(chain[2].score_, 487.68122585374317168);
  EXPECT_DOUBLE_EQ(chain[3].score_, 487.68810277217346538);
  EXPECT_DOUBLE_EQ(chain[4].score_, 487.68810277217346538);
#elif __linux__
  // EXPECT_NEAR(chain[0].score_, 487.520638972034, 1e-5);
  // EXPECT_NEAR(chain[1].score_, 487.5396497473042, 1e-5);
  // EXPECT_NEAR(chain[2].score_, 487.6318753298668, 1e-5);
  // EXPECT_NEAR(chain[3].score_, 487.6383538202512, 1e-5);
  // EXPECT_NEAR(chain[4].score_, 487.6383538202512, 1e-5);
#endif
}

/**
 * @brief Construct a new test f object
 *
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Five_Iteration_With_CasalComplexOne_LeapFrog_Delta_OneESeven) {
  string ammended_definition = testcases::test_cases_casal_complex_1;
  boost::replace_all(ammended_definition, "threads 1", "threads 4");
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  AddConfigurationLine(ammended_definition, __FILE__, __LINE__);

  string mcmc_definition = R"(
    @mcmc my_mcmc
    type hamiltonian
    length 5
    start 0
    step_size 0.02
    keep 1
    leapfrog_delta 1e-7
    leapfrog_steps 10
  )";
  AddConfigurationLine(mcmc_definition, __FILE__, __LINE__);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kMCMC));
  auto model = runner_->model();
  auto mcmc  = model->managers()->mcmc()->active_mcmc();
  ASSERT_TRUE(mcmc != nullptr);

  auto chain = mcmc->chain();
  ASSERT_EQ(5u, chain.size());

#ifdef __GITHUB_ACTIONS_WINDOWS
  EXPECT_DOUBLE_EQ(chain[0].score_, 487.52063897234791);
  EXPECT_DOUBLE_EQ(chain[1].score_, 487.52082862038958);
  EXPECT_DOUBLE_EQ(chain[2].score_, 487.52088943758434);
  EXPECT_DOUBLE_EQ(chain[3].score_, 487.5209452134136);
  EXPECT_DOUBLE_EQ(chain[4].score_, 487.52100271762271);
#elif _WIN64
  EXPECT_DOUBLE_EQ(chain[0].score_, 487.52063897213412247);
  EXPECT_DOUBLE_EQ(chain[1].score_, 487.52082861950833603);
  EXPECT_DOUBLE_EQ(chain[2].score_, 487.52089792652054712);
  EXPECT_DOUBLE_EQ(chain[3].score_, 487.52095374163752695);
  EXPECT_DOUBLE_EQ(chain[4].score_, 487.52100918145538344);
#elif __linux__
  EXPECT_NEAR(chain[0].score_, 487.520638972034, 1e-5);
  EXPECT_NEAR(chain[1].score_, 487.5208286193689, 1e-5);
  EXPECT_NEAR(chain[2].score_, 487.5208929731917, 1e-5);
  EXPECT_NEAR(chain[3].score_, 487.5209487497292, 1e-5);
  EXPECT_NEAR(chain[4].score_, 487.5210056161228, 1e-5);
#endif
}

/**
 * @brief Construct a new test f object
 *
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Five_Iteration_With_CasalComplexOne_LeapFrog_Delta_OneEThree_RandomStart) {
  string ammended_definition = testcases::test_cases_casal_complex_1;
  boost::replace_all(ammended_definition, "threads 1", "threads 4");
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  AddConfigurationLine(ammended_definition, __FILE__, __LINE__);

  string mcmc_definition = R"(
    @mcmc my_mcmc
    type hamiltonian
    length 5
    start 1
    step_size 0.02
    keep 1
    leapfrog_delta 1e-3
    leapfrog_steps 10
  )";
  AddConfigurationLine(mcmc_definition, __FILE__, __LINE__);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kMCMC));
  auto model = runner_->model();
  auto mcmc  = model->managers()->mcmc()->active_mcmc();
  ASSERT_TRUE(mcmc != nullptr);

  auto chain = mcmc->chain();
  ASSERT_EQ(5u, chain.size());

#ifdef __GITHUB_ACTIONS_WINDOWS
  EXPECT_DOUBLE_EQ(chain[0].score_, 12343.088960065077);
  EXPECT_DOUBLE_EQ(chain[1].score_, 12343.088960065077);
  EXPECT_DOUBLE_EQ(chain[2].score_, 12343.088960065077);
  EXPECT_DOUBLE_EQ(chain[3].score_, 12343.088960065077);
  EXPECT_DOUBLE_EQ(chain[4].score_, 12343.088960065077);
#elif _WIN64
  EXPECT_DOUBLE_EQ(chain[0].score_, 11565.956258254713248);
  EXPECT_DOUBLE_EQ(chain[1].score_, 11565.956258254713248);
  EXPECT_DOUBLE_EQ(chain[2].score_, 11565.956258254713248);
  EXPECT_DOUBLE_EQ(chain[3].score_, 11539.212775746313127);
  EXPECT_DOUBLE_EQ(chain[4].score_, 11539.212775746313127);
#elif __linux__
  // EXPECT_NEAR(chain[0].score_, 11622.04009177895, 1e-5);
  // EXPECT_NEAR(chain[1].score_, 11622.04009177895, 1e-5);
  // EXPECT_NEAR(chain[2].score_, 11622.04009177895, 1e-5);
  // EXPECT_NEAR(chain[3].score_, 11622.04009177895, 1e-5);
  // EXPECT_NEAR(chain[4].score_, 11622.04009177895, 1e-5);
#endif
}

/**
 * @brief Construct a new test f object
 *
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Five_Iteration_With_TwoSex_CustomGradientStepSize) {
  string ammended_definition = testcases::test_cases_two_sex_model_population;
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  AddConfigurationLine(ammended_definition, __FILE__, __LINE__);

  string mcmc_definition = R"(
    @mcmc my_mcmc
    type hamiltonian
    length 5
    start 0
    step_size 0.02
    keep 1
    leapfrog_delta 1e-5
    leapfrog_steps 3
    gradient_step_size 1e-9
  )";
  AddConfigurationLine(mcmc_definition, __FILE__, __LINE__);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kMCMC));
  auto model = runner_->model();
  auto mcmc  = model->managers()->mcmc()->active_mcmc();
  ASSERT_TRUE(mcmc != nullptr);

  auto chain = mcmc->chain();
  ASSERT_EQ(5u, chain.size());

#ifdef __GITHUB_ACTIONS_WINDOWS
  EXPECT_DOUBLE_EQ(chain[0].score_, 1978.1596979263302);
  EXPECT_DOUBLE_EQ(chain[1].score_, 1978.1594543486492);
  EXPECT_DOUBLE_EQ(chain[2].score_, 1978.1591024197194);
  EXPECT_DOUBLE_EQ(chain[3].score_, 1978.1588850290236);
  EXPECT_DOUBLE_EQ(chain[4].score_, 1978.1585957112704);
#elif _WIN64
  EXPECT_DOUBLE_EQ(chain[0].score_, 1979.3020499611750438);
  EXPECT_DOUBLE_EQ(chain[1].score_, 1979.3017644429294251);
  EXPECT_DOUBLE_EQ(chain[2].score_, 1979.3017412402209629);
  EXPECT_DOUBLE_EQ(chain[3].score_, 1979.3013843614132838);
  EXPECT_DOUBLE_EQ(chain[4].score_, 1979.3011473702440526);
#elif __linux__
  EXPECT_NEAR(chain[0].score_, 1979.31910941548, 1e-5);
  EXPECT_NEAR(chain[1].score_, 1979.318837290026, 1e-5);
  EXPECT_NEAR(chain[2].score_, 1979.318332128151, 1e-5);
  EXPECT_NEAR(chain[3].score_, 1979.318110301642, 1e-5);
  EXPECT_NEAR(chain[4].score_, 1979.317733207502, 1e-5);
#endif
}

/**
 * @brief Construct a new test f object
 *
 */
TEST_F(HamiltonianMonteCarloThreadedModel, TwentyFive_Iteration_With_TwoSex_RandomStart_OneEFourStepSize) {
  string ammended_definition = testcases::test_cases_two_sex_model_population;
  boost::replace_all(ammended_definition, "threads 1", "threads 4");
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  AddConfigurationLine(ammended_definition, __FILE__, 76);

  string mcmc_definition = R"(
    @mcmc my_mcmc
    type hamiltonian
    length 25
    start 1
    step_size 0.02
    keep 1
    leapfrog_delta 1e-4
    leapfrog_steps 5
  )";
  AddConfigurationLine(mcmc_definition, __FILE__, 70);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kMCMC));
  auto model = runner_->model();
  auto mcmc  = model->managers()->mcmc()->active_mcmc();
  ASSERT_TRUE(mcmc != nullptr);

  auto chain = mcmc->chain();
  ASSERT_EQ(25u, chain.size());

#ifdef __GITHUB_ACTIONS_WINDOWS
  EXPECT_DOUBLE_EQ(chain[0].score_, 31353.946762517651);
  EXPECT_DOUBLE_EQ(chain[1].score_, 31353.127544135874);
  EXPECT_DOUBLE_EQ(chain[2].score_, 31350.646398193501);
  EXPECT_DOUBLE_EQ(chain[3].score_, 31350.787226282864);
  EXPECT_DOUBLE_EQ(chain[4].score_, 31349.152647978866);
  EXPECT_DOUBLE_EQ(chain[5].score_, 31349.52959076811);
  EXPECT_DOUBLE_EQ(chain[6].score_, 31348.313181183752);
  EXPECT_DOUBLE_EQ(chain[7].score_, 31348.313181183752);
  EXPECT_DOUBLE_EQ(chain[8].score_, 31347.124025770627);
  EXPECT_DOUBLE_EQ(chain[9].score_, 31347.124025770627);
  EXPECT_DOUBLE_EQ(chain[10].score_, 31346.208285879755);
  EXPECT_DOUBLE_EQ(chain[11].score_, 31339.460200059671);
  EXPECT_DOUBLE_EQ(chain[12].score_, 31336.87361038887);
  EXPECT_DOUBLE_EQ(chain[13].score_, 31335.911377193759);
  EXPECT_DOUBLE_EQ(chain[14].score_, 31335.911377193759);
  EXPECT_DOUBLE_EQ(chain[15].score_, 31336.115282239854);
  EXPECT_DOUBLE_EQ(chain[16].score_, 31335.791212351458);
  EXPECT_DOUBLE_EQ(chain[17].score_, 31353.196700932273);
  EXPECT_DOUBLE_EQ(chain[18].score_, 31352.544811853302);
  EXPECT_DOUBLE_EQ(chain[19].score_, 31349.645126084823);
  EXPECT_DOUBLE_EQ(chain[20].score_, 31349.495281098607);
  EXPECT_DOUBLE_EQ(chain[21].score_, 31347.66302564969);
  EXPECT_DOUBLE_EQ(chain[22].score_, 31347.257579932004);
  EXPECT_DOUBLE_EQ(chain[23].score_, 31345.804670039342);
  EXPECT_DOUBLE_EQ(chain[24].score_, 31347.849428154186);
#elif _WIN64
  EXPECT_DOUBLE_EQ(chain[0].score_, 26439.620838100392575);
  EXPECT_DOUBLE_EQ(chain[1].score_, 26438.939148705259868);
  EXPECT_DOUBLE_EQ(chain[2].score_, 26436.716361595710623);
  EXPECT_DOUBLE_EQ(chain[3].score_, 26436.270875127793261);
  EXPECT_DOUBLE_EQ(chain[4].score_, 26434.976273072981712);
  EXPECT_DOUBLE_EQ(chain[5].score_, 26435.92486035615002);
  EXPECT_DOUBLE_EQ(chain[6].score_, 26433.400062890883419);
  EXPECT_DOUBLE_EQ(chain[7].score_, 26433.400062890883419);
  EXPECT_DOUBLE_EQ(chain[8].score_, 26433.553438238308445);
  EXPECT_DOUBLE_EQ(chain[9].score_, 26430.721650127838075);
  EXPECT_DOUBLE_EQ(chain[10].score_, 26429.992692499967234);
  EXPECT_DOUBLE_EQ(chain[11].score_, 26428.628996958799689);
  EXPECT_DOUBLE_EQ(chain[12].score_, 26428.349099746465072);
  EXPECT_DOUBLE_EQ(chain[13].score_, 26428.844178388575529);
  EXPECT_DOUBLE_EQ(chain[14].score_, 26360.168229553743004);
  EXPECT_DOUBLE_EQ(chain[15].score_, 26366.893333353003982);
  EXPECT_DOUBLE_EQ(chain[16].score_, 26366.568797878040641);
  EXPECT_DOUBLE_EQ(chain[17].score_, 26364.670679732371354);
  EXPECT_DOUBLE_EQ(chain[18].score_, 26364.56108402996324);
  EXPECT_DOUBLE_EQ(chain[19].score_, 26364.152701565632015);
  EXPECT_DOUBLE_EQ(chain[20].score_, 26369.560192795433977);
  EXPECT_DOUBLE_EQ(chain[21].score_, 26374.04230406428178);
  EXPECT_DOUBLE_EQ(chain[22].score_, 26364.52020825065847);
  EXPECT_DOUBLE_EQ(chain[23].score_, 26364.596421441088751);
  EXPECT_DOUBLE_EQ(chain[24].score_, 26362.487774858731427);
#elif __linux__
  EXPECT_NEAR(chain[0].score_, 36389.07677669402, 1e-5);
  EXPECT_NEAR(chain[1].score_, 36389.07677669402, 1e-5);
  EXPECT_NEAR(chain[2].score_, 36388.361465831, 1e-5);
  EXPECT_NEAR(chain[3].score_, 36388.361465831, 1e-5);
  EXPECT_NEAR(chain[4].score_, 36388.361465831, 1e-5);
  EXPECT_NEAR(chain[5].score_, 36388.8173073483, 1e-5);
  EXPECT_NEAR(chain[6].score_, 36388.8173073483, 1e-5);
  EXPECT_NEAR(chain[7].score_, 36388.8173073483, 1e-5);
  EXPECT_NEAR(chain[8].score_, 36388.8173073483, 1e-5);
  EXPECT_NEAR(chain[9].score_, 36388.8173073483, 1e-5);
  EXPECT_NEAR(chain[10].score_, 36388.8173073483, 1e-5);
  EXPECT_NEAR(chain[11].score_, 36388.50090547473, 1e-5);
  EXPECT_NEAR(chain[12].score_, 36374.96143373416, 1e-5);
  EXPECT_NEAR(chain[13].score_, 36365.27657489545, 1e-5);
  EXPECT_NEAR(chain[14].score_, 36322.78711909857, 1e-5);
  EXPECT_NEAR(chain[15].score_, 36322.48103413592, 1e-5);
  EXPECT_NEAR(chain[16].score_, 36322.90573281318, 1e-5);
  EXPECT_NEAR(chain[17].score_, 36324.69622726875, 1e-5);
  EXPECT_NEAR(chain[18].score_, 36322.25413765675, 1e-5);
  EXPECT_NEAR(chain[19].score_, 36323.19781703433, 1e-5);
  EXPECT_NEAR(chain[20].score_, 36312.8207311583, 1e-5);
  EXPECT_NEAR(chain[21].score_, 36315.46101229153, 1e-5);
  EXPECT_NEAR(chain[22].score_, 36311.54510717453, 1e-5);
  EXPECT_NEAR(chain[23].score_, 36326.95904361141, 1e-5);
  EXPECT_NEAR(chain[24].score_, 36320.59445996349, 1e-5);
#endif
}

/**
 * @brief Construct a new test f object
 *
 */
TEST_F(HamiltonianMonteCarloThreadedModel, TwentyFive_Iteration_With_CasalComplexOne_RandomStart_OneEFourStepSize) {
  string ammended_definition = testcases::test_cases_casal_complex_1;
  boost::replace_all(ammended_definition, "threads 1", "threads 4");
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  AddConfigurationLine(ammended_definition, __FILE__, 76);

  string mcmc_definition = R"(
    @mcmc my_mcmc
    type hamiltonian
    length 25
    start 1
    step_size 0.02
    keep 1
    leapfrog_delta 1e-4
    leapfrog_steps 4
  )";
  AddConfigurationLine(mcmc_definition, __FILE__, 70);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kMCMC));
  auto model = runner_->model();
  auto mcmc  = model->managers()->mcmc()->active_mcmc();
  ASSERT_TRUE(mcmc != nullptr);

  auto chain = mcmc->chain();
  ASSERT_EQ(25u, chain.size());

#ifdef __GITHUB_ACTIONS_WINDOWS

#elif _WIN64
  EXPECT_DOUBLE_EQ(chain[0].score_, 11565.956258254713248);
  EXPECT_DOUBLE_EQ(chain[1].score_, 11563.945542542387557);
  EXPECT_DOUBLE_EQ(chain[2].score_, 11562.403964855100639);
  EXPECT_DOUBLE_EQ(chain[3].score_, 11556.223840474351164);
  EXPECT_DOUBLE_EQ(chain[4].score_, 11556.223840474351164);
  EXPECT_DOUBLE_EQ(chain[5].score_, 11538.665841878730134);
  EXPECT_DOUBLE_EQ(chain[6].score_, 11538.665841878730134);
  EXPECT_DOUBLE_EQ(chain[7].score_, 11542.452783154632925);
  EXPECT_DOUBLE_EQ(chain[8].score_, 11559.318709193883478);
  EXPECT_DOUBLE_EQ(chain[9].score_, 11563.362169696029014);
  EXPECT_DOUBLE_EQ(chain[10].score_, 11547.913278676778646);
  EXPECT_DOUBLE_EQ(chain[11].score_, 11542.315814010795293);
  EXPECT_DOUBLE_EQ(chain[12].score_, 11537.110225219963468);
  EXPECT_DOUBLE_EQ(chain[13].score_, 11536.834146950903232);
  EXPECT_DOUBLE_EQ(chain[14].score_, 11534.602795194192367);
  EXPECT_DOUBLE_EQ(chain[15].score_, 11523.536728180339196);
  EXPECT_DOUBLE_EQ(chain[16].score_, 11523.275861281310426);
  EXPECT_DOUBLE_EQ(chain[17].score_, 11544.741998528204931);
  EXPECT_DOUBLE_EQ(chain[18].score_, 11556.198860973217961);
  EXPECT_DOUBLE_EQ(chain[19].score_, 11556.198860973217961);
  EXPECT_DOUBLE_EQ(chain[20].score_, 11548.83913677889359);
  EXPECT_DOUBLE_EQ(chain[21].score_, 11542.115686500175798);
  EXPECT_DOUBLE_EQ(chain[22].score_, 11545.440962512842816);
  EXPECT_DOUBLE_EQ(chain[23].score_, 11535.675498516950029);
  EXPECT_DOUBLE_EQ(chain[24].score_, 11532.701532135375601);
#elif __linux__
  // EXPECT_NEAR(chain[0].score_, 11622.04009177895, 1e-5);
  // EXPECT_NEAR(chain[1].score_, 11620.01985703409, 1e-5);
  // EXPECT_NEAR(chain[2].score_, 11618.73341844001, 1e-5);
  // EXPECT_NEAR(chain[3].score_, 11618.21271520679, 1e-5);
  // EXPECT_NEAR(chain[4].score_, 11618.21271520679, 1e-5);
  // EXPECT_NEAR(chain[5].score_, 11600.91618645375, 1e-5);
  // EXPECT_NEAR(chain[6].score_, 11591.3493038226, 1e-5);
  // EXPECT_NEAR(chain[7].score_, 11600.90538041638, 1e-5);
  // EXPECT_NEAR(chain[8].score_, 11600.90538041638, 1e-5);
  // EXPECT_NEAR(chain[9].score_, 11601.31245975959, 1e-5);
  // EXPECT_NEAR(chain[10].score_, 11589.56936167276, 1e-5);
  // EXPECT_NEAR(chain[11].score_, 11594.77573869354, 1e-5);
  // EXPECT_NEAR(chain[12].score_, 11582.39226485891, 1e-5);
  // EXPECT_NEAR(chain[13].score_, 11582.72638786315, 1e-5);
  // EXPECT_NEAR(chain[14].score_, 11576.40774277365, 1e-5);
  // EXPECT_NEAR(chain[15].score_, 11581.47904821901, 1e-5);
  // EXPECT_NEAR(chain[16].score_, 11591.86020479118, 1e-5);
  // EXPECT_NEAR(chain[17].score_, 11586.80098401977, 1e-5);
  // EXPECT_NEAR(chain[18].score_, 11588.94000961911, 1e-5);
  // EXPECT_NEAR(chain[19].score_, 11588.94000961911, 1e-5);
  // EXPECT_NEAR(chain[20].score_, 11584.2109864414, 1e-5);
  // EXPECT_NEAR(chain[21].score_, 11577.90015058309, 1e-5);
  // EXPECT_NEAR(chain[22].score_, 11577.90015058309, 1e-5);
  // EXPECT_NEAR(chain[23].score_, 11563.32968127872, 1e-5);
  // EXPECT_NEAR(chain[24].score_, 11556.61019993286, 1e-5);
#endif
}

/**
 * @brief Construct a new test f object
 *
 */
TEST_F(HamiltonianMonteCarloThreadedModel, TwentyFive_Iteration_With_CasalComplexOne_RandomStart_OneEFourStepSize_LeapFrogs_Ten) {
  string ammended_definition = testcases::test_cases_casal_complex_1;
  boost::replace_all(ammended_definition, "threads 1", "threads 4");
  boost::replace_all(ammended_definition, "numerical_differences", "deltadiff");
  AddConfigurationLine(ammended_definition, __FILE__, 76);

  string mcmc_definition = R"(
    @mcmc my_mcmc
    type hamiltonian
    length 25
    start 1
    step_size 0.02
    keep 1
    leapfrog_delta 1e-4
    leapfrog_steps 10
  )";
  AddConfigurationLine(mcmc_definition, __FILE__, 70);
  LoadConfiguration();

  ASSERT_NO_THROW(runner_->GoWithRunMode(RunMode::kMCMC));
  auto model = runner_->model();
  auto mcmc  = model->managers()->mcmc()->active_mcmc();
  ASSERT_TRUE(mcmc != nullptr);

  auto chain = mcmc->chain();
  ASSERT_EQ(25u, chain.size());

#ifdef __GITHUB_ACTIONS_WINDOWS

#elif _WIN64
  EXPECT_DOUBLE_EQ(chain[0].score_, 11565.956258254713248);
  EXPECT_DOUBLE_EQ(chain[1].score_, 11560.925391541399222);
  EXPECT_DOUBLE_EQ(chain[2].score_, 11556.358222655624559);
  EXPECT_DOUBLE_EQ(chain[3].score_, 11547.160905872500734);
  EXPECT_DOUBLE_EQ(chain[4].score_, 11547.160905872500734);
  EXPECT_DOUBLE_EQ(chain[5].score_, 11526.616702604178499);
  EXPECT_DOUBLE_EQ(chain[6].score_, 11526.616702604178499);
  EXPECT_DOUBLE_EQ(chain[7].score_, 11527.36910459252249);
  EXPECT_DOUBLE_EQ(chain[8].score_, 11541.173675399866625);
  EXPECT_DOUBLE_EQ(chain[9].score_, 11542.176350763020309);
  EXPECT_DOUBLE_EQ(chain[10].score_, 11523.785378120803216);
  EXPECT_DOUBLE_EQ(chain[11].score_, 11515.203344504101551);
  EXPECT_DOUBLE_EQ(chain[12].score_, 11506.993057282208611);
  EXPECT_DOUBLE_EQ(chain[13].score_, 11503.69681019298514);
  EXPECT_DOUBLE_EQ(chain[14].score_, 11498.440172131728104);
  EXPECT_DOUBLE_EQ(chain[15].score_, 11484.458059830782076);
  EXPECT_DOUBLE_EQ(chain[16].score_, 11481.239832735902382);
  EXPECT_DOUBLE_EQ(chain[17].score_, 11499.426411198786809);
  EXPECT_DOUBLE_EQ(chain[18].score_, 11507.756054537794626);
  EXPECT_DOUBLE_EQ(chain[19].score_, 11507.756054537794626);
  EXPECT_DOUBLE_EQ(chain[20].score_, 11497.442242054468807);
  EXPECT_DOUBLE_EQ(chain[21].score_, 11487.765671450759328);
  EXPECT_DOUBLE_EQ(chain[22].score_, 11488.138485860403307);
  EXPECT_DOUBLE_EQ(chain[23].score_, 11475.37891402193236);
  EXPECT_DOUBLE_EQ(chain[24].score_, 11469.454839964448183);
#elif __linux__
  // EXPECT_NEAR(chain[0].score_, 11622.04009177895, 1e-5);
  // EXPECT_NEAR(chain[1].score_, 11616.98912320913, 1e-5);
  // EXPECT_NEAR(chain[2].score_, 11612.66679924271, 1e-5);
  // EXPECT_NEAR(chain[3].score_, 11609.11006547069, 1e-5);
  // EXPECT_NEAR(chain[4].score_, 11609.11006547069, 1e-5);
  // EXPECT_NEAR(chain[5].score_, 11588.81821244274, 1e-5);
  // EXPECT_NEAR(chain[6].score_, 11576.18072649907, 1e-5);
  // EXPECT_NEAR(chain[7].score_, 11582.66232534397, 1e-5);
  // EXPECT_NEAR(chain[8].score_, 11602.24104623353, 1e-5);
  // EXPECT_NEAR(chain[9].score_, 11599.60566906656, 1e-5);
  // EXPECT_NEAR(chain[10].score_, 11584.84484506599, 1e-5);
  // EXPECT_NEAR(chain[11].score_, 11587.00651477895, 1e-5);
  // EXPECT_NEAR(chain[12].score_, 11571.63800588351, 1e-5);
  // EXPECT_NEAR(chain[13].score_, 11568.92469216468, 1e-5);
  // EXPECT_NEAR(chain[14].score_, 11559.61656781061, 1e-5);
  // EXPECT_NEAR(chain[15].score_, 11561.62636514807, 1e-5);
  // EXPECT_NEAR(chain[16].score_, 11568.94428881099, 1e-5);
  // EXPECT_NEAR(chain[17].score_, 11560.86209234402, 1e-5);
  // EXPECT_NEAR(chain[18].score_, 11559.94823220482, 1e-5);
  // EXPECT_NEAR(chain[19].score_, 11622.22043635682, 1e-5);
  // EXPECT_NEAR(chain[20].score_, 11614.3868750975, 1e-5);
  // EXPECT_NEAR(chain[21].score_, 11604.94834979492, 1e-5);
  // EXPECT_NEAR(chain[22].score_, 11604.94834979492, 1e-5);
  // EXPECT_NEAR(chain[23].score_, 11587.16841312723, 1e-5);
  // EXPECT_NEAR(chain[24].score_, 11577.35793797639, 1e-5);
#endif
}

}  // namespace niwa
#endif  // USE_AUTODIFF
#endif  // TESTMODE
