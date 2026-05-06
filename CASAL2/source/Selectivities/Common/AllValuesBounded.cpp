/**
 * @file AllValuesBounded.cpp
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
#include "AllValuesBounded.h"

#include <boost/math/distributions/lognormal.hpp>
#include <cmath>

#include "../../AgeLengths/AgeLength.h"
#include "../../Model/Model.h"

// Namespaces
namespace niwa {
namespace selectivities {

/**
 * Default constructor
 */
AllValuesBounded::AllValuesBounded(shared_ptr<Model> model) : Selectivity(model) {
  parameters_.Bind<unsigned>(PARAM_L, &low_, "The low value (L)");
  parameters_.Bind<unsigned>(PARAM_H, &high_, "The high value (H)");
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
void AllValuesBounded::DoValidate() {
  if (model()->partition_type() == PartitionType::kAge) {
    unsigned min_age = model()->min_age();
    unsigned max_age = model()->max_age();

    parameters_.Validate(PARAM_L)->GreaterThanOrEqualTo(min_age);
    parameters_.Validate(PARAM_H)->LessThanOrEqualTo(max_age);
    parameters_.ValidateVector(PARAM_V)->NumberOfElements((high_ - low_) + 1)->GreaterThanOrEqualTo(0.0);

  } else if (model()->partition_type() == PartitionType::kLength) {
    vector<double> length_bins     = model()->length_bin_mid_points();
    unsigned       length_low_ndx  = model()->get_length_bin_ndx(low_);
    unsigned       length_high_ndx = model()->get_length_bin_ndx(high_);
    parameters_.ValidateVector(PARAM_V)->NumberOfElements((length_high_ndx - length_low_ndx) + 1)->GreaterThanOrEqualTo(0.0);
  }

  parameters_.Validate(PARAM_L)->LessThanParameter(PARAM_H);
  lower_length_bin_ = model()->get_length_bin_ndx(low_);
  LOG_FINE() << "lower_length_bin_ = " << lower_length_bin_;
}

/**
 * The core function
 */
Double AllValuesBounded::get_value(Double value) {
  if (value < low_) {
    return 0.0;
  } else if (value > high_) {
    return *v_.rbegin();
  } else {
    if (model()->partition_type() == PartitionType::kAge) {
      LOG_CODE_ERROR() << "model_->partition_type() == PartitionType::kAge";
    } else {
      // Length based a little more tricky
      unsigned len_ndx = model()->get_length_bin_ndx(value);
      LOG_FINE() << "len " << len_ndx;
      return v_[len_ndx - lower_length_bin_];
    }
  }
  LOG_CODE_ERROR() << "AllValuesBounded::get_value(Double value) value = " << value;
  return 1.0;
}

/**
 * The core function
 */
Double AllValuesBounded::get_value(unsigned value) {
  if (value < low_) {
    return 0.0;
  } else if (value > high_) {
    return *v_.rbegin();
  } else {
    if (model()->partition_type() == PartitionType::kAge) {
      return v_[value - low_];
    } else {
      // Length based a little more tricky
      LOG_CODE_ERROR() << "model_->partition_type() == PartitionType::kLength";
    }
  }
  LOG_CODE_ERROR() << "AllValuesBounded::get_value(unsigned value) value = " << value;
  return 1.0;
}
} /* namespace selectivities */
} /* namespace niwa */
