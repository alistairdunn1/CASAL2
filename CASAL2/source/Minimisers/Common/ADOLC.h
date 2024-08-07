/**
 * @file ADOLC.h
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 17/11/2014
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * << Add Description >>
 */
#ifdef USE_AUTODIFF
#ifdef USE_ADOLC
#ifndef MINIMISERS_ADOLC_H_
#define MINIMISERS_ADOLC_H_

// headers
#include "../../Minimisers/Minimiser.h"

// namespaces
namespace niwa {
namespace minimisers {

/**
 * Class definition
 */
class ADOLC : public niwa::Minimiser {
public:
  // methods
  explicit ADOLC(shared_ptr<Model> model);
  virtual ~ADOLC() = default;
  void DoValidate() override final{};
  void DoBuild() override final{};
  void DoReset() override final{};
  void Execute() override final;

private:
  // Members
  int    max_iterations_;
  int    max_evaluations_;
  double gradient_tolerance_;
  double step_size_;
  string parameter_transformation_;
  bool   use_tan_transform;
};

} /* namespace minimisers */
} /* namespace niwa */

#endif /* MINIMISERS_ADOLC_H_ */
#endif /* USE_ADOLC */
#endif /* USE_AUTODIFF */
