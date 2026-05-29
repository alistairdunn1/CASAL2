/**
 * @file AR1.h
 * @author Casal2
 * @date 2026/05/29
 * @section LICENSE
 *
 * Copyright Casal2 Project 2026 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * Additional prior that applies an AR1 penalty to sequential elements
 * of a vector/map/multiple addressable.
 */
#ifndef ADDITIONAL_PRIORS_AR1_H_
#define ADDITIONAL_PRIORS_AR1_H_

// headers
#include "../../AdditionalPriors/AdditionalPrior.h"

// namespaces
namespace niwa {
namespace additionalpriors {

/**
 * Class definition
 */
class AR1 : public niwa::AdditionalPrior {
public:
  explicit AR1(shared_ptr<Model> model);
  virtual ~AR1() = default;
  Double GetScore() override final;

protected:
  void DoValidate() override final;
  void DoBuild() override final;

  map<unsigned, Double>* addressable_map_        = nullptr;
  vector<Double>*        addressable_vector_     = nullptr;
  vector<Double*>*       addressable_ptr_vector_ = nullptr;

  Double rho_        = 0.0;
  Double sigma_      = 1.0;
  Double mean_       = 0.0;
  Double multiplier_ = 1.0;
  bool   log_scale_  = false;
};

} /* namespace additionalpriors */
} /* namespace niwa */

#endif /* ADDITIONAL_PRIORS_AR1_H_ */
