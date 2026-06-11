/**
 * @file InitialisationPartition.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 25/08/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "InitialisationPartition.h"

#include <iomanip>
#include <iostream>

#include "../../Model/Model.h"
#include "../../Partition/Accessors/All.h"

// namespaces
namespace niwa {
namespace reports {

/**
 * Default constructor
 *
 * Set the run mode and model state for this report
 */
InitialisationPartition::InitialisationPartition() {
  run_mode_    = (RunMode::Type)(RunMode::kBasic | RunMode::kProjection);
  model_state_ = State::kInitialise;
  skip_tags_   = true;
}

/**
 * Execute the report
 */
void InitialisationPartition::DoExecute(shared_ptr<Model> model) {
  LOG_TRACE();
  // First, figure out the lowest and highest ages/length

  niwa::partition::accessors::All all_view(model);

  // Print the header
  // Note: uses the initialisation phase name rather than the @report label so that
  // each phase produces a uniquely-keyed block when there are multiple phases.
  cache_ << ReportHeader(type_, model->get_current_initialisation_phase_label(), format_);
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

void InitialisationPartition::DoPrepareTabular(shared_ptr<Model> model) {
  cache_ << ReportHeader(type_, label_, format_);
  string marker = (report_sep_ == "\t") ? REPORT_R_DATAFRAME_TSV : REPORT_R_DATAFRAME;
  cache_ << "values " << marker << REPORT_EOL;

  cache_ << "initialisation_phase" << report_sep_ << "category";
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

void InitialisationPartition::DoExecuteTabular(shared_ptr<Model> model) {
  niwa::partition::accessors::All all_view(model);
  string                          phase_label = model->get_current_initialisation_phase_label();

  for (auto iterator : all_view) {
    cache_ << phase_label << report_sep_ << iterator->name_;
    for (auto value : iterator->data_) {
      cache_ << report_sep_ << std::fixed << AS_DOUBLE(value);
    }
    cache_ << REPORT_EOL;
  }
}

void InitialisationPartition::DoFinaliseTabular(shared_ptr<Model> model) {
  ready_for_writing_ = true;
}

} /* namespace reports */
} /* namespace niwa */
