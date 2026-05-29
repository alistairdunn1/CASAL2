/**
 * @file TimeVarying.cpp
 * @author  C. Marsh
 * @date 01/06/2016
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

#include "TimeVarying.h"

#include "../../TimeVarying/Manager.h"

namespace niwa {
namespace reports {

/**
 * Default constructor
 */
TimeVarying::TimeVarying() {
  run_mode_    = (RunMode::Type)(RunMode::kBasic | RunMode::kProjection | RunMode::kSimulation | RunMode::kEstimation | RunMode::kProfiling);
  model_state_ = (State::Type)(State::kIterationComplete);

  parameters_.Bind<string>(PARAM_TIME_VARYING, &time_varying_label_, "The time varying label that is reported")->set_is_optional(true);
}

/**
 * Validate object
 */
void TimeVarying::DoValidate(shared_ptr<Model> model) {
  if (!model->global_configuration().print_tabular()) {
    parameters_.Validate(PARAM_TIME_VARYING)->DuplicateParameterIfNotAssigned(PARAM_LABEL);
  }
}

/**
 *  Build object
 */
void TimeVarying::DoBuild(shared_ptr<Model> model) {
  // Outside tabular mode we retain legacy behaviour by inferring the time-varying label from the report label.
  if (!model->global_configuration().print_tabular() && time_varying_label_ == "") {
    time_varying_label_ = label_;
  }

  if (time_varying_label_ != "") {
    timevarying_ = model->managers()->time_varying()->GetTimeVarying(time_varying_label_);
  }

  if (time_varying_label_ != "" && !timevarying_) {
#ifndef TESTMODE
    LOG_WARNING() << "The " << PARAM_TIME_VARYING << " report with label '" << time_varying_label_ << "' was requested. This " << PARAM_TIME_VARYING
                  << " was not found in the input configuration file and the report will not be generated";
#endif
    is_valid_ = false;
  }
}

/**
 * Execute the report
 */
void TimeVarying::DoExecute(shared_ptr<Model> model) {
  LOG_TRACE();
  // single valid time-varying report value
  if (is_valid_) {
    cache_ << ReportHeader(type_, label_, format_);
    LOG_FINEST() << "Reporting for @time_varying block " << time_varying_label_;
    cache_ << "values " << REPORT_R_DATAFRAME << REPORT_EOL;

    map<unsigned, Double>& parameter_by_year = timevarying_->get_parameter_by_year();
    cache_ << "year"
           << " Value" << REPORT_EOL;
    for (auto param : parameter_by_year) {
      cache_ << param.first << "  " << AS_DOUBLE(param.second) << REPORT_EOL;
    }
    ready_for_writing_ = true;
  }
}
/**
 * Prepare Tabular report write header output
 */
void TimeVarying::DoPrepareTabular(shared_ptr<Model> model) {
  if (!is_valid_)
    return;

  cache_ << ReportHeader(type_, label_, format_);
  string marker = (report_sep_ == "\t") ? REPORT_R_DATAFRAME_TSV : REPORT_R_DATAFRAME;
  cache_ << "values " << marker << REPORT_EOL;

  bool first_col = true;
  if (time_varying_label_ != "") {
    map<unsigned, Double>& parameter_by_year = timevarying_->get_parameter_by_year();
    for (const auto& param : parameter_by_year) {
      if (!first_col)
        cache_ << report_sep_;
      first_col = false;
      cache_ << "time_varying[" << timevarying_->label() << "][" << param.first << "]";
    }
  } else {
    auto& time_varying_manager = *model->managers()->time_varying();
    for (auto* time_varying : time_varying_manager.objects()) {
      map<unsigned, Double>& parameter_by_year = time_varying->get_parameter_by_year();
      for (const auto& param : parameter_by_year) {
        if (!first_col)
          cache_ << report_sep_;
        first_col = false;
        cache_ << "time_varying[" << time_varying->label() << "][" << param.first << "]";
      }
    }
  }
  cache_ << REPORT_EOL;
}

/**
 * Execute Tabular report
 */
void TimeVarying::DoExecuteTabular(shared_ptr<Model> model) {
  if (!is_valid_)
    return;

  bool first_col = true;
  if (time_varying_label_ != "") {
    map<unsigned, Double>& parameter_by_year = timevarying_->get_parameter_by_year();
    for (auto& param : parameter_by_year) {
      if (!first_col)
        cache_ << report_sep_;
      first_col = false;
      cache_ << AS_DOUBLE(param.second);
    }
  } else {
    auto& time_varying_manager = *model->managers()->time_varying();
    for (auto* time_varying : time_varying_manager.objects()) {
      map<unsigned, Double>& parameter_by_year = time_varying->get_parameter_by_year();
      for (auto& param : parameter_by_year) {
        if (!first_col)
          cache_ << report_sep_;
        first_col = false;
        cache_ << AS_DOUBLE(param.second);
      }
    }
  }
  cache_ << REPORT_EOL;
}

/**
 *
 */
void TimeVarying::DoFinaliseTabular(shared_ptr<Model> model) {
  if (!is_valid_)
    return;

  ready_for_writing_ = true;
}

} /* namespace reports */
} /* namespace niwa */
