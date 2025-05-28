/**
 * @file AllValues.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 14/01/2013
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "AllValues.h"

#include <boost/math/distributions/lognormal.hpp>
#include <cmath>

#include "../../AgeLengths/AgeLength.h"
#include "../../Model/Model.h"

// Namespaces
namespace niwa {
namespace selectivities {

/**
 * Default Constructor
 */
AllValues::AllValues(shared_ptr<Model> model) : Selectivity(model) {
  parameters_.Bind<Double>(PARAM_V, &v_, "The v parameter");

  RegisterAsAddressable(PARAM_V, &v_);
  allowed_length_based_in_age_based_model_ = false;
}

/**
 * Validate this selectivity. This will load the
 * values that were passed in from the configuration
 * file and assign them to the local variables.
 *
 * Then do some basic checks on the local
 * variables to ensure they are within the business
 * rules for the model.
 */
void AllValues::DoValidate() {
  switch (model_->partition_type()) {
    case PartitionType::kAge:
      parameters_.ValidateVector(PARAM_V)->SameNumberOfElementsModelAgeSpread();
      break;

    case PartitionType::kLength:
      parameters_.ValidateVector(PARAM_V)->SameNumberOfElementsModelLengthBinMidPoints();
      break;

    default:
      LOG_CODE_ERROR() << "Unknown partition_type";
      break;
  }
}

/**
 * The core function
 */
Double AllValues::get_value(Double value) {
  if (model_->partition_type() == PartitionType::kLength) {
    return v_[model_->get_length_bin_ndx(value)];
  }
  LOG_CODE_ERROR() << "AllValues::get_value(Double value) value = " << value;
  return 1.0;
}

/**
 * The core function
 */
Double AllValues::get_value(unsigned value) {
  if (model_->partition_type() == PartitionType::kAge) {
    return v_[value - model_->min_age()];
  } else {
  }
  LOG_CODE_ERROR() << "AllValues::get_value(unsigned value) value = " << value;
  return 1.0;
}

} /* namespace selectivities */
} /* namespace niwa */
