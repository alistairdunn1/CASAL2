/**
 * @file OffByOne.cpp
 * @author C Marsh
 * @github https://github.com/Zaita
 * @date 01/09/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "OffByOne.h"

// namespaces
namespace niwa {
namespace ageingerrors {

// clang-format off
// Constructor
OffByOne::OffByOne(shared_ptr<Model> model) : AgeingError(model) {
  parameters_.Bind<Double>(PARAM_P1, &p1_, "The proportion misclassified as one year younger, e.g., the proportion of age k individuals that were misclassified as age (k-1)");
  parameters_.Bind<Double>(PARAM_P2, &p2_, "The proportion misclassified as one year older, e.g., the proportion of age k individuals that were misclassified as age (k+1))");
  parameters_.Bind<unsigned>(PARAM_K, &k_, "The minimum age of animals which can be misclassified, i.e., animals of age less than k are assumed to be correctly classified")->set_default_value(0u);
  RegisterAsAddressable(PARAM_P1, &p1_);
  RegisterAsAddressable(PARAM_P2, &p2_);
}
// clang-format on

/**
 * Populate any parameters,
 * Validate that values are within expected ranges when bind<>() overloads cannot be used
 *
 * Note: all parameters are populated from configuration files
 */
void OffByOne::DoValidate() {
  parameters_.Validate(PARAM_P1)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);
  parameters_.Validate(PARAM_P2)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);
  parameters_.Validate(PARAM_K)->IsAge();
}

/**
 * Reset this object
 */
void OffByOne::DoBuild() {
  DoReset();
}

/**
 * Reset the mis_matrix to ensure that it has the latest
 * changes from any addressable modifications
 */
void OffByOne::DoReset() {
  mis_matrix_[0][0] = 1.0 - p2_;
  mis_matrix_[0][1] = p2_;
  for (unsigned i = 1; i < (mis_matrix_.size() - 1); ++i) {
    mis_matrix_[i][i - 1] = p1_;
    mis_matrix_[i][i]     = 1.0 - (p1_ + p2_);
    mis_matrix_[i][i + 1] = p2_;
  }

  mis_matrix_[age_spread_ - 1][age_spread_ - 2] = p1_;

  if (plus_group_) {
    mis_matrix_[age_spread_ - 1][age_spread_ - 1] = 1.0 - p1_;
  } else {
    mis_matrix_[age_spread_ - 1][age_spread_ - 1] = 1.0 - (p1_ + p2_);
  }

  if (k_ > min_age_) {
    for (unsigned i = 0; i < k_ - min_age_; ++i) {
      for (unsigned j = 0; j < age_spread_; ++j) {
        mis_matrix_[i][j] = (i == j) ? 1.0 : 0.0;  // Identity submatrix
      }
    }
  }
}

} /* namespace ageingerrors */
} /* namespace niwa */
