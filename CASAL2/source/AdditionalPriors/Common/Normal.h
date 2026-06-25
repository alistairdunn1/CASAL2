/**
 * @file Normal.h
 * @author Casal2 Team
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */
#ifndef ADDITIONALPRIORS_NORMAL_H_
#define ADDITIONALPRIORS_NORMAL_H_

// headers
#include "../../AdditionalPriors/AdditionalPrior.h"

// namespaces
namespace niwa {
namespace additionalpriors {

// classes
class Normal : public AdditionalPrior {
public:
  // methods
  Normal(shared_ptr<Model> model);
  virtual ~Normal() = default;
  void   DoValidate() final;
  void   DoBuild() final;
  Double GetScore() final;

protected:
  // members
  Double*                addressable_            = nullptr;
  map<unsigned, Double>* addressable_map_        = nullptr;
  vector<Double>*        addressable_vector_     = nullptr;
  vector<Double*>*       addressable_ptr_vector_ = nullptr;
  Double                 score_                  = 0.0;
  Double                 mu_;
  Double                 cv_;
  Double                 sigma_;
};

} /* namespace additionalpriors */
} /* namespace niwa */

#endif /* ADDITIONALPRIORS_NORMAL_H_ */
