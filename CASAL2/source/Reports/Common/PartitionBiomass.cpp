/**
 * @file PartitionBiomass.cpp
 * @author  C.Marsh
 * @version 1.0
 * @date 2022
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// Headers
#include "PartitionBiomass.h"

#include <iomanip>
#include <iostream>

#include "../../Model/Model.h"
#include "../../Partition/Accessors/All.h"
#include "../../TimeSteps/Manager.h"
#include "AgeLengths/AgeLength.h"
#include "GrowthIncrements/GrowthIncrement.h"

// Namespaces
namespace niwa {
namespace reports {

/**
 * Default constructor
 */
PartitionBiomass::PartitionBiomass() {
  run_mode_    = (RunMode::Type)(RunMode::kBasic | RunMode::kProjection);
  model_state_ = State::kExecute;
  skip_tags_   = true;

  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_, "Time Step label");
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "Years")->set_is_optional(true);
}

void PartitionBiomass::DoValidate(shared_ptr<Model> model) {
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
}
/**
 * Execute the report
 */
void PartitionBiomass::DoExecute(shared_ptr<Model> model) {
  LOG_TRACE();
  // First, figure out the lowest and highest ages/length
  niwa::partition::accessors::All all_view(model);
  unsigned                        time_step_index = model->managers()->time_step()->current_time_step();

  // Print the header
  cache_ << ReportHeader(type_, label_, format_);
  cache_ << "year: " << model->current_year() << REPORT_EOL;
  cache_ << "time_step: " << time_step_ << REPORT_EOL;
  cache_ << "values " << REPORT_R_DATAFRAME_ROW_LABELS << REPORT_EOL;
  cache_ << "category";

  if (model->partition_type() == PartitionType::kAge) {
    for (unsigned i = model->min_age(); i <= model->max_age(); ++i) cache_ << " " << i;
    cache_ << REPORT_EOL;
    for (auto iterator : all_view) {
      cache_ << iterator->name_;
      unsigned age          = model->min_age();
      Double   weight_value = 0.0;
      for (auto value : iterator->data_) {
        weight_value = value * iterator->age_length_->mean_weight(time_step_index, age);
        cache_ << " " << std::fixed << AS_DOUBLE(weight_value);
        age++;
      }
      cache_ << REPORT_EOL;
    }
  } else if (model->partition_type() == PartitionType::kLength) {
    for (auto len_bin : model->length_bin_mid_points()) cache_ << " " << len_bin;
    cache_ << REPORT_EOL;
    for (auto iterator : all_view) {
      cache_ << iterator->name_;
      unsigned length_ndx   = 0;
      Double   weight_value = 0.0;
      for (auto value : iterator->data_) {
        weight_value = value * iterator->growth_increment_->get_mean_weight(length_ndx);
        cache_ << " " << std::fixed << AS_DOUBLE(weight_value);
        length_ndx++;
      }
      cache_ << REPORT_EOL;
    }
  }
  cache_ << REPORT_END << REPORT_EOL;
  ready_for_writing_ = true;
}

void PartitionBiomass::DoPrepareTabular(shared_ptr<Model> model) {
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

void PartitionBiomass::DoExecuteTabular(shared_ptr<Model> model) {
  niwa::partition::accessors::All all_view(model);
  unsigned                        time_step_index = model->managers()->time_step()->current_time_step();

  if (model->partition_type() == PartitionType::kAge) {
    for (auto iterator : all_view) {
      cache_ << model->current_year() << report_sep_ << time_step_ << report_sep_ << iterator->name_;
      unsigned age          = model->min_age();
      Double   weight_value = 0.0;
      for (auto value : iterator->data_) {
        weight_value = value * iterator->age_length_->mean_weight(time_step_index, age);
        cache_ << report_sep_ << std::fixed << AS_DOUBLE(weight_value);
        age++;
      }
      cache_ << REPORT_EOL;
    }
  } else if (model->partition_type() == PartitionType::kLength) {
    for (auto iterator : all_view) {
      cache_ << model->current_year() << report_sep_ << time_step_ << report_sep_ << iterator->name_;
      unsigned length_ndx   = 0;
      Double   weight_value = 0.0;
      for (auto value : iterator->data_) {
        weight_value = value * iterator->growth_increment_->get_mean_weight(length_ndx);
        cache_ << report_sep_ << std::fixed << AS_DOUBLE(weight_value);
        length_ndx++;
      }
      cache_ << REPORT_EOL;
    }
  }
}

void PartitionBiomass::DoFinaliseTabular(shared_ptr<Model> model) {
  ready_for_writing_ = true;
}

} /* namespace reports */
} /* namespace niwa */
