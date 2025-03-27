/**
 * @file ObjectiveFunction.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 21/02/2013
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "ObjectiveFunction.h"

#include "ObjectiveFunction/ObjectiveFunction.h"

// Namespaces
namespace niwa::reports::test {

/**
 * Default constructor
 */
ObjectiveFunction::ObjectiveFunction() {
  model_state_ = State::kIterationComplete;
  run_mode_    = (RunMode::Type)(RunMode::kEstimation | RunMode::kBasic | RunMode::kProfiling | RunMode::kProjection);
}

/**
 *
 * Execute the report
 */
void ObjectiveFunction::DoExecute(shared_ptr<Model> model) {
  LOG_MEDIUM() << "Objective function report DoExecute";
  if (!model->is_primary_thread_model() && model->run_mode() == RunMode::kBasic)
    return;
  if (!model->is_primary_thread_model() && model->run_mode() == RunMode::kEstimation)
    return;

  ::niwa::ObjectiveFunction& obj_function = model->objective_function();
  obj_function.CalculateScore();
  scores_.push_back(obj_function.score());
}

}  // namespace niwa::reports::test
