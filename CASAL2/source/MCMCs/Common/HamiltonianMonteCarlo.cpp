/**
 * @file HamiltonianMonteCarlo.cpp
 * @author Scott Rasmussen (scott@zaita.com)
 * @brief
 * @version 0.1
 * @date 2021-04-24
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef USE_AUTODIFF

// headers
#include "HamiltonianMonteCarlo.h"

#include <boost/numeric/ublas/blas.hpp>

#include "../../Estimates/Manager.h"
#include "../../Minimisers/Manager.h"
#include "../../Model/Model.h"
#include "../../ObjectiveFunction/ObjectiveFunction.h"
#include "../../Reports/Manager.h"
#include "../../Utilities/Gradient.h"
#include "../../Utilities/Math.h"
#include "../../Utilities/RandomNumberGenerator.h"
#include "../../Utilities/Vector.h"

// namespaces
namespace niwa::mcmcs {

namespace math  = niwa::utilities::math;
namespace ublas = boost::numeric::ublas;
using math::PI;
using niwa::utilities::Vector_Add;
using niwa::utilities::Vector_Debug;
using niwa::utilities::Vector_Scale;

/**
 * @brief norm2. Return cumulative squard values of
 * the vector
 *
 * @param target
 * @return double
 */
double HamiltonianMonteCarlo::norm2(const std::vector<double>& target) {
  double result = 0.0;
  for (auto d : target) result += d * d;
  return result;
}

/**
 * @brief Calculate the log density of a multivariate normal distribution
 *
 * @param x The point at which to evaluate the density
 * @param mean The mean vector of the distribution
 * @param cov_l The Cholesky decomposition (lower triangular) of the covariance matrix
 * @param log_det The log determinant of the covariance matrix
 * @param constant The normalization constant (-n/2 * log(2*pi))
 * @return double The log density value
 */
double HamiltonianMonteCarlo::logDensity(const std::vector<double>& x, const std::vector<double>& mean, const std::vector<std::vector<double>>& cov_l, double log_det,
                                         double constant) {
  std::vector<double> diff(x.size());
  for (size_t i = 0; i < x.size(); ++i) {
    diff[i] = mean[i] - x[i];
  }

  // Forward solve: L * y = diff
  // This gives y = L^(-1) * diff
  // The Mahalanobis distance is diff^T * Σ^(-1) * diff = y^T * y = norm2(y)
  std::vector<double> y(diff.size(), 0.0);
  for (size_t i = 0; i < diff.size(); ++i) {
    double sum = diff[i];
    for (size_t j = 0; j < i; ++j) {
      sum -= cov_l[i][j] * y[j];
    }
    y[i] = sum / cov_l[i][i];
  }

  double result = constant - log_det - 0.5 * norm2(y);
  return result;
}

/**
 * @brief Compute the kinetic energy from momentum
 *
 * In HMC, kinetic energy is defined as K(p) = 0.5 * p^T * M^{-1} * p
 * With identity mass matrix (M = I), this simplifies to K(p) = 0.5 * ||p||^2
 *
 * @param momentum The momentum vector p
 * @return double The kinetic energy value
 */
double HamiltonianMonteCarlo::computeKineticEnergy(const std::vector<double>& momentum) {
  return 0.5 * norm2(momentum);
}

/**
 * @brief Compute the potential energy (negative log posterior)
 *
 * In HMC, potential energy U(q) corresponds to the negative log posterior.
 * For optimization problems, this is the objective function score.
 * This method sets the estimate values and evaluates the model.
 *
 * @param position The position vector q (parameter values)
 * @return double The potential energy value (objective score)
 */
double HamiltonianMonteCarlo::computePotentialEnergy(const std::vector<double>& position) {
  // Set estimate values to the given position
  for (unsigned i = 0; i < position.size(); ++i) {
    estimates_[i]->set_value(position[i]);
  }

  // Run model iteration and get objective score
  model()->FullIteration();
  model()->objective_function().CalculateScore();
  return model()->objective_function().score();
}

/**
 * @brief Compute the total Hamiltonian (total energy)
 *
 * The Hamiltonian H(q,p) = U(q) + K(p) represents total energy
 * and should be conserved during perfect leapfrog integration.
 *
 * @param potential_energy The potential energy U(q)
 * @param kinetic_energy The kinetic energy K(p)
 * @return double The total Hamiltonian value
 */
double HamiltonianMonteCarlo::computeHamiltonian(double potential_energy, double kinetic_energy) {
  return potential_energy + kinetic_energy;
}

/**
 * @brief Perform a half momentum update step of leapfrog integration
 *
 * Updates momentum using: p = p - (dt/2) * gradient(U)
 * The gradient points towards higher potential, so we subtract.
 *
 * @param momentum Momentum vector p (modified in place)
 * @param gradient Gradient of potential energy at current position
 * @param delta Time step size (full step, will be halved internally)
 */
void HamiltonianMonteCarlo::leapfrogHalfMomentumStep(std::vector<double>& momentum, const std::vector<double>& gradient, double delta) {
  double delta_half = delta / 2.0;
  for (size_t i = 0; i < momentum.size(); ++i) {
    momentum[i] = momentum[i] - delta_half * gradient[i];
  }
}

/**
 * @brief Perform the full position update step of leapfrog integration
 *
 * Updates position using: q = q + dt * p
 * Position should be in scaled (unbounded) space when calling this.
 *
 * @param position Position vector q in scaled space (modified in place)
 * @param momentum Momentum vector p
 * @param delta Time step size (leapfrog_delta_)
 */
void HamiltonianMonteCarlo::leapfrogFullPositionStep(std::vector<double>& position, const std::vector<double>& momentum, double delta) {
  for (size_t i = 0; i < position.size(); ++i) {
    position[i] = position[i] + delta * momentum[i];
  }
}

/**
 * @brief Compute the Metropolis-Hastings acceptance probability
 *
 * The acceptance probability for HMC is: alpha = min(1, exp(-deltaH))
 * where deltaH = H_proposed - H_current
 *
 * @param H_current Current (initial) Hamiltonian
 * @param H_proposed Proposed (final) Hamiltonian
 * @return double Acceptance probability in range [0, 1]
 */
double HamiltonianMonteCarlo::computeAcceptanceProbability(double H_current, double H_proposed) {
  double delta_H = H_proposed - H_current;
  return std::min(1.0, std::exp(-delta_H));
}

/**
 * @brief Sample fresh momentum from standard normal distribution N(0, I)
 *
 * In HMC, momentum must be independently sampled at the start of each
 * trajectory. This ensures detailed balance and proper exploration.
 *
 * @param size The dimension of the momentum vector
 * @return std::vector<double> Fresh momentum samples from N(0, I)
 */
std::vector<double> HamiltonianMonteCarlo::sampleMomentum(size_t size) {
  utilities::RandomNumberGenerator& rng = utilities::RandomNumberGenerator::Instance();
  std::vector<double>               momentum(size);
  for (size_t i = 0; i < size; ++i) {
    momentum[i] = rng.normal();
  }
  return momentum;
}

/**
 * @brief Compute the log Jacobian of the bounded-to-unbounded transformation
 *
 * When transforming from bounded [lower, upper] to unbounded (-inf, inf) space
 * using tan transformation, we need to correct the potential energy by the
 * log absolute Jacobian determinant.
 *
 * For the arctan transformation: q = ((atan(q*)/PI) + 0.5) * (max - min) + min
 * The Jacobian is: dq/dq* = (max - min) / (PI * (1 + q*^2))
 * Log Jacobian: log(max - min) - log(PI) - log(1 + q*^2)
 *
 * The sign is positive because we add this correction when computing
 * the transformed potential energy (we're working in q* space).
 *
 * @param position_unbounded Position in unbounded space q*
 * @return double The log Jacobian correction (sum over all dimensions)
 */
double HamiltonianMonteCarlo::computeLogJacobian(const std::vector<double>& position_unbounded) {
  double log_jacobian = 0.0;
  for (size_t i = 0; i < position_unbounded.size(); ++i) {
    double range     = estimate_upper_bounds_[i] - estimate_lower_bounds_[i];
    double q_star    = position_unbounded[i];
    double q_star_sq = q_star * q_star;
    // log|dq/dq*| = log(range/PI) - log(1 + q*^2)
    log_jacobian += std::log(range / PI) - std::log(1.0 + q_star_sq);
  }
  return log_jacobian;
}

/**
 * @brief Scale position values from bounded to unbounded space
 *
 * Uses arctan transformation to map [lower, upper] bounds to (-inf, inf)
 *
 * @param position Position vector to scale (modified in place)
 */
void HamiltonianMonteCarlo::scalePosition(std::vector<double>& position) {
  math::scale_vector(position, estimate_lower_bounds_, estimate_upper_bounds_);
}

/**
 * @brief Unscale position values from unbounded back to bounded space
 *
 * Uses inverse arctan transformation to map (-inf, inf) to [lower, upper] bounds
 *
 * @param position Position vector to unscale (modified in place)
 */
void HamiltonianMonteCarlo::unscalePosition(std::vector<double>& position) {
  math::unscale_vector(position, estimate_lower_bounds_, estimate_upper_bounds_);
}

/**
 * @brief Construct a new Hamiltonian Monte Carlo:: Hamiltonian Monte Carlo object
 *
 * @param model
 */
HamiltonianMonteCarlo::HamiltonianMonteCarlo(shared_ptr<Model> model) : MCMC(model) {
  parameters_.Bind<unsigned>(PARAM_LEAPFROG_STEPS, &leapfrog_steps_, "Number of leapfrog steps")->set_default_value(10u);
  parameters_.Bind<double>(PARAM_LEAPFROG_DELTA, &leapfrog_delta_, "Amount to leapfrog per step")->set_default_value(0.01);
  parameters_.Bind<double>(PARAM_GRADIENT_STEP_SIZE, &gradient_step_size_, "Step size to use when calculating gradient")->set_default_value(1e-6);

  type_ = PARAM_HAMILTONIAN;
}

/**
 *
 */
void HamiltonianMonteCarlo::DoValidate() {
  parameters_.Validate(PARAM_LEAPFROG_STEPS)->GreaterThan(0u);
  parameters_.Validate(PARAM_LEAPFROG_DELTA)->GreaterThan(0.0);
  parameters_.Validate(PARAM_GRADIENT_STEP_SIZE)->GreaterThan(1e-13);
}

/**
 * @brief
 *
 */
double scale(double value, double min, double max) {
  double scaled = tan(((value - min) / (max - min) - 0.5) * PI);
  return scaled;
}

double unscale(double value, double min, double max) {
  double unscaled = ((atan(value) / PI) + 0.5) * (max - min) + min;
  return unscaled;
}

/**
 * @brief Generate new candidates using the parent method then scale them so that
 * HMC can work entirely within scaled space when doing the leap frogs
 */
void HamiltonianMonteCarlo::GeneratedNewScaledCandidates() {
  GenerateNewCandidates();
  for (unsigned i = 0; i < candidates_.size(); ++i) {
    candidates_[i] = scale(candidates_[i], estimate_lower_bounds_[i], estimate_upper_bounds_[i]);
  }
}

/**
 * @brief Execute the HMC algorithm in single-threaded mode
 *
 * This method implements the Hamiltonian Monte Carlo algorithm without
 * using a thread pool. It uses model()->FullIteration() directly for
 * model evaluations.
 *
 * The algorithm works entirely in unbounded (transformed) space:
 * - Position q* is in unbounded space via tan transformation
 * - Momentum p is sampled fresh from N(0, I) each iteration
 * - Potential energy includes Jacobian correction for the transformation
 */
void HamiltonianMonteCarlo::DoExecuteSingleThreaded() {
  LOG_MEDIUM() << "Starting Hamiltonian Monte Carlo (Single Threaded)";
  ObjectiveFunction&                obj_function = model()->objective_function();
  utilities::RandomNumberGenerator& rng          = utilities::RandomNumberGenerator::Instance();

  vector<double> gradient(candidates_.size(), 0);

  // Initialize estimate bounds
  estimate_lower_bounds_.resize(estimates_.size(), 0.0);
  estimate_upper_bounds_.resize(estimates_.size(), 0.0);
  for (unsigned i = 0; i < estimates_.size(); ++i) {
    estimate_lower_bounds_[i] = estimates_[i]->lower_bound();
    estimate_upper_bounds_[i] = estimates_[i]->upper_bound();
  }

  // Lambda to run a model iteration and return the objective score
  auto run_model = [this]() {
    model()->FullIteration();
    model()->objective_function().CalculateScore();
    return model()->objective_function().score();
  };

  // Lambda to set estimate values (in bounded space) and run the model
  auto quick_run = [this, &run_model](const vector<double>& candidate_values) {
    for (unsigned i = 0; i < candidate_values.size(); ++i) {
      estimates_[i]->set_value(candidate_values[i]);
    }
    return run_model();
  };

  // Lambda to compute potential energy in unbounded space (includes Jacobian correction)
  // U*(q*) = U(h(q*)) - log|J| where h is the unscale transformation
  auto compute_potential_unbounded = [this, &quick_run](const vector<double>& q_star) {
    // Transform to bounded space
    vector<double> q_bounded = q_star;
    unscalePosition(q_bounded);

    // Get objective score in bounded space
    double U = quick_run(q_bounded);

    // Subtract log Jacobian (we want to sample from the correct transformed distribution)
    // The Jacobian correction accounts for the change of variables
    double log_jacobian = computeLogJacobian(q_star);

    // U* = U - log|J| (subtracting because we're working with negative log posterior)
    return U - log_jacobian;
  };

  // Lambda to calculate the gradient of potential energy in unbounded space
  auto calculate_gradient_unbounded = [this, &compute_potential_unbounded](const vector<double>& q_star, double current_potential) {
    vector<double> grad(q_star.size(), 0.0);

    for (unsigned i = 0; i < q_star.size(); ++i) {
      vector<double> q_perturbed = q_star;

      double step_size_temp = gradient_step_size_ * ((q_star[i] >= 0) ? 1.0 : -1.0);
      q_perturbed[i] += step_size_temp;

      double perturbed_potential = compute_potential_unbounded(q_perturbed);
      grad[i]                    = (perturbed_potential - current_potential) / step_size_temp;
    }

    return grad;
  };

  // Initialize the first chain link if not resuming
  if (chain_.empty()) {
    mcmc::ChainLink initial_link{.iteration_                   = 0,
                                 .mcmc_state_                  = PARAM_BURN_IN,
                                 .score_                       = obj_function.score(),
                                 .likelihood_                  = obj_function.likelihoods(),
                                 .prior_                       = obj_function.priors(),
                                 .penalty_                     = obj_function.penalties(),
                                 .additional_priors_           = obj_function.additional_priors(),
                                 .jacobians_                   = obj_function.jacobians(),
                                 .acceptance_rate_             = 0,
                                 .acceptance_rate_since_adapt_ = 0,
                                 .step_size_                   = step_size_,
                                 .values_                      = candidates_};
    chain_.push_back(initial_link);
  }

  // Main MCMC loop
  do {
    /**
     * @brief Hamiltonian Monte Carlo uses leapfrog integration to propose new states.
     *
     * The algorithm (working entirely in unbounded space):
     * 1. Transform current position to unbounded space: q* = scale(q)
     * 2. Sample fresh momentum p from N(0, I)
     * 3. Perform L leapfrog steps:
     *    a. p = p - (dt/2) * grad(U*)  [half momentum step]
     *    b. q* = q* + dt * p           [full position step]
     *    c. p = p - (dt/2) * grad(U*)  [half momentum step]
     * 4. Accept/reject based on Hamiltonian change
     * 5. Transform accepted position back to bounded space
     *
     * U* is the potential energy with Jacobian correction for the transformation.
     */

    // Get current position and transform to unbounded space
    vector<double> q0_bounded = chain_.rbegin()->values_;
    vector<double> q0_star    = q0_bounded;
    scalePosition(q0_star);

    // Working copies in unbounded space
    vector<double> q_star = q0_star;

    // Sample fresh momentum from N(0, I) - CRITICAL for correct HMC
    vector<double> p  = sampleMomentum(estimates_.size());
    vector<double> p0 = p;  // Store initial momentum for Hamiltonian calculation

    // Compute initial potential energy (with Jacobian correction)
    double U0 = compute_potential_unbounded(q0_star);

    // Leapfrog integration (all in unbounded space)
    for (unsigned step = 0; step < leapfrog_steps_; ++step) {
      // Compute potential and gradient at current position
      double current_potential = compute_potential_unbounded(q_star);
      gradient                 = calculate_gradient_unbounded(q_star, current_potential);

      // Half step for momentum: p = p - (dt/2) * grad(U*)
      leapfrogHalfMomentumStep(p, gradient, leapfrog_delta_);

      // Full step for position: q* = q* + dt * p
      leapfrogFullPositionStep(q_star, p, leapfrog_delta_);

      // Compute gradient at new position for second half step
      current_potential = compute_potential_unbounded(q_star);
      gradient          = calculate_gradient_unbounded(q_star, current_potential);

      // Half step for momentum: p = p - (dt/2) * grad(U*)
      leapfrogHalfMomentumStep(p, gradient, leapfrog_delta_);
    }

    // Calculate Hamiltonians for acceptance probability
    double U = compute_potential_unbounded(q_star);

    double K0 = computeKineticEnergy(p0);  // Initial kinetic energy
    double K  = computeKineticEnergy(p);   // Proposed kinetic energy

    double H0 = computeHamiltonian(U0, K0);  // Initial Hamiltonian
    double H  = computeHamiltonian(U, K);    // Proposed Hamiltonian

    // Metropolis acceptance criterion
    double accept_prob = computeAcceptanceProbability(H0, H);
    double rng_uniform = rng.uniform();

    jumps_++;

    double delta_H = H - H0;  // For logging purposes

    // Transform proposed position back to bounded space for storage
    vector<double> q_proposed = q_star;
    unscalePosition(q_proposed);

    if (rng_uniform < accept_prob) {
      // Accept the proposed state
      LOG_MEDIUM() << "Accepting Jump (iteration: " << jumps_ << ", accept_prob: " << accept_prob << ", rng: " << rng_uniform << ", delta_H: " << delta_H << ")";
      successful_jumps_++;

      // Update model with accepted values to get correct objective components
      quick_run(q_proposed);

      mcmc::ChainLink new_link{.iteration_                   = jumps_,
                               .mcmc_state_                  = (jumps_ > burn_in_) ? PARAM_MCMC : PARAM_BURN_IN,
                               .score_                       = obj_function.score(),
                               .likelihood_                  = obj_function.likelihoods(),
                               .prior_                       = obj_function.priors(),
                               .penalty_                     = obj_function.penalties(),
                               .additional_priors_           = obj_function.additional_priors(),
                               .jacobians_                   = obj_function.jacobians(),
                               .acceptance_rate_             = static_cast<double>(successful_jumps_) / static_cast<double>(jumps_),
                               .acceptance_rate_since_adapt_ = 0,
                               .step_size_                   = step_size_,
                               .values_                      = q_proposed};
      chain_.push_back(new_link);

    } else {
      // Reject the proposed state, keep the previous state
      LOG_MEDIUM() << "Rejecting Jump (iteration: " << jumps_ << ", accept_prob: " << accept_prob << ", rng: " << rng_uniform << ", delta_H: " << delta_H << ")";

      mcmc::ChainLink new_link  = *chain_.rbegin();
      new_link.iteration_       = jumps_;
      new_link.mcmc_state_      = (jumps_ > burn_in_) ? PARAM_MCMC : PARAM_BURN_IN;
      new_link.acceptance_rate_ = static_cast<double>(successful_jumps_) / static_cast<double>(jumps_);
      chain_.push_back(new_link);
    }

    // Report progress periodically
    if (jumps_ % keep_ == 0) {
      model()->managers()->report()->Execute(model(), State::kIterationComplete);
    }

  } while (chain_.size() < length_);
}

/**
 * @brief Execute the HMC algorithm using a threaded system
 *
 * This method implements HMC using a thread pool for parallel gradient computation.
 * The algorithm works entirely in unbounded (transformed) space:
 * - Position q* is in unbounded space via tan transformation
 * - Momentum p is sampled fresh from N(0, I) each iteration
 * - Potential energy includes Jacobian correction for the transformation
 *
 * @param thread_pool Pool of threads to use when computing gradients
 */
void HamiltonianMonteCarlo::DoExecute(shared_ptr<ThreadPool> thread_pool) {
  LOG_MEDIUM() << "Starting Hamiltonian Monte Carlo";
  ObjectiveFunction&                obj_function = model()->objective_function();
  utilities::RandomNumberGenerator& rng          = utilities::RandomNumberGenerator::Instance();
  vector<double>                    gradient(candidates_.size(), 0);

  estimate_lower_bounds_.resize(estimates_.size(), 0.0);
  estimate_upper_bounds_.resize(estimates_.size(), 0.0);
  for (unsigned i = 0; i < estimates_.size(); ++i) {
    estimate_lower_bounds_[i] = estimates_[i]->lower_bound();
    estimate_upper_bounds_[i] = estimates_[i]->upper_bound();
  }

  model()->FullIteration();

  // Lambda to set estimate values (in bounded space) and run the model
  auto quick_run = [this](const vector<double>& candidates) {
    for (unsigned i = 0; i < candidates.size(); ++i) estimates_[i]->set_value(candidates[i]);

    model()->FullIteration();
    model()->objective_function().CalculateScore();
    double score = model()->objective_function().score();
    return score;
  };

  // Lambda to compute potential energy in unbounded space (includes Jacobian correction)
  auto compute_potential_unbounded = [this, &quick_run](const vector<double>& q_star) {
    // Transform to bounded space
    vector<double> q_bounded = q_star;
    unscalePosition(q_bounded);

    // Get objective score in bounded space
    double U = quick_run(q_bounded);

    // Subtract log Jacobian for the transformation
    double log_jacobian = computeLogJacobian(q_star);
    return U - log_jacobian;
  };

  do {
    /**
     * @brief Hamiltonian Monte Carlo uses leapfrog integration in unbounded space.
     *
     * The algorithm:
     * 1. Transform current position to unbounded space: q* = scale(q)
     * 2. Sample fresh momentum p from N(0, I)
     * 3. Perform L leapfrog steps (all in unbounded space)
     * 4. Accept/reject based on Hamiltonian change
     * 5. Transform accepted position back to bounded space
     */

    // Get current position and transform to unbounded space
    vector<double> q0_bounded = chain_.rbegin()->values_;
    vector<double> q0_star    = q0_bounded;
    scalePosition(q0_star);

    // Working copy in unbounded space
    vector<double> q_star = q0_star;

    // Sample fresh momentum from N(0, I) - CRITICAL for correct HMC
    vector<double> p  = sampleMomentum(estimates_.size());
    vector<double> p0 = p;

    // Compute initial potential energy (with Jacobian correction)
    double U0 = compute_potential_unbounded(q0_star);

    // Leapfrog integration (all in unbounded space)
    for (unsigned i = 0; i < leapfrog_steps_; ++i) {
      // Transform to bounded space for gradient calculation (threaded gradient needs bounded values)
      vector<double> q_bounded = q_star;
      unscalePosition(q_bounded);

      // Compute raw objective score (WITHOUT Jacobian correction) for gradient calculation
      // The gradient calculator perturbs and runs models which return raw scores,
      // so we must use a raw score as the baseline for consistent gradient computation
      double raw_score = quick_run(q_bounded);
      gradient         = utilities::gradient::Calculate(thread_pool, q_bounded, estimate_lower_bounds_, estimate_upper_bounds_, gradient_step_size_, raw_score, false);

      // Half step for momentum
      leapfrogHalfMomentumStep(p, gradient, leapfrog_delta_);

      // Full step for position (in unbounded space)
      leapfrogFullPositionStep(q_star, p, leapfrog_delta_);

      // Transform again for second gradient
      q_bounded = q_star;
      unscalePosition(q_bounded);
      raw_score = quick_run(q_bounded);
      gradient  = utilities::gradient::Calculate(thread_pool, q_bounded, estimate_lower_bounds_, estimate_upper_bounds_, gradient_step_size_, raw_score, false);

      // Second half step for momentum
      leapfrogHalfMomentumStep(p, gradient, leapfrog_delta_);
    }

    // Calculate Hamiltonians for acceptance probability
    double U = compute_potential_unbounded(q_star);

    double K0 = computeKineticEnergy(p0);
    double K  = computeKineticEnergy(p);

    double H0 = computeHamiltonian(U0, K0);
    double H  = computeHamiltonian(U, K);

    // Metropolis acceptance criterion
    double accept_prob = computeAcceptanceProbability(H0, H);
    double rng_uniform = rng.uniform();
    double delta_H     = H - H0;

    jumps_++;

    // Transform proposed position back to bounded space
    vector<double> q_proposed = q_star;
    unscalePosition(q_proposed);

    // Check if we accept this jump
    if (rng_uniform < accept_prob) {
      LOG_MEDIUM() << "Accepting Jump (accept_prob: " << accept_prob << ", rng: " << rng_uniform << ", delta_H: " << delta_H << ")" << endl;
      successful_jumps_++;

      // Update model with accepted values to get correct objective components
      quick_run(q_proposed);

      mcmc::ChainLink new_link{.iteration_                   = jumps_,
                               .mcmc_state_                  = (jumps_ > burn_in_) ? PARAM_MCMC : PARAM_BURN_IN,
                               .score_                       = obj_function.score(),
                               .likelihood_                  = obj_function.likelihoods(),
                               .prior_                       = obj_function.priors(),
                               .penalty_                     = obj_function.penalties(),
                               .additional_priors_           = obj_function.additional_priors(),
                               .jacobians_                   = obj_function.jacobians(),
                               .acceptance_rate_             = static_cast<double>(successful_jumps_) / static_cast<double>(jumps_),
                               .acceptance_rate_since_adapt_ = 0,
                               .step_size_                   = step_size_,
                               .values_                      = q_proposed};
      chain_.push_back(new_link);

    } else {
      LOG_MEDIUM() << "Rejecting Jump (accept_prob: " << accept_prob << ", rng: " << rng_uniform << ", delta_H: " << delta_H << ")" << endl;
      // Copy the last chain accepted link to the end of the vector
      auto temp             = *chain_.rbegin();
      temp.iteration_       = jumps_;
      temp.acceptance_rate_ = static_cast<double>(successful_jumps_) / static_cast<double>(jumps_);
      if (jumps_ > burn_in_)
        temp.mcmc_state_ = PARAM_MCMC;
      else
        temp.mcmc_state_ = PARAM_BURN_IN;
      chain_.push_back(temp);
    }

  } while (chain_.size() != length_);
}

}  // namespace niwa::mcmcs
#endif  // USE_AUTODIFF
