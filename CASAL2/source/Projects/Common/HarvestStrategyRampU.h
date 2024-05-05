/**
 * @file HarvestStrategyRampU.h
 * @author  A Dunn
 * @date 01/04/2024
 * @section LICENSE
 *
 * @section DESCRIPTION
 *
 * This projection class will take a parameter and apply a ramp U harvest strategy
 */
#ifndef PROJECTS_HS_RAMP_U_H_
#define PROJECTS_HS_RAMP_U_H_

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
class HarvestStrategyRampU : public niwa::Project {
public:
  explicit HarvestStrategyRampU(shared_ptr<Model> model);
  virtual ~HarvestStrategyRampU() = default;
  void DoValidate();
  void DoBuild();
  void DoReset();
  void DoUpdate();

private:
  // members
  string           biomass_index_label_;
  string           reference_index_label_;
  vector<double>   u_;
  vector<double>   reference_points_;
  double           value_;
  bool             adjust_bias_;
  double           min_delta_;
  double           max_delta_;
  unsigned         year_lag_;
  unsigned         year_delta_;
  unsigned         first_year_;
  DerivedQuantity* biomass_index_ = nullptr;
  double           biomass_index_scalar_;
  DerivedQuantity* reference_index_     = nullptr;
  unsigned         biomass_index_phase_ = 0;
  double           last_catch_;
  double           this_catch_;
  double           current_catch_;
  int              update_counter_ = 0;
  string           initialisation_phase_label_;
  unsigned         initialisation_phase_;

protected:
  shared_ptr<Model> model_ = nullptr;
};

} /* namespace projects */
} /* namespace niwa */

#endif /* PROJECTS_HS_RAMP_U_H_ */
