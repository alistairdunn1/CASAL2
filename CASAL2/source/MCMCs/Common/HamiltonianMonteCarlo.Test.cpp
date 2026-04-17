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
#include "HamiltonianMonteCarlo.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

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
#include "Utilities/Math.h"

// Namespaces
namespace niwa {

using niwa::MockMPD;
using niwa::utilities::math::PI;
using std::cout;
using std::endl;
using ::testing::NiceMock;

class HamiltonianMonteCarloThreadedModel : public testfixtures::BaseThreaded {};

class TestHamiltonianMonteCarlo : public niwa::mcmcs::HamiltonianMonteCarlo {
public:
  TestHamiltonianMonteCarlo(shared_ptr<Model> model) : niwa::mcmcs::HamiltonianMonteCarlo(model) {}

  // Test wrappers for protected methods
  double TestNorm2(const std::vector<double>& target) { return this->norm2(target); }
  double TestLogDensity(const std::vector<double>& x, const std::vector<double>& mean, const std::vector<std::vector<double>>& cov_l, double log_det, double constant) {
    return this->logDensity(x, mean, cov_l, log_det, constant);
  }
  double TestComputeKineticEnergy(const std::vector<double>& momentum) { return this->computeKineticEnergy(momentum); }
  double TestComputeHamiltonian(double potential_energy, double kinetic_energy) { return this->computeHamiltonian(potential_energy, kinetic_energy); }
  double TestComputeAcceptanceProbability(double H_current, double H_proposed) { return this->computeAcceptanceProbability(H_current, H_proposed); }

  void TestLeapfrogHalfMomentumStep(std::vector<double>& momentum, const std::vector<double>& gradient, double delta) { this->leapfrogHalfMomentumStep(momentum, gradient, delta); }
  void TestLeapfrogFullPositionStep(std::vector<double>& position, const std::vector<double>& momentum, double delta) { this->leapfrogFullPositionStep(position, momentum, delta); }

  // Set bounds for scale/unscale testing
  void SetBounds(const std::vector<double>& lower, const std::vector<double>& upper) {
    estimate_lower_bounds_ = lower;
    estimate_upper_bounds_ = upper;
  }
  void TestScalePosition(std::vector<double>& position) { this->scalePosition(position); }
  void TestUnscalePosition(std::vector<double>& position) { this->unscalePosition(position); }

  // New method wrappers for momentum sampling and Jacobian
  std::vector<double> TestSampleMomentum(size_t size) { return this->sampleMomentum(size); }
  double              TestComputeLogJacobian(const std::vector<double>& position_unbounded) { return this->computeLogJacobian(position_unbounded); }
};

std::unique_ptr<TestHamiltonianMonteCarlo> CreateHMCForTesting() {
  auto mock_model = std::make_shared<NiceMock<MockModel>>();

  auto hmc = std::make_unique<TestHamiltonianMonteCarlo>(mock_model);
  return hmc;
}

TEST_F(HamiltonianMonteCarloThreadedModel, Norm2_Empty_Vector) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> test_vector = {};
  double              result      = hmc->TestNorm2(test_vector);
  EXPECT_DOUBLE_EQ(result, 0.0);
}

TEST_F(HamiltonianMonteCarloThreadedModel, Norm2) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> test_vector = {-0.4249, -0.2915};
  double              result      = hmc->TestNorm2(test_vector);
  EXPECT_DOUBLE_EQ(result, 0.26551226);
}

TEST_F(HamiltonianMonteCarloThreadedModel, LogDensity_1D_StandardNormal) {
  auto hmc = CreateHMCForTesting();

  // Test 1D standard normal: mean=0, variance=1
  // Cholesky of [[1]] is [[1]]
  std::vector<double>              x        = {0.0};
  std::vector<double>              mean     = {0.0};
  std::vector<std::vector<double>> cov_l    = {{1.0}};
  double                           log_det  = 0.0;                        // log(1) = 0
  double                           constant = -0.5 * std::log(2.0 * PI);  // -n/2 * log(2*pi) for n=1

  double result = hmc->TestLogDensity(x, mean, cov_l, log_det, constant);
  // At mean, log density = constant - log_det - 0.5 * 0 = -0.5 * log(2*pi)
  double expected = -0.5 * std::log(2.0 * PI);
  EXPECT_NEAR(result, expected, 1e-10);
}

TEST_F(HamiltonianMonteCarloThreadedModel, LogDensity_1D_AwayFromMean) {
  auto hmc = CreateHMCForTesting();

  // Test 1D normal at x=1, mean=0, variance=1
  std::vector<double>              x        = {1.0};
  std::vector<double>              mean     = {0.0};
  std::vector<std::vector<double>> cov_l    = {{1.0}};
  double                           log_det  = 0.0;
  double                           constant = -0.5 * std::log(2.0 * PI);

  double result = hmc->TestLogDensity(x, mean, cov_l, log_det, constant);
  // At x=1, (x-mean)^2 = 1, so log density = constant - 0.5 * 1
  double expected = -0.5 * std::log(2.0 * PI) - 0.5;
  EXPECT_NEAR(result, expected, 1e-10);
}

TEST_F(HamiltonianMonteCarloThreadedModel, LogDensity_2D_IndependentNormal) {
  auto hmc = CreateHMCForTesting();

  // Test 2D independent normal: mean=[0,0], covariance=I (identity)
  // Cholesky of I is I
  std::vector<double>              x        = {0.0, 0.0};
  std::vector<double>              mean     = {0.0, 0.0};
  std::vector<std::vector<double>> cov_l    = {{1.0, 0.0}, {0.0, 1.0}};
  double                           log_det  = 0.0;                        // log(det(I)) = log(1) = 0
  double                           constant = -1.0 * std::log(2.0 * PI);  // -n/2 * log(2*pi) for n=2

  double result = hmc->TestLogDensity(x, mean, cov_l, log_det, constant);
  // At mean, Mahalanobis distance = 0
  double expected = -std::log(2.0 * PI);
  EXPECT_NEAR(result, expected, 1e-10);
}

TEST_F(HamiltonianMonteCarloThreadedModel, LogDensity_2D_AwayFromMean) {
  auto hmc = CreateHMCForTesting();

  // Test 2D at x=[1,1], mean=[0,0], covariance=I
  std::vector<double>              x        = {1.0, 1.0};
  std::vector<double>              mean     = {0.0, 0.0};
  std::vector<std::vector<double>> cov_l    = {{1.0, 0.0}, {0.0, 1.0}};
  double                           log_det  = 0.0;
  double                           constant = -1.0 * std::log(2.0 * PI);

  double result = hmc->TestLogDensity(x, mean, cov_l, log_det, constant);
  // Mahalanobis distance = 1^2 + 1^2 = 2
  double expected = -std::log(2.0 * PI) - 0.5 * 2.0;
  EXPECT_NEAR(result, expected, 1e-10);
}

TEST_F(HamiltonianMonteCarloThreadedModel, LogDensity_2D_Correlated) {
  auto hmc = CreateHMCForTesting();

  // Test 2D correlated normal
  // Covariance matrix: [[1, 0.5], [0.5, 1]]
  // Cholesky decomposition L: [[1, 0], [0.5, sqrt(0.75)]]
  std::vector<double>              x     = {0.0, 0.0};
  std::vector<double>              mean  = {0.0, 0.0};
  std::vector<std::vector<double>> cov_l = {{1.0, 0.0}, {0.5, std::sqrt(0.75)}};
  // det(cov) = 1*1 - 0.5*0.5 = 0.75
  double log_det  = std::log(0.75);
  double constant = -1.0 * std::log(2.0 * PI);

  double result = hmc->TestLogDensity(x, mean, cov_l, log_det, constant);
  // At mean, Mahalanobis distance = 0
  double expected = -std::log(2.0 * PI) - std::log(0.75);
  EXPECT_NEAR(result, expected, 1e-10);
}

TEST_F(HamiltonianMonteCarloThreadedModel, LogDensity_2D_NonZeroMean) {
  auto hmc = CreateHMCForTesting();

  // Test 2D at mean=[2,3], x=[2,3], covariance=I
  std::vector<double>              x        = {2.0, 3.0};
  std::vector<double>              mean     = {2.0, 3.0};
  std::vector<std::vector<double>> cov_l    = {{1.0, 0.0}, {0.0, 1.0}};
  double                           log_det  = 0.0;
  double                           constant = -1.0 * std::log(2.0 * PI);

  double result = hmc->TestLogDensity(x, mean, cov_l, log_det, constant);
  // At mean, distance = 0
  double expected = -std::log(2.0 * PI);
  EXPECT_NEAR(result, expected, 1e-10);
}

TEST_F(HamiltonianMonteCarloThreadedModel, LogDensity_3D_Diagonal) {
  auto hmc = CreateHMCForTesting();

  // Test 3D with diagonal covariance [[4,0,0],[0,1,0],[0,0,9]]
  // Cholesky L = [[2,0,0],[0,1,0],[0,0,3]]
  std::vector<double>              x     = {1.0, 1.0, 1.0};
  std::vector<double>              mean  = {0.0, 0.0, 0.0};
  std::vector<std::vector<double>> cov_l = {{2.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 3.0}};
  // det(cov) = 4 * 1 * 9 = 36
  double log_det  = std::log(36.0);
  double constant = -1.5 * std::log(2.0 * PI);  // -n/2 * log(2*pi) for n=3

  double result = hmc->TestLogDensity(x, mean, cov_l, log_det, constant);
  // Mahalanobis: (1-0)^2/4 + (1-0)^2/1 + (1-0)^2/9 = 0.25 + 1 + 0.111... = 1.361...
  double mahalanobis = 1.0 / 4.0 + 1.0 / 1.0 + 1.0 / 9.0;
  double expected    = -1.5 * std::log(2.0 * PI) - std::log(36.0) - 0.5 * mahalanobis;
  EXPECT_NEAR(result, expected, 1e-10);
}

// ============================================================================
// Kinetic Energy Tests
// ============================================================================

/**
 * @brief Test kinetic energy with zero momentum
 *
 * K(p) = 0.5 * ||p||^2 = 0 when p = 0
 */
TEST_F(HamiltonianMonteCarloThreadedModel, KineticEnergy_ZeroMomentum) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> momentum = {0.0, 0.0, 0.0};
  double              result   = hmc->TestComputeKineticEnergy(momentum);
  EXPECT_DOUBLE_EQ(result, 0.0);
}

/**
 * @brief Test kinetic energy with unit momentum
 *
 * K(p) = 0.5 * (1^2 + 1^2) = 1.0
 */
TEST_F(HamiltonianMonteCarloThreadedModel, KineticEnergy_UnitMomentum) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> momentum = {1.0, 1.0};
  double              result   = hmc->TestComputeKineticEnergy(momentum);
  // K = 0.5 * (1 + 1) = 1.0
  EXPECT_DOUBLE_EQ(result, 1.0);
}

/**
 * @brief Test kinetic energy with arbitrary momentum values
 *
 * K(p) = 0.5 * (2^2 + 3^2 + 4^2) = 0.5 * (4 + 9 + 16) = 14.5
 */
TEST_F(HamiltonianMonteCarloThreadedModel, KineticEnergy_ArbitraryMomentum) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> momentum = {2.0, 3.0, 4.0};
  double              result   = hmc->TestComputeKineticEnergy(momentum);
  // K = 0.5 * (4 + 9 + 16) = 0.5 * 29 = 14.5
  EXPECT_DOUBLE_EQ(result, 14.5);
}

/**
 * @brief Test kinetic energy with negative momentum values
 *
 * K(p) = 0.5 * ((-2)^2 + (-3)^2) = 0.5 * (4 + 9) = 6.5
 */
TEST_F(HamiltonianMonteCarloThreadedModel, KineticEnergy_NegativeMomentum) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> momentum = {-2.0, -3.0};
  double              result   = hmc->TestComputeKineticEnergy(momentum);
  // K = 0.5 * (4 + 9) = 6.5
  EXPECT_DOUBLE_EQ(result, 6.5);
}

/**
 * @brief Test kinetic energy with empty momentum vector
 */
TEST_F(HamiltonianMonteCarloThreadedModel, KineticEnergy_EmptyMomentum) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> momentum = {};
  double              result   = hmc->TestComputeKineticEnergy(momentum);
  EXPECT_DOUBLE_EQ(result, 0.0);
}

// ============================================================================
// Hamiltonian Tests
// ============================================================================

/**
 * @brief Test Hamiltonian with zero energies
 *
 * H = U + K = 0 + 0 = 0
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Hamiltonian_ZeroEnergies) {
  auto hmc = CreateHMCForTesting();

  double result = hmc->TestComputeHamiltonian(0.0, 0.0);
  EXPECT_DOUBLE_EQ(result, 0.0);
}

/**
 * @brief Test Hamiltonian computation
 *
 * H = U + K = 5.5 + 3.2 = 8.7
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Hamiltonian_PositiveEnergies) {
  auto hmc = CreateHMCForTesting();

  double potential_energy = 5.5;
  double kinetic_energy   = 3.2;
  double result           = hmc->TestComputeHamiltonian(potential_energy, kinetic_energy);
  EXPECT_DOUBLE_EQ(result, 8.7);
}

/**
 * @brief Test Hamiltonian with negative potential (valid in log-posterior context)
 *
 * H = U + K = -10.0 + 3.0 = -7.0
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Hamiltonian_NegativePotential) {
  auto hmc = CreateHMCForTesting();

  double result = hmc->TestComputeHamiltonian(-10.0, 3.0);
  EXPECT_DOUBLE_EQ(result, -7.0);
}

// ============================================================================
// Acceptance Probability Tests
// ============================================================================

/**
 * @brief Test acceptance probability when Hamiltonians are equal
 *
 * alpha = min(1, exp(0)) = 1.0
 */
TEST_F(HamiltonianMonteCarloThreadedModel, AcceptanceProbability_EqualHamiltonians) {
  auto hmc = CreateHMCForTesting();

  double result = hmc->TestComputeAcceptanceProbability(100.0, 100.0);
  EXPECT_DOUBLE_EQ(result, 1.0);
}

/**
 * @brief Test acceptance probability when proposed is lower (better)
 *
 * H_proposed < H_current means delta_H < 0, so exp(-delta_H) > 1
 * alpha = min(1, exp(-delta_H)) = 1.0
 */
TEST_F(HamiltonianMonteCarloThreadedModel, AcceptanceProbability_ProposedLower) {
  auto hmc = CreateHMCForTesting();

  // H_current = 100, H_proposed = 90, delta_H = -10
  // alpha = min(1, exp(10)) = 1.0
  double result = hmc->TestComputeAcceptanceProbability(100.0, 90.0);
  EXPECT_DOUBLE_EQ(result, 1.0);
}

/**
 * @brief Test acceptance probability when proposed is higher (worse)
 *
 * H_proposed > H_current means delta_H > 0, so exp(-delta_H) < 1
 * alpha = exp(-delta_H)
 */
TEST_F(HamiltonianMonteCarloThreadedModel, AcceptanceProbability_ProposedHigher) {
  auto hmc = CreateHMCForTesting();

  // H_current = 100, H_proposed = 102, delta_H = 2
  // alpha = exp(-2) ≈ 0.1353
  double result   = hmc->TestComputeAcceptanceProbability(100.0, 102.0);
  double expected = std::exp(-2.0);
  EXPECT_NEAR(result, expected, 1e-10);
}

/**
 * @brief Test acceptance probability with large energy difference
 */
TEST_F(HamiltonianMonteCarloThreadedModel, AcceptanceProbability_LargeDifference) {
  auto hmc = CreateHMCForTesting();

  // H_current = 100, H_proposed = 150, delta_H = 50
  // alpha = exp(-50) ≈ 1.93e-22
  double result   = hmc->TestComputeAcceptanceProbability(100.0, 150.0);
  double expected = std::exp(-50.0);
  EXPECT_NEAR(result, expected, 1e-30);
}

// ============================================================================
// Leapfrog Half Momentum Step Tests
// ============================================================================

/**
 * @brief Test half momentum step with zero gradient
 *
 * p = p - (dt/2) * 0 = p (no change)
 */
TEST_F(HamiltonianMonteCarloThreadedModel, LeapfrogHalfMomentum_ZeroGradient) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> momentum = {1.0, 2.0, 3.0};
  std::vector<double> gradient = {0.0, 0.0, 0.0};
  double              delta    = 0.1;

  hmc->TestLeapfrogHalfMomentumStep(momentum, gradient, delta);

  EXPECT_DOUBLE_EQ(momentum[0], 1.0);
  EXPECT_DOUBLE_EQ(momentum[1], 2.0);
  EXPECT_DOUBLE_EQ(momentum[2], 3.0);
}

/**
 * @brief Test half momentum step with unit gradient
 *
 * p = p - (dt/2) * grad
 * With dt = 0.1 and grad = [1, 1], p_new = [1 - 0.05, 2 - 0.05] = [0.95, 1.95]
 */
TEST_F(HamiltonianMonteCarloThreadedModel, LeapfrogHalfMomentum_UnitGradient) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> momentum = {1.0, 2.0};
  std::vector<double> gradient = {1.0, 1.0};
  double              delta    = 0.1;

  hmc->TestLeapfrogHalfMomentumStep(momentum, gradient, delta);

  // half_delta = 0.1 / 2 = 0.05
  // p[0] = 1.0 - 0.05 * 1.0 = 0.95
  // p[1] = 2.0 - 0.05 * 1.0 = 1.95
  EXPECT_DOUBLE_EQ(momentum[0], 0.95);
  EXPECT_DOUBLE_EQ(momentum[1], 1.95);
}

/**
 * @brief Test half momentum step with arbitrary values
 */
TEST_F(HamiltonianMonteCarloThreadedModel, LeapfrogHalfMomentum_ArbitraryValues) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> momentum = {5.0, -3.0, 2.5};
  std::vector<double> gradient = {2.0, -4.0, 1.0};
  double              delta    = 0.2;  // half_delta = 0.1

  hmc->TestLeapfrogHalfMomentumStep(momentum, gradient, delta);

  // p[0] = 5.0 - 0.1 * 2.0 = 4.8
  // p[1] = -3.0 - 0.1 * (-4.0) = -3.0 + 0.4 = -2.6
  // p[2] = 2.5 - 0.1 * 1.0 = 2.4
  EXPECT_DOUBLE_EQ(momentum[0], 4.8);
  EXPECT_DOUBLE_EQ(momentum[1], -2.6);
  EXPECT_DOUBLE_EQ(momentum[2], 2.4);
}

// ============================================================================
// Leapfrog Full Position Step Tests
// ============================================================================

/**
 * @brief Test full position step with zero momentum
 *
 * q = q + dt * 0 = q (no change)
 */
TEST_F(HamiltonianMonteCarloThreadedModel, LeapfrogFullPosition_ZeroMomentum) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> position = {1.0, 2.0, 3.0};
  std::vector<double> momentum = {0.0, 0.0, 0.0};
  double              delta    = 0.1;

  hmc->TestLeapfrogFullPositionStep(position, momentum, delta);

  EXPECT_DOUBLE_EQ(position[0], 1.0);
  EXPECT_DOUBLE_EQ(position[1], 2.0);
  EXPECT_DOUBLE_EQ(position[2], 3.0);
}

/**
 * @brief Test full position step with unit momentum
 *
 * q = q + dt * p
 * With dt = 0.1 and p = [1, 1], q_new = [1 + 0.1, 2 + 0.1] = [1.1, 2.1]
 */
TEST_F(HamiltonianMonteCarloThreadedModel, LeapfrogFullPosition_UnitMomentum) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> position = {1.0, 2.0};
  std::vector<double> momentum = {1.0, 1.0};
  double              delta    = 0.1;

  hmc->TestLeapfrogFullPositionStep(position, momentum, delta);

  EXPECT_DOUBLE_EQ(position[0], 1.1);
  EXPECT_DOUBLE_EQ(position[1], 2.1);
}

/**
 * @brief Test full position step with arbitrary values
 */
TEST_F(HamiltonianMonteCarloThreadedModel, LeapfrogFullPosition_ArbitraryValues) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> position = {0.0, 5.0, -2.5};
  std::vector<double> momentum = {3.0, -2.0, 4.0};
  double              delta    = 0.5;

  hmc->TestLeapfrogFullPositionStep(position, momentum, delta);

  // q[0] = 0.0 + 0.5 * 3.0 = 1.5
  // q[1] = 5.0 + 0.5 * (-2.0) = 4.0
  // q[2] = -2.5 + 0.5 * 4.0 = -0.5
  EXPECT_DOUBLE_EQ(position[0], 1.5);
  EXPECT_DOUBLE_EQ(position[1], 4.0);
  EXPECT_DOUBLE_EQ(position[2], -0.5);
}

// ============================================================================
// Scale/Unscale Position Tests
// ============================================================================

/**
 * @brief Test that scale followed by unscale returns original value
 *
 * Tests the round-trip property: unscale(scale(x)) ≈ x
 */
TEST_F(HamiltonianMonteCarloThreadedModel, ScaleUnscale_RoundTrip) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> lower = {0.0, -10.0, 1.0};
  std::vector<double> upper = {1.0, 10.0, 100.0};
  hmc->SetBounds(lower, upper);

  // Test point in the middle of bounds
  std::vector<double> position = {0.5, 0.0, 50.0};
  std::vector<double> original = position;

  hmc->TestScalePosition(position);
  hmc->TestUnscalePosition(position);

  for (size_t i = 0; i < original.size(); ++i) {
    EXPECT_NEAR(position[i], original[i], 1e-10);
  }
}

/**
 * @brief Test scale at midpoint of bounds
 *
 * At the midpoint, scaled value should be 0 (since tan(0) = 0)
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Scale_Midpoint) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> lower = {0.0};
  std::vector<double> upper = {10.0};
  hmc->SetBounds(lower, upper);

  std::vector<double> position = {5.0};  // midpoint of [0, 10]
  hmc->TestScalePosition(position);

  // At midpoint, (x - min) / (max - min) = 0.5, so arg to tan is 0
  EXPECT_NEAR(position[0], 0.0, 1e-10);
}

/**
 * @brief Test unscale at zero (scaled midpoint)
 *
 * Unscaling 0 should return the midpoint of bounds
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Unscale_Zero) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> lower = {0.0};
  std::vector<double> upper = {10.0};
  hmc->SetBounds(lower, upper);

  std::vector<double> position = {0.0};  // scaled midpoint
  hmc->TestUnscalePosition(position);

  // Should return midpoint
  EXPECT_NEAR(position[0], 5.0, 1e-10);
}

/**
 * @brief Test scaling near bounds
 *
 * Values near bounds should scale to large positive/negative values
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Scale_NearBounds) {
  auto hmc = CreateHMCForTesting();

  std::vector<double> lower = {0.0, 0.0};
  std::vector<double> upper = {1.0, 1.0};
  hmc->SetBounds(lower, upper);

  // Test near lower and upper bounds
  std::vector<double> position = {0.1, 0.9};
  hmc->TestScalePosition(position);

  // Near lower bound (0.1) should give negative scaled value
  EXPECT_LT(position[0], 0.0);
  // Near upper bound (0.9) should give positive scaled value
  EXPECT_GT(position[1], 0.0);
}

// ============================================================================
// Combined Leapfrog Integration Tests
// ============================================================================

/**
 * @brief Test a complete leapfrog step (half momentum + full position + half momentum)
 *
 * This tests the symplectic integrator property by performing one complete step
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Leapfrog_CompleteStep) {
  auto hmc = CreateHMCForTesting();

  // Initial conditions
  std::vector<double> position = {0.0, 0.0};
  std::vector<double> momentum = {1.0, 0.5};
  double              delta    = 0.1;

  // Gradient at origin (assume constant for this test)
  std::vector<double> gradient1 = {0.2, 0.1};

  // First half momentum step
  hmc->TestLeapfrogHalfMomentumStep(momentum, gradient1, delta);
  // p = [1.0 - 0.05*0.2, 0.5 - 0.05*0.1] = [0.99, 0.495]
  EXPECT_NEAR(momentum[0], 0.99, 1e-10);
  EXPECT_NEAR(momentum[1], 0.495, 1e-10);

  // Full position step
  hmc->TestLeapfrogFullPositionStep(position, momentum, delta);
  // q = [0 + 0.1*0.99, 0 + 0.1*0.495] = [0.099, 0.0495]
  EXPECT_NEAR(position[0], 0.099, 1e-10);
  EXPECT_NEAR(position[1], 0.0495, 1e-10);

  // Second gradient (would normally be re-evaluated; use same for simplicity)
  std::vector<double> gradient2 = {0.2, 0.1};

  // Second half momentum step
  hmc->TestLeapfrogHalfMomentumStep(momentum, gradient2, delta);
  // p = [0.99 - 0.05*0.2, 0.495 - 0.05*0.1] = [0.98, 0.49]
  EXPECT_NEAR(momentum[0], 0.98, 1e-10);
  EXPECT_NEAR(momentum[1], 0.49, 1e-10);
}

/**
 * @brief Test energy conservation property of leapfrog (approximate)
 *
 * For a simple harmonic oscillator H = 0.5*q^2 + 0.5*p^2,
 * the Hamiltonian should be approximately conserved after many leapfrog steps.
 * Here we just verify the kinetic + "potential" (norm2(q)/2) stays similar.
 */
TEST_F(HamiltonianMonteCarloThreadedModel, Leapfrog_EnergyConservation) {
  auto hmc = CreateHMCForTesting();

  // Simple harmonic oscillator: U(q) = 0.5 * ||q||^2, grad(U) = q
  std::vector<double> position = {1.0, 0.0};  // Start at (1, 0)
  std::vector<double> momentum = {0.0, 1.0};  // Moving in y direction
  double              delta    = 0.1;

  // Initial energy: U = 0.5 * 1 = 0.5, K = 0.5 * 1 = 0.5, H = 1.0
  auto calc_energy = [&hmc](const std::vector<double>& q, const std::vector<double>& p) {
    double U = 0.5 * hmc->TestNorm2(q);  // Simple harmonic potential
    double K = hmc->TestComputeKineticEnergy(p);
    return hmc->TestComputeHamiltonian(U, K);
  };

  double H_initial = calc_energy(position, momentum);
  EXPECT_NEAR(H_initial, 1.0, 1e-10);

  // Perform 10 leapfrog steps
  for (int step = 0; step < 10; ++step) {
    // For SHO, gradient = position
    std::vector<double> gradient = position;

    // Half momentum step
    hmc->TestLeapfrogHalfMomentumStep(momentum, gradient, delta);

    // Full position step
    hmc->TestLeapfrogFullPositionStep(position, momentum, delta);

    // Recompute gradient at new position
    gradient = position;

    // Second half momentum step
    hmc->TestLeapfrogHalfMomentumStep(momentum, gradient, delta);
  }

  double H_final = calc_energy(position, momentum);

  // Energy should be approximately conserved (small drift due to discretization)
  // For dt = 0.1, expect < 1% drift over 10 steps
  EXPECT_NEAR(H_final, H_initial, 0.01 * H_initial);
}

/**
 * @brief Test that sampleMomentum returns a vector of the correct size
 */
TEST_F(HamiltonianMonteCarloThreadedModel, SampleMomentum_CorrectSize) {
  auto hmc = CreateHMCForTesting();

  // Test different sizes
  std::vector<double> momentum3 = hmc->TestSampleMomentum(3);
  EXPECT_EQ(momentum3.size(), 3u);

  std::vector<double> momentum10 = hmc->TestSampleMomentum(10);
  EXPECT_EQ(momentum10.size(), 10u);

  std::vector<double> momentum1 = hmc->TestSampleMomentum(1);
  EXPECT_EQ(momentum1.size(), 1u);
}

/**
 * @brief Test that sampleMomentum returns values with reasonable statistical properties
 *        (not all zeros, varied values across multiple calls)
 */
TEST_F(HamiltonianMonteCarloThreadedModel, SampleMomentum_StatisticalProperties) {
  auto hmc = CreateHMCForTesting();

  // Sample multiple times and verify:
  // 1. Values are not all the same
  // 2. Different calls produce different results
  const size_t dim         = 5;
  const int    num_samples = 100;

  std::vector<std::vector<double>> samples;
  for (int i = 0; i < num_samples; ++i) {
    samples.push_back(hmc->TestSampleMomentum(dim));
  }

  // Check that not all values in a sample are identical
  bool has_variation_within = false;
  for (const auto& sample : samples) {
    for (size_t i = 1; i < sample.size(); ++i) {
      if (sample[i] != sample[0]) {
        has_variation_within = true;
        break;
      }
    }
    if (has_variation_within)
      break;
  }
  EXPECT_TRUE(has_variation_within) << "All momentum values within samples are identical";

  // Check that different samples are not identical
  bool has_variation_between = false;
  for (int i = 1; i < num_samples; ++i) {
    for (size_t j = 0; j < dim; ++j) {
      if (samples[i][j] != samples[0][j]) {
        has_variation_between = true;
        break;
      }
    }
    if (has_variation_between)
      break;
  }
  EXPECT_TRUE(has_variation_between) << "All samples are identical across calls";

  // Check mean is approximately 0 (with large tolerance due to RNG)
  double sum = 0.0;
  for (const auto& sample : samples) {
    for (double val : sample) {
      sum += val;
    }
  }
  double mean = sum / (num_samples * dim);
  EXPECT_NEAR(mean, 0.0, 0.5) << "Mean of samples deviates significantly from 0";
}

/**
 * @brief Test computeLogJacobian at the midpoint (q*=0) where formula simplifies
 *        log|dq/dq*| = log(range/PI) - log(1 + q*^2) = log(range/PI) when q*=0
 */
TEST_F(HamiltonianMonteCarloThreadedModel, ComputeLogJacobian_AtMidpoint) {
  auto hmc = CreateHMCForTesting();

  // Set up bounds: 2 parameters with ranges [0, 1] and [0, 2]
  hmc->SetBounds({0.0, 0.0}, {1.0, 2.0});

  // At q*=0 (midpoint of the bounded range):
  // log|dq/dq*| = log(range/PI) - log(1 + 0) = log(range/PI)
  // Expected: log(1/PI) + log(2/PI) = log(2/PI^2)
  std::vector<double> q_star_zeros = {0.0, 0.0};

  double log_jacobian = hmc->TestComputeLogJacobian(q_star_zeros);

  // Expected value: log(1/PI) + log(2/PI) = log(1) - log(PI) + log(2) - log(PI) = log(2) - 2*log(PI)
  double expected = std::log(1.0 / PI) + std::log(2.0 / PI);
  EXPECT_NEAR(log_jacobian, expected, 1e-10) << "Log Jacobian at midpoint should equal log(range/PI) summed";
}

/**
 * @brief Test computeLogJacobian symmetry: |q*| should give same result due to q*^2 term
 */
TEST_F(HamiltonianMonteCarloThreadedModel, ComputeLogJacobian_Symmetry) {
  auto hmc = CreateHMCForTesting();

  // Set up bounds: 2 parameters with ranges [0, 1] and [0, 2]
  hmc->SetBounds({0.0, 0.0}, {1.0, 2.0});

  // Test symmetry: log|dq/dq*| should be same for +q* and -q*
  // because the formula has q*^2
  std::vector<double> q_star_positive = {1.0, 0.5};
  std::vector<double> q_star_negative = {-1.0, -0.5};

  double log_jac_positive = hmc->TestComputeLogJacobian(q_star_positive);
  double log_jac_negative = hmc->TestComputeLogJacobian(q_star_negative);

  EXPECT_DOUBLE_EQ(log_jac_positive, log_jac_negative) << "Log Jacobian should be symmetric in q*";
}

/**
 * @brief Test computeLogJacobian decreases as |q*| increases (gets closer to bounds)
 */
TEST_F(HamiltonianMonteCarloThreadedModel, ComputeLogJacobian_DecreasesAtBounds) {
  auto hmc = CreateHMCForTesting();

  // Set up bounds: 2 parameters with ranges [0, 1] and [0, 2]
  hmc->SetBounds({0.0, 0.0}, {1.0, 2.0});

  // As |q*| increases (approaching bounds), log(1 + q*^2) increases,
  // so log|dq/dq*| = log(range/PI) - log(1 + q*^2) decreases
  std::vector<double> q_star_center = {0.0, 0.0};
  std::vector<double> q_star_mid    = {1.0, 1.0};
  std::vector<double> q_star_far    = {3.0, 3.0};

  double log_jac_center = hmc->TestComputeLogJacobian(q_star_center);
  double log_jac_mid    = hmc->TestComputeLogJacobian(q_star_mid);
  double log_jac_far    = hmc->TestComputeLogJacobian(q_star_far);

  EXPECT_GT(log_jac_center, log_jac_mid) << "Log Jacobian should decrease as |q*| increases";
  EXPECT_GT(log_jac_mid, log_jac_far) << "Log Jacobian should continue decreasing as |q*| increases further";
}

/**
 * @brief Test computeLogJacobian with empty input (edge case)
 */
TEST_F(HamiltonianMonteCarloThreadedModel, ComputeLogJacobian_EmptyInput) {
  auto hmc = CreateHMCForTesting();

  // Set up empty bounds
  hmc->SetBounds({}, {});

  // Verify the function handles empty input gracefully
  std::vector<double> empty_q_star  = {};
  double              log_jac_empty = hmc->TestComputeLogJacobian(empty_q_star);
  EXPECT_DOUBLE_EQ(log_jac_empty, 0.0) << "Empty q* should give log Jacobian of 0";
}

}  // namespace niwa
#endif  // USE_AUTODIFF
#endif  // TESTMODE
