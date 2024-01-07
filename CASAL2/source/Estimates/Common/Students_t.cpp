/**
 * @file Students_t.cpp
 * @author  A Dunn
 * @version 1.0
 *
 * @section LICENSE
 *
 */

// Headers
#include "Students_t.h"

// Namespaces
namespace niwa {
namespace estimates {

constexpr double PI = 3.1415926535897932384626433832795028;

/**
 * Default constructor
 */
Students_t::Students_t(shared_ptr<Model> model) : Estimate(model) {
  parameters_.Bind<Double>(PARAM_MU, &mu_, "The Student's t location (mu) parameter", "");
  parameters_.Bind<Double>(PARAM_SIGMA, &sigma_, "The Student's t scale (sigma) parameter", "")->set_lower_bound(0.0, false);
  parameters_.Bind<unsigned>(PARAM_DF, &df_, "The Student's t degrees of freedom (df) parameter", "", 1u)->set_lower_bound(0, false);

  RegisterAsAddressable(PARAM_MU, &mu_);
  RegisterAsAddressable(PARAM_SIGMA, &sigma_);
}

/**
 * Calculate and return the score
 * @return The score
 */
Double Students_t::GetScore() {
  Double x1    = lgamma((df_ + 1) / 2) - lgamma(df_ / 2);
  Double x2    = 1 / 2 * log(df_ * PI) + log(sigma_);
  Double x3    = log(1.0 + (1.0 / df_) * pow((value() - mu_) / sigma_, 2.0));
  Double x4    = (df_ + 1) / 2;
  Double score = -(x1 - x2 - x3 * x4);
  return score;
}
/**
 * GetPriorValues()
 *
 * @return vector of mu and sigma
 */
vector<Double> Students_t::GetPriorValues() {
  vector<Double> result = {mu_, sigma_, (Double)(df_)};
  return result;
}
vector<string> Students_t::GetPriorLabels() {
  vector<string> result = {PARAM_MU, PARAM_SIGMA, PARAM_DF};
  return result;
}
} /* namespace estimates */
} /* namespace niwa */
