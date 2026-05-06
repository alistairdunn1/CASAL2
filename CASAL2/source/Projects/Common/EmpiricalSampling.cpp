/**
 * @file EmpiricalSampling.cpp
 * @author  Craig Marsh
 * @date 05/02/2016
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
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
  parameters_.Bind<unsigned>(PARAM_START_YEAR, &start_year_, "The start year of sampling")->set_is_optional(true);
  parameters_.Bind<unsigned>(PARAM_FINAL_YEAR, &final_year_, "The final year of sampling")->set_is_optional(true);
  parameters_.Bind<Double>(PARAM_MULTIPLIER, &multiplier_, "Multiplier that is applied to the projected value")->set_is_optional(true);
}

/**
 * Validate
 */
void EmpiricalSampling::DoValidate() {
  parameters_.Validate(PARAM_START_YEAR)->IsModelYear()->DefaultValue(model()->start_year())->LessThanParameter(PARAM_FINAL_YEAR);
  parameters_.Validate(PARAM_FINAL_YEAR)->IsModelYear()->DefaultValue(model()->projection_final_year());
  parameters_.ValidateVector(PARAM_MULTIPLIER)
      ->DefaultValue(1.0, years_.size())
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->GreaterThanOrEqualTo(0.0);
  multiplier_by_year_ = utilities::Map::create(years_, multiplier_);
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
  LOG_FINE() << "In year: " << model()->current_year() << " setting value to: " << value_ << " drawn from year: " << resampled_years_[model()->current_year()]
             << ", with multiplier: " << multiplier_by_year_[model()->current_year()];
  value_ = stored_values_[resampled_years_[model()->current_year()]] * multiplier_by_year_[model()->current_year()];
  (this->*DoUpdateFunc_)(value_, true, model()->current_year());
}

} /* namespace projects */
} /* namespace niwa */
