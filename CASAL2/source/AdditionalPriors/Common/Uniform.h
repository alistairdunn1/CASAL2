/**
 * @file Uniform.h
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 21/10/2014
 * @section LICENSE
 *
 * @section DESCRIPTION
 *
 */
#ifndef ADDITIONALPRIORS_UNIFORM_H_
#define ADDITIONALPRIORS_UNIFORM_H_

// headers
#include "../../AdditionalPriors/AdditionalPrior.h"

// namespaces
namespace niwa {
namespace additionalpriors {

// classes
class Uniform : public AdditionalPrior {
public:
  // methods
  Uniform(shared_ptr<Model> model);
  virtual ~Uniform() = default;
  void   DoValidate() final {};
  void   DoBuild() final;
  Double GetScore() final;

protected:
  // members
  Double* addressable_ = nullptr;
  Double  score_       = 0.0;
};

} /* namespace additionalpriors */
} /* namespace niwa */

#endif /* ADDITIONALPRIORS_UNIFORM_H_ */
