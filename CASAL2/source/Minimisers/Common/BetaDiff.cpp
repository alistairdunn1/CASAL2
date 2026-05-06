/*
 * BetaDiff.cpp
 *
 *  Created on: 20/05/2013
 *      Author: Admin
 */
#ifdef USE_AUTODIFF
#ifdef USE_BETADIFF
#include "BetaDiff.h"

#include <betadiff.h>

#include "../../BaseClasses/Object.h"
#include "../../Estimates/Manager.h"
#include "../../Model/Model.h"
#include "../../ObjectiveFunction/ObjectiveFunction.h"

namespace niwa {
namespace minimisers {

/**
 * Objective Function
 */
class MyModel {};
class MyObjective {
public:
  MyObjective(shared_ptr<Model> model) : model_(model) {}
  shared_ptr<Model> model() const { return base::Object::LockWeakPtr(model_, "BetaDiff::MyObjective"); }

  Double operator()(const MyModel& model, const dvv& x_unbounded) {
    auto current_model = this->model();
    auto estimates     = current_model->managers()->estimate()->GetIsEstimated();

    for (int i = 0; i < x_unbounded.size(); ++i) {
      dvariable estimate = x_unbounded[i + 1];
      estimates[i]->set_value(estimate.x);
      LOG_MEDIUM() << estimates[i]->value() << " ";
    }

    current_model->FullIteration();
    ObjectiveFunction& objective = current_model->objective_function();
    objective.CalculateScore();
    return objective.score();
  }

private:
  weak_ptr<Model> model_;
};

/**
 * Default constructor
 */
BetaDiff::BetaDiff(shared_ptr<Model> model) : Minimiser(model) {
  parameters_.Bind<unsigned>(PARAM_MAX_ITERATIONS, &max_iterations_, "The maximum number of iterations")->set_default_value(1000);
  parameters_.Bind<unsigned>(PARAM_MAX_EVALUATIONS, &max_evaluations_, "The maximum number of evaluations")->set_default_value(4000);
  parameters_.Bind<double>(PARAM_TOLERANCE, &gradient_tolerance_, "The tolerance of the gradient for convergence")->set_default_value(1e-5);
}

/**
 *
 */
void BetaDiff::DoValidate() {
  parameters_.Validate(PARAM_MAX_ITERATIONS)->GreaterThan(1u);
  parameters_.Validate(PARAM_MAX_EVALUATIONS)->GreaterThan(1u);
  parameters_.Validate(PARAM_TOLERANCE)->GreaterThanOrEqualTo(0.0);
}

/**
 * Execute the minimiser
 */
void BetaDiff::Execute() {
  auto current_model    = model();
  auto estimate_manager = current_model->managers()->estimate();
  auto estimates        = estimate_manager->GetIsEstimated();

  dvector lower_bounds((int)estimates.size());
  dvector upper_bounds((int)estimates.size());
  dvector start_values((int)estimates.size());

  LOG_INFO() << "Estimation with the " << PARAM_BETADIFF << " minimiser";
  LOG_MEDIUM() << "estimated parameters";
  int i = 0;
  // Note betadiff uses dvector class which are indexed starting at 1 not 0 !!!
  for (auto estimate : estimates) {
    LOG_MEDIUM() << estimate->parameter();
    ++i;
    lower_bounds[i] = AS_DOUBLE(estimate->lower_bound());
    upper_bounds[i] = AS_DOUBLE(estimate->upper_bound());
    start_values[i] = AS_DOUBLE(estimate->value());

    if (estimate->value() < estimate->lower_bound()) {
      LOG_ERROR() << "The starting value for estimate " << estimate->parameter() << " (" << estimate->value() << ") was less than the lower bound (" << estimate->lower_bound()
                  << ")";
    } else if (estimate->value() > estimate->upper_bound()) {
      LOG_ERROR() << "The starting value for estimate " << estimate->parameter() << " (" << estimate->value() << ") was greater than the upper bound (" << estimate->upper_bound()
                  << ")";
    }
  }

  MyModel     my_model;
  MyObjective my_objective(current_model);

  dmatrix betadiff_hessian(estimates.size(), estimates.size());
  //  dmatrix adolc_hessian(estimates.size(), estimates.size());
  int    convergence     = 0;
  int    max_iterations  = (int)max_iterations_;
  int    max_evaluations = (int)max_evaluations_;
  double score           = optimise<MyModel, MyObjective>(my_model, my_objective, start_values, lower_bounds, upper_bounds, convergence, 0, max_iterations, max_evaluations,
                                                          gradient_tolerance_, 0, &betadiff_hessian, 0, 1);

  LOG_FINE() << "complete optimise, get hessian, " << hessian_size_;
  for (int row = 0; row < (int)estimates.size(); ++row) {
    for (int col = 0; col < (int)estimates.size(); ++col) {
      hessian_[row][col] = betadiff_hessian[row + 1][col + 1];
    }
  }

  LOG_MEDIUM() << "start values now. n_pars = " << start_values.size();
  for (int j = 1; j <= start_values.size(); ++j) {
    LOG_MEDIUM() << " start value " << start_values[j];
    estimated_values_.push_back(start_values[j]);
  }
  LOG_FINE() << "return convergence";

  // Note C.M
  // The convergence check is done at include/Betadiff.h line 1094
  // But the convergence gets + 2 at line 1167 and 1367
  // I have kept the result consistent with line 1094, but at a + 2 in the following case: so that it is consistent with the reported convergence
  switch (convergence) {
    case -3 + 2:
      result_ = MinimiserResult::kUnclearConvergence;
      break;
    case -2 + 2:
      result_ = MinimiserResult::kTooManyIterations;
      break;
    case -1 + 2:
      result_ = MinimiserResult::kSuccess;
      break;
    default:
      result_ = MinimiserResult::kError;
      break;
  }
}

}  // namespace minimisers
} /* namespace niwa */
#endif /* USE_BETADIFF */
#endif /* USE_AUTODIFF */
