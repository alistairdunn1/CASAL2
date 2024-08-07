/**
 * @file VectorAverage.h
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 28/10/2014
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * vector average penalty = a penalty on the mean of a vector being
 * difference from some user defined value (i) square(mean(vector)-k).
 * Optionally in log space (ii) square(mean(log(vector))-k) or (iii)
 * square(log(mean(vector))/k). Has an arbitrary multiplier (default=1) as well.
 */
#ifndef PENALTIES_VECTORAVERAGE_H_
#define PENALTIES_VECTORAVERAGE_H_

// headers
#include "../../AdditionalPriors/AdditionalPrior.h"

// namespaces
namespace niwa {
namespace additionalpriors {

/**
 * Class definition
 */
class VectorAverage : public niwa::AdditionalPrior {
public:
  // methods
  VectorAverage(shared_ptr<Model> model);
  virtual ~VectorAverage() = default;
  Double GetScore() override final;

protected:
  // methods
  void DoValidate() final{};
  void DoBuild() final;
  // members
  string                 method_                 = "";
  map<unsigned, Double>* addressable_map_        = nullptr;
  vector<Double>*        addressable_vector_     = nullptr;
  vector<Double*>*       addressable_ptr_vector_ = nullptr;
  Double                 multiplier_             = 0.0;
  Double                 k_;
};

} /* namespace additionalpriors */
} /* namespace niwa */

#endif /* VECTORAVERAGE_H_ */
