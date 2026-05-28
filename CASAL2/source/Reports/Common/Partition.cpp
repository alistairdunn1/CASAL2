/**
 * @file Partition.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 13/02/2013
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "Partition.h"

#include <iomanip>
#include <iostream>

#include "../../Model/Model.h"
#include "../../Partition/Accessors/All.h"

// Namespaces
namespace niwa {
namespace reports {

/**
 * Default constructor
 */
Partition::Partition() {
  run_mode_    = (RunMode::Type)(RunMode::kBasic | RunMode::kProjection);
  model_state_ = State::kExecute;
  skip_tags_   = true;

  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_, "Time Step label");
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "Years")->set_is_optional(true);
}

void Partition::DoValidate(shared_ptr<Model> model) {
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
}
/**
 * Execute the report
 */
void Partition::DoExecute(shared_ptr<Model> model) {
  LOG_TRACE();
  // First, figure out the lowest and highest ages/length
  niwa::partition::accessors::All all_view(model);

  // Print the header
  cache_ << ReportHeader(type_, label_, format_);
  cache_ << "year: " << model->current_year() << REPORT_EOL;
  cache_ << "time_step: " << time_step_ << REPORT_EOL;
  cache_ << "values " << REPORT_R_DATAFRAME_ROW_LABELS << REPORT_EOL;
  cache_ << "category";
  if (model->partition_type() == PartitionType::kAge) {
    for (unsigned i = model->min_age(); i <= model->max_age(); ++i) cache_ << " " << i;
    cache_ << REPORT_EOL;
  } else if (model->partition_type() == PartitionType::kLength) {
    for (auto len_bin : model->length_bin_mid_points()) cache_ << " " << len_bin;
    cache_ << REPORT_EOL;
  }

  for (auto iterator : all_view) {
    cache_ << iterator->name_;
    for (auto value : iterator->data_) {
      cache_ << " " << std::fixed << AS_DOUBLE(value);
    }
    cache_ << REPORT_EOL;
  }
  cache_ << REPORT_END << REPORT_EOL;

  ready_for_writing_ = true;
}

void Partition::DoPrepareTabular(shared_ptr<Model> model) {
  cache_ << ReportHeader(type_, label_, format_);
  string marker = (report_sep_ == "\t") ? REPORT_R_DATAFRAME_TSV : REPORT_R_DATAFRAME;
  cache_ << "values " << marker << REPORT_EOL;

  cache_ << "year" << report_sep_ << "time_step" << report_sep_ << "category";
  if (model->partition_type() == PartitionType::kAge) {
    for (unsigned i = model->min_age(); i <= model->max_age(); ++i) {
      cache_ << report_sep_ << i;
    }
  } else if (model->partition_type() == PartitionType::kLength) {
    for (auto len_bin : model->length_bin_mid_points()) {
      cache_ << report_sep_ << len_bin;
    }
  }
  cache_ << REPORT_EOL;
}

void Partition::DoExecuteTabular(shared_ptr<Model> model) {
  niwa::partition::accessors::All all_view(model);

  for (auto iterator : all_view) {
    cache_ << model->current_year() << report_sep_ << time_step_ << report_sep_ << iterator->name_;
    for (auto value : iterator->data_) {
      cache_ << report_sep_ << std::fixed << AS_DOUBLE(value);
    }
    cache_ << REPORT_EOL;
  }
}

void Partition::DoFinaliseTabular(shared_ptr<Model> model) {
  ready_for_writing_ = true;
}

} /* namespace reports */
} /* namespace niwa */
