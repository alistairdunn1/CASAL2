/**
 * @file ProcessProportionsByCategory.h
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 17/02/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * << Add Description >>
 */
#ifndef AGE_OBSERVATIONS_PROCESSPROPORTIONSBYCATEGORY_H_
#define AGE_OBSERVATIONS_PROCESSPROPORTIONSBYCATEGORY_H_

// headers
#include "Observations/Age/ProportionsByCategory.h"

// namespaces
namespace niwa {
namespace observations {
namespace age {

/**
 * Class definition
 */
class ProcessProportionsByCategory : public observations::age::ProportionsByCategory {
public:
  // methods
  ProcessProportionsByCategory(shared_ptr<Model> model);
  virtual ~ProcessProportionsByCategory() = default;
  void DoValidate() override final;
  void DoBuild() override final;

private:
  // members
  string process_label_      = "";   ///< The label of the process for the observation
  Double process_proportion_ = 0.0;  ///< The proportion through the process when the observation is evaluated
};

} /* namespace age */
} /* namespace observations */
} /* namespace niwa */

#endif /* AGE_OBSERVATIONS_PROCESSPROPORTIONSBYCATEGORY_H_ */
