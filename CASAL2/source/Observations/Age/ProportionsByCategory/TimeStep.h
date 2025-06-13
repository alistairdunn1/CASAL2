/**
 * @file TimeStepProportionsByCategory.h
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 10/03/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * << Add Description >>
 */
#ifndef AGE_OBSERVATIONS_TIMESTEPPROPORTIONSBYCATEGORY_H_
#define AGE_OBSERVATIONS_TIMESTEPPROPORTIONSBYCATEGORY_H_

// headers
#include "Observations/Age/ProportionsByCategory.h"

// namespaces
namespace niwa {
namespace observations {
namespace age {

/**
 *
 */
class TimeStepProportionsByCategory : public observations::age::ProportionsByCategory {
public:
  // methods
  TimeStepProportionsByCategory(shared_ptr<Model> model);
  virtual ~TimeStepProportionsByCategory() = default;
  void DoValidate() override final;
  void DoBuild() override final;

private:
  Double time_step_proportion_ = 0.0;  ///< The proportion through the time step when the observation is evaluated
};

} /* namespace age */
} /* namespace observations */
} /* namespace niwa */

#endif /* AGE_OBSERVATIONS_TIMESTEPPROPORTIONSBYCATEGORY_H_ */
