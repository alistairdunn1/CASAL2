/**
 * @file Sum.cpp
 * @author A. Dunn
 * @date 2/8/2023
 * @section LICENSE
 *
 * @section DESCRIPTION
 *
 * Applied to list of parameters and applied a prior on their sum
 */
#ifndef SUM_H_
#define SUM_H_

// headers
#include "../../AdditionalPriors/AdditionalPrior.h"
#include "../../Estimates/Estimate.h"

// namespaces
namespace niwa {
namespace additionalpriors {

/**
 * Class definition
 */
class Sum : public niwa::AdditionalPrior {
public:
  // methods
  Sum(shared_ptr<Model> model);
  virtual ~Sum() = default;
  Double GetScore() final;

protected:
  // methods
  void DoValidate() final;
  void DoBuild() final;
  // members
  vector<string>       parameter_list_;
  string               distribution_;
  Double               mu_;
  Double               cv_;
  Double               sigma_;
  Double*              addressable_     = nullptr;
  map<string, Double>* addressable_map_ = nullptr;
};

} /* namespace additionalpriors */
} /* namespace niwa */

#endif /* SUM_H_ */
