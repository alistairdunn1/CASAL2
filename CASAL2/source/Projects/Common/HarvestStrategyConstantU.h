/**
 * @file HarvestStrategyConstantU.h
 * @author  A Dunn
 * @date 01/04/2024
 * @section LICENSE
 *
 * @section DESCRIPTION
 *
 * This projection class will take a parameter and apply a constant U harvest strategy
 */
#ifndef PROJECTS_HS_CONSTANT_U_H_
#define PROJECTS_HS_CONSTANT_U_H_

// headers
#include "../../DerivedQuantities/DerivedQuantity.h"
#include "../../DerivedQuantities/Manager.h"
#include "../../Model/Model.h"
#include "../../Projects/Project.h"

// namespaces
namespace niwa {
namespace projects {

/**
 * Class definition
 */
class HarvestStrategyConstantU : public niwa::Project {
public:
  explicit HarvestStrategyConstantU(shared_ptr<Model> model);
  virtual ~HarvestStrategyConstantU() = default;
  void DoValidate();
  void DoBuild();
  void DoReset();
  void DoUpdate();

private:
  // members
  string           biomass_index_label_;
  Double           u_;
  Double           value_;
  Double           min_delta_;
  Double           max_delta_;
  unsigned         year_lag_;
  unsigned         year_delta_;
  unsigned         first_year_;
  DerivedQuantity* biomass_index_       = nullptr;
  unsigned         biomass_index_phase_ = 0;
  Double           biomass_index_scalar_;
  Double           last_catch_;
  Double           this_catch_;
  Double           current_catch_;
  int              update_counter_ = 0;

protected:
  shared_ptr<Model> model_ = nullptr;
};

} /* namespace projects */
} /* namespace niwa */

#endif /* PROJECTS_HS_CONSTANT_U_H_ */
