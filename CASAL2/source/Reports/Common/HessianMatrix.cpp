/*
 * HessianMatrix.cpp
 *
 *  Created on: 4/09/2013
 *      Author: Admin
 */

#include "HessianMatrix.h"

#include "../../Minimisers/Manager.h"

namespace niwa {
namespace reports {
namespace ublas = boost::numeric::ublas;
/**
 * Default constructor
 */
HessianMatrix::HessianMatrix() {
  run_mode_    = (RunMode::Type)(RunMode::kEstimation | RunMode::kProfiling);
  model_state_ = State::kIterationComplete;
}

/**
 * If a minimiser pointer exists this report will ask and print for the hessian matrix
 */
void HessianMatrix::DoExecute(shared_ptr<Model> model) {
  /*
   * This reports the Hessian matrix
   */
  LOG_TRACE();
  auto minimiser_ = model->managers()->minimiser()->active_minimiser();
  if (!minimiser_)
    return;

  hessian_              = minimiser_->hessian_matrix();
  unsigned hessian_size = minimiser_->hessian_size();

  cache_ << ReportHeader(type_, label_, format_);
  cache_ << "hessian_matrix " << REPORT_R_MATRIX << REPORT_EOL;
  for (unsigned i = 0; i < hessian_size; ++i) {
    for (unsigned j = 0; j < hessian_size; ++j) {
      Double value = hessian_[i][j];
      cache_ << AS_DOUBLE(value) << " ";
    }
    cache_ << REPORT_EOL;
  }
  ready_for_writing_ = true;
}

void HessianMatrix::DoPrepareTabular(shared_ptr<Model> model) {
  LOG_INFO() << "Tabular mode for reports of type " << PARAM_HESSIAN_MATRIX << " has not been implemented";
}
} /* namespace reports */
} /* namespace niwa */
