/**
 * @file AnnualShift.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 2/02/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "AnnualShift.h"

#include "../../Utilities/Map.h"

// namespaces
namespace niwa {
namespace timevarying {

/**
 * Default constructor
 */
AnnualShift::AnnualShift(shared_ptr<Model> model) : TimeVarying(model) {
  parameters_.Bind<Double>(PARAM_A, &a_, "Parameter A");
  parameters_.Bind<Double>(PARAM_B, &b_, "Parameter B");
  parameters_.Bind<Double>(PARAM_C, &c_, "Parameter C");
  parameters_.Bind<unsigned>(PARAM_SCALING_YEARS, &scaling_years_, "The scaling years")->set_is_optional(true);
  parameters_.Bind<Double>(PARAM_VALUES, &values_, "The values for each year");

  RegisterAsAddressable(PARAM_A, &a_);
  RegisterAsAddressable(PARAM_B, &b_);
  RegisterAsAddressable(PARAM_C, &c_);
  RegisterAsAddressable(PARAM_VALUES, &values_);
}

/**
 * Validate
 */
void AnnualShift::DoValidate() {
  parameters_.ValidateVector(PARAM_VALUES)->ExpandToSameNumberOfElementsAs(PARAM_YEARS)->SameNumberOfElementsAs(PARAM_YEARS);
  parameters_.ValidateVector(PARAM_SCALING_YEARS)->DuplicateParameterIfNotAssigned(PARAM_YEARS)->ExpandToSameNumberOfElementsAs(PARAM_YEARS)->SameNumberOfElementsAs(PARAM_YEARS);
}

/**
 * Build
 */
void AnnualShift::DoBuild() {
  map<unsigned, Double> values = utilities::Map::create(years_, values_);

  Double total = 0.0;
  for (unsigned scaling_year : scaling_years_) {
    total += values[scaling_year];
  }

  for (unsigned year : years_) {
    Double scaled_value   = values[year] - (total / scaling_years_.size());
    values_by_year_[year] = original_value_ + a_ * scaled_value + b_ * pow(scaled_value, 2) + c_ * pow(scaled_value, 3);
  }
}

/**
 * Reset
 */
void AnnualShift::DoReset() {}

/**
 * Update
 */
void AnnualShift::DoUpdate() {
  auto current_model = model();
  LOG_FINE() << "Setting Value to: " << values_by_year_[current_model->current_year()];
  (this->*update_function_)(values_by_year_[current_model->current_year()]);
  parameter_by_year_[current_model->current_year()] = values_by_year_[current_model->current_year()];
}

} /* namespace timevarying */
} /* namespace niwa */
