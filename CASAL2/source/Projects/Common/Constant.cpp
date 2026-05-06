/**
 * @file Constant.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 28/05/2014
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "Constant.h"

#include "../../Utilities/Map.h"

// namespaces
namespace niwa {
namespace projects {

/**
 * Default constructor
 */
Constant::Constant(shared_ptr<Model> model) : Project(model) {
  parameters_.Bind<Double>(PARAM_VALUES, &values_, "The values to assign to the addressable");
  parameters_.Bind<Double>(PARAM_MULTIPLIER, &multiplier_, "Multiplier that is applied to the projected value")->set_is_optional(true);
}

/**
 * Validate
 */
void Constant::DoValidate() {
  parameters_.ValidateVector(PARAM_VALUES)->ExpandToSameNumberOfElementsAs(PARAM_YEARS)->SameNumberOfElementsAs(PARAM_YEARS);
  parameters_.ValidateVector(PARAM_MULTIPLIER)
      ->DefaultValue(1.0, years_.size())
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->GreaterThanOrEqualTo(0.0);
  year_values_        = utilities::Map::create(years_, values_);
  multiplier_by_year_ = utilities::Map::create(years_, multiplier_);
}

/**
 * Build
 */
void Constant::DoBuild() {}

/**
 * Reset
 */
void Constant::DoReset() {}

/**
 * Update
 */
void Constant::DoUpdate() {
  value_ = year_values_[model()->current_year()] * multiplier_by_year_[model()->current_year()];
  LOG_FINE() << "Setting Value to: " << value_ << ", with multiplier: " << multiplier_by_year_[model()->current_year()];
  (this->*DoUpdateFunc_)(value_, true, model()->current_year());
}

} /* namespace projects */
} /* namespace niwa */
