/**
 * @file EmpiricalSampling.cpp
 * @author  Craig Marsh
 * @date 05/02/2016
 * @section LICENSE
 *
 * Copyright NIWA Science �2016 - www.niwa.co.nz
 *
 */

// headers
#include "EmpiricalSampling.h"

#include "../../Utilities/RandomNumberGenerator.h"
#include "../../Utilities/To.h"

// namespaces
namespace niwa {
namespace projects {

/**
 * Default constructor
 */
EmpiricalSampling::EmpiricalSampling(shared_ptr<Model> model) : Project(model) {
  parameters_.Bind<unsigned>(PARAM_START_YEAR, &start_year_, "The start year of sampling", "", false);
  parameters_.Bind<unsigned>(PARAM_FINAL_YEAR, &final_year_, "The final year of sampling", "", false);
  parameters_.Bind<Double>(PARAM_MULTIPLIER, &multiplier_, "Multiplier that is applied to the projected value", "", 1.0)->set_lower_bound(0, false);
}

/**
 * Validate
 */
void EmpiricalSampling::DoValidate() {
  // if no values specified then set default as the model lifespan
  if (!parameters_.Get(PARAM_START_YEAR)->has_been_defined())
    start_year_ = model_->start_year();
  if (!parameters_.Get(PARAM_FINAL_YEAR)->has_been_defined())
    final_year_ = model_->final_year();

  if (start_year_ < model_->start_year())
    LOG_ERROR_P(PARAM_START_YEAR) << start_year_ << " must be greater than or equal to the model start year " << model_->start_year();
  if (final_year_ > model_->final_year())
    LOG_ERROR_P(PARAM_FINAL_YEAR) << final_year_ << " must be less than or equal to the model final year " << model_->final_year();

  if (final_year_ <= start_year_)
    LOG_ERROR_P(PARAM_FINAL_YEAR) << final_year_ << " must be larger than start year " << start_year_;

  // if only one multiplier supplied then assume its the same for all years
  if (multiplier_.size() == 1) {
    multiplier_.resize(years_.size(), multiplier_[0]);
  }

  if (multiplier_.size() != 0) {
    if (multiplier_.size() != years_.size()) {
      LOG_FATAL_P(PARAM_MULTIPLIER) << "Supply a multiplier for each year. Values for " << multiplier_.size() << " years were provided, but " << years_.size()
                                    << " years are required";
    }
    multiplier_by_year_ = utilities::Map::create(years_, multiplier_);
  } else {
    Double val          = 1.0;
    multiplier_by_year_ = utilities::Map::create(years_, val);
  }
}

/**
 * Build
 */
void EmpiricalSampling::DoBuild() {}

/**
 * Reset
 */
void EmpiricalSampling::DoReset() {
  // Build a vector of years that have been resampled with replacement between start_year and end_year
  utilities::RandomNumberGenerator& rng         = utilities::RandomNumberGenerator::Instance();
  Double                            Random_draw = 0.0;
  unsigned                          year        = 0;
  for (unsigned project_year : years_) {
    Random_draw = floor(rng.uniform((double)start_year_, ((double)final_year_ + 0.99999)));
    year        = 0;
    // if (!utilities::To<Double, unsigned>(Random_draw, year))
    if (!utilities::To<Double>(Random_draw, year))
      LOG_ERROR() << "Random draw " << Random_draw << " could not be converted to Double";
    resampled_years_[project_year] = year;
    LOG_FINEST() << "Value from year: " << year << " used in projection year: " << project_year;
  }
}

/**
 *  Update the parameter with a random resample of the parameter between start_year_ and final_year_
 */
void EmpiricalSampling::DoUpdate() {
  LOG_FINE() << "In year: " << model_->current_year() << " setting value to: " << value_ << " drawn from year: " << resampled_years_[model_->current_year()]
             << ", with multiplier: " << multiplier_by_year_[model_->current_year()];
  value_ = stored_values_[resampled_years_[model_->current_year()]] * multiplier_by_year_[model_->current_year()];
  (this->*DoUpdateFunc_)(value_, true, model_->current_year());
}

} /* namespace projects */
} /* namespace niwa */
