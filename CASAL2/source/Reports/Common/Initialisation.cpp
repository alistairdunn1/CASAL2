/**
 * @file Initialisation.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 12/10/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */
#include "Initialisation.h"

#include "../../InitialisationPhases/Age/Iterative.h"
#include "../../InitialisationPhases/InitialisationPhase.h"
#include "../../InitialisationPhases/Length/Iterative.h"
#include "../../InitialisationPhases/Manager.h"
#include "../../Model/Model.h"
#include "../../Partition/Accessors/All.h"

namespace niwa {
namespace reports {

/**
 *
 */
Initialisation::Initialisation() {
  run_mode_    = (RunMode::Type)(RunMode::kBasic | RunMode::kProjection);
  model_state_ = State::kInitialise;
  skip_tags_   = true;
}

/**
 * Execute the report
 */
void Initialisation::DoExecute(shared_ptr<Model> model) {
  LOG_TRACE();

  niwa::partition::accessors::All all_view(model);

  // Print the header
  // this report is slightly unique. Instead of the label it will use the name of the initialisation phase
  // The reason this was done was to deal with multi initialisation phases.
  label_          = model->get_current_initialisation_phase_label();
  initialisation_ = model->managers()->initialisation_phase()->GetInitPhase(label_);

  cache_ << ReportHeader(type_, label_, format_);
  cache_ << PARAM_TYPE << ": " << initialisation_->type() << REPORT_EOL;

  if (initialisation_->type() == PARAM_ITERATIVE) {
    years_             = initialisation_->GetConvergenceYears();
    lambda_            = initialisation_->GetTestConvergenceLambda();
    Double true_lambda = initialisation_->GetConvergenceLambda();
    cache_ << "values " << REPORT_R_DATAFRAME << REPORT_EOL;
    cache_ << PARAM_YEAR << " " << PARAM_LAMBDA << " " << PARAM_VALUE << " converged" << REPORT_EOL;
    for (unsigned i = 0; i < years_.size(); ++i) {
      if (lambda_[i] < 0.0)
        break;
      if (true_lambda <= lambda_[i])
        cache_ << years_[i] << " " << true_lambda << " " << lambda_[i] << " FALSE" << REPORT_EOL;
      else
        cache_ << years_[i] << " " << true_lambda << " " << lambda_[i] << " TRUE" << REPORT_EOL;
    }
  }
  cache_ << REPORT_END << REPORT_EOL;
  ready_for_writing_ = true;
}

void Initialisation::DoPrepareTabular(shared_ptr<Model> model) {
  cache_ << ReportHeader(type_, label_, format_);
  string marker = (report_sep_ == "\t") ? REPORT_R_DATAFRAME_TSV : REPORT_R_DATAFRAME;
  cache_ << "values " << marker << REPORT_EOL;
  cache_ << "initialisation_phase" << report_sep_ << PARAM_TYPE << report_sep_ << PARAM_YEAR << report_sep_ << PARAM_LAMBDA << report_sep_ << PARAM_VALUE << report_sep_
         << "converged" << REPORT_EOL;
}

void Initialisation::DoExecuteTabular(shared_ptr<Model> model) {
  label_                     = model->get_current_initialisation_phase_label();
  initialisation_            = model->managers()->initialisation_phase()->GetInitPhase(label_);
  string initialisation_type = initialisation_->type();

  if (initialisation_type == PARAM_ITERATIVE) {
    years_             = initialisation_->GetConvergenceYears();
    lambda_            = initialisation_->GetTestConvergenceLambda();
    Double true_lambda = initialisation_->GetConvergenceLambda();
    for (unsigned i = 0; i < years_.size(); ++i) {
      if (lambda_[i] < 0.0)
        break;
      cache_ << label_ << report_sep_ << initialisation_type << report_sep_ << years_[i] << report_sep_ << true_lambda << report_sep_ << lambda_[i] << report_sep_;
      if (true_lambda <= lambda_[i])
        cache_ << "FALSE";
      else
        cache_ << "TRUE";
      cache_ << REPORT_EOL;
    }
  } else {
    cache_ << label_ << report_sep_ << initialisation_type << report_sep_ << "NA" << report_sep_ << "NA" << report_sep_ << "NA" << report_sep_ << "NA" << REPORT_EOL;
  }
}

void Initialisation::DoFinaliseTabular(shared_ptr<Model> model) {
  ready_for_writing_ = true;
}

} /* namespace reports */
} /* namespace niwa */
