/*
 * Constant.cpp
 *
 *  Created on: 28/01/2015
 *      Author: Admin
 */

#include "Constant.h"

#include "../../Utilities/Map.h"

namespace niwa {
namespace timevarying {

/**
 * Default constructor
 */
Constant::Constant(shared_ptr<Model> model) : TimeVarying(model) {
  parameters_.Bind<Double>(PARAM_VALUES, &values_, "The value to assign to addressable");

  RegisterAsAddressable(PARAM_VALUES, &parameter_by_year_);
}

/**
 * Validate
 */
void Constant::DoValidate() {
  parameters_.ValidateVector(PARAM_VALUES)->ExpandToSameNumberOfElementsAs(PARAM_YEARS)->SameNumberOfElementsAs(PARAM_YEARS);

  parameter_by_year_ = utilities::Map::create(years_, values_);
}

/**
 * Update
 */
void Constant::DoUpdate() {
  auto current_model = model();
  LOG_FINE() << "Setting Value to: " << parameter_by_year_[current_model->current_year()];
  (this->*update_function_)(parameter_by_year_[current_model->current_year()]);
}

} /* namespace timevarying */
} /* namespace niwa */
