/**
 * @file Constant.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 28/05/2014
 * @section LICENSE
 *
 * Copyright NIWA Science �2014 - www.niwa.co.nz
 *
 */

// headers
#include "Constant.h"

// namespaces
namespace niwa {
namespace projects {

/**
 * Default constructor
 */
Constant::Constant(shared_ptr<Model> model) : Project(model) {
  parameters_.Bind<Double>(PARAM_VALUES, &values_, "The values to assign to the addressable", "");
  parameters_.Bind<Double>(PARAM_MULTIPLIER, &multiplier_, "Multiplier that is applied to the projected value", "", 1.0)->set_lower_bound(0, false);
}

/**
 * Validate
 */
void Constant::DoValidate() {
  if (values_.size() != 1 && values_.size() != years_.size()) {
    LOG_ERROR_P(PARAM_VALUES) << "length (" << values_.size() << ") must match the number of years provided (" << years_.size() << "), or use a single value for all years";
    return;
  }

  if (values_.size() == 1) {
    Double value = values_[0];
    values_.assign(years_.size(), value);
    LOG_FINEST() << "number of values converted from 1 to " << values_.size();
  }
  for (unsigned i = 0; i < years_.size(); ++i) {
    LOG_FINEST() << "value in year " << years_[i] << " = " << values_[i];
    year_values_[years_[i]] = values_[i];
  }

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
void Constant::DoBuild() {}

/**
 * Reset
 */
void Constant::DoReset() {}

/**
 * Update
 */
void Constant::DoUpdate() {
  value_ = year_values_[model_->current_year()] * multiplier_by_year_[model_->current_year()];
  LOG_FINE() << "Setting Value to: " << value_ << ", with multiplier: " << multiplier_by_year_[model_->current_year()];
  (this->*DoUpdateFunc_)(value_, true, model_->current_year());
}

} /* namespace projects */
} /* namespace niwa */
