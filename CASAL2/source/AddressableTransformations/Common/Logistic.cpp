/**
 * @file Logistic.cpp
 * @author C.Marsh
 * @github https://github.com/Craig44
 * @date 2022
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "Logistic.h"

#include "../../Estimates/Estimate.h"
#include "../../Estimates/Manager.h"
#include "../../Model/Managers.h"
#include "../../Model/Model.h"
#include "../../Model/Objects.h"
#include "../../Utilities/Math.h"

// namespaces
namespace niwa {
namespace addressabletransformations {

/**
 * Default constructor
 */
Logistic::Logistic(shared_ptr<Model> model) : AddressableTransformation(model) {
  parameters_.Bind<Double>(PARAM_LOWER_BOUND, &lower_bounds_, "Lower bounds of parameter space (one value or one per parameter)", "", true);
  parameters_.Bind<Double>(PARAM_UPPER_BOUND, &upper_bounds_, "Upper bounds of parameter space (one value or one per parameter)", "", true);
  RegisterAsAddressable(PARAM_LOGISTIC_PARAMETER, &logistic_values_);
}

/**
 * Validate
 */
void Logistic::DoValidate() {
  // Broadcast single bound value to all parameters, or validate length matches
  if (lower_bounds_.size() == 1)
    lower_bounds_.resize(n_params_, lower_bounds_[0]);
  else if (lower_bounds_.size() != n_params_)
    LOG_ERROR_P(PARAM_LOWER_BOUND) << "Expected 1 or " << n_params_ << " values but got " << lower_bounds_.size();
  if (upper_bounds_.size() == 1)
    upper_bounds_.resize(n_params_, upper_bounds_[0]);
  else if (upper_bounds_.size() != n_params_)
    LOG_ERROR_P(PARAM_UPPER_BOUND) << "Expected 1 or " << n_params_ << " values but got " << upper_bounds_.size();

  restored_values_.resize(n_params_, 0.0);
  logistic_values_.resize(n_params_, 0.0);
  for (unsigned i = 0; i < n_params_; ++i) {
    if (lower_bounds_[i] >= upper_bounds_[i])
      LOG_ERROR_P(PARAM_LOWER_BOUND) << "lower_bound [" << i << "] (" << lower_bounds_[i] << ") must be less than upper_bound (" << upper_bounds_[i] << ")";
    if (init_values_[i] == lower_bounds_[i])
      LOG_ERROR_P(PARAM_PARAMETERS) << "initial value [" << i << "] was equal to lower bound. This will cause an Inf and is not allowed. Change starting value";
    if (init_values_[i] == upper_bounds_[i])
      LOG_ERROR_P(PARAM_PARAMETERS) << "initial value [" << i << "] was equal to upper bound. This will cause an Inf and is not allowed. Change starting value";
    logistic_values_[i] = utilities::math::logit_bounds(init_values_[i], lower_bounds_[i], upper_bounds_[i]);  // this will get over-ridden by load estimables
    restored_values_[i] = utilities::math::invlogit_bounds(logistic_values_[i], lower_bounds_[i], upper_bounds_[i]);
    if (restored_values_[i] != init_values_[i]) {
      LOG_FINE() << "i = " << i << " restored val " << restored_values_[i] << " init value = " << init_values_[i];
    }
  }
}

/**
 * Build
 */
void Logistic::DoBuild() {}

/**
 * Restore
 */
void Logistic::DoRestore() {
  for (unsigned i = 0; i < n_params_; ++i) {
    restored_values_[i] = utilities::math::invlogit_bounds(logistic_values_[i], lower_bounds_[i], upper_bounds_[i]);
    LOG_FINE() << "Setting Value[" << i << "] to: " << restored_values_[i];
  }
  (this->*restore_function_)(restored_values_);
}

/**
 * Get Score
 * @return -log(Jacobian) if transformed with Jacobian, otherwise 0.0
 */
Double Logistic::GetScore() {
  LOG_TRACE()
  if (prior_applies_to_restored_parameters_) {
    jacobian_ = 0.0;
    for (unsigned i = 0; i < n_params_; ++i) {
      Double sigma_y = utilities::math::invlogit(logistic_values_[i]);
      jacobian_ -= log((upper_bounds_[i] - lower_bounds_[i]) * sigma_y * (1.0 - sigma_y));
    }
  }
  return jacobian_;
}
/**
 * PrepareForObjectiveFunction
 * if prior_applies_to_restored_parameters_ then set log_value_ = exp(log_value_)
 */
void Logistic::PrepareForObjectiveFunction() {
  if (prior_applies_to_restored_parameters_) {
    for (unsigned i = 0; i < n_params_; ++i)
      logistic_values_[i] = utilities::math::invlogit_bounds(logistic_values_[i], lower_bounds_[i], upper_bounds_[i]);  // this will get over-riden by load estimables
  }
}

/**
 * RestoreForObjectiveFunction
 * if prior_applies_to_restored_parameters_ then set log_value_ = log(log_value_)
 */
void Logistic::RestoreForObjectiveFunction() {
  if (prior_applies_to_restored_parameters_) {
    for (unsigned i = 0; i < n_params_; ++i)
      logistic_values_[i] = utilities::math::logit_bounds(logistic_values_[i], lower_bounds_[i], upper_bounds_[i]);  // this will get over-riden by load estimables
  }
}

/**
 * Report stuff for this transformation
 */
void Logistic::FillReportCache(ostringstream& cache) {
  LOG_FINE() << "FillReportCache";
  cache << PARAM_PARAMETERS << ": ";
  for (unsigned i = 0; i < parameter_labels_.size(); ++i) cache << parameter_labels_[i] << " ";
  cache << REPORT_EOL;
  cache << "parameter_values: ";
  for (unsigned i = 0; i < restored_values_.size(); ++i) cache << restored_values_[i] << " ";
  cache << REPORT_EOL;
  cache << PARAM_LOWER_BOUND << ": ";
  for (unsigned i = 0; i < lower_bounds_.size(); ++i) cache << lower_bounds_[i] << " ";
  cache << REPORT_EOL;
  cache << PARAM_UPPER_BOUND << ": ";
  for (unsigned i = 0; i < upper_bounds_.size(); ++i) cache << upper_bounds_[i] << " ";
  cache << REPORT_EOL;
  cache << PARAM_LOGISTIC_PARAMETER << ": ";
  for (unsigned i = 0; i < logistic_values_.size(); ++i) cache << logistic_values_[i] << " ";
  cache << REPORT_EOL;
  cache << "negative_log_jacobian: " << jacobian_ << REPORT_EOL;
}

/**
 * Report stuff for this transformation
 */
void Logistic::FillTabularReportCache(ostringstream& cache, bool first_run, const string& sep) {
  LOG_FINEST() << "FillTabularReportCache";
  if (first_run) {
    for (unsigned i = 0; i < logistic_values_.size(); ++i) cache << PARAM_LOGISTIC_PARAMETER << "{" << (i + 1) << "}" << sep;
    for (unsigned i = 0; i < parameter_labels_.size(); ++i) cache << parameter_labels_[i] << sep;
    cache << "negative_log_jacobian" << REPORT_EOL;
  }
  for (unsigned i = 0; i < logistic_values_.size(); ++i) cache << logistic_values_[i] << sep;
  for (unsigned i = 0; i < restored_values_.size(); ++i) cache << restored_values_[i] << sep;
  cache << jacobian_ << REPORT_EOL;
}

} /* namespace addressabletransformations */
} /* namespace niwa */
