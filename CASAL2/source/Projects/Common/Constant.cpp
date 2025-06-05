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
}

/**
 * Validate
 */
void Constant::DoValidate() {
  parameters_.ValidateVector(PARAM_VALUES)->ExpandToSameNumberOfElementsAs(PARAM_YEARS)->SameNumberOfElementsAs(PARAM_YEARS);
  year_values_ = utilities::Map::create(years_, values_);
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
  value_ = year_values_[model_->current_year()] * multiplier_;
  LOG_FINE() << "Setting Value to: " << value_;
  (this->*DoUpdateFunc_)(value_, true, model_->current_year());
}

} /* namespace projects */
} /* namespace niwa */
