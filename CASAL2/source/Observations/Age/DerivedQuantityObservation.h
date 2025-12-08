/**
 * @file DerivedQuantityObservation.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 7/01/2014
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 * @section DESCRIPTION
 *
 * << Add Description >>
 */
#ifndef AGE_OBSERVATIONS_DERIVED_QUANTITY_H_
#define AGE_OBSERVATIONS_DERIVED_QUANTITY_H_

// headers
#include "../../DerivedQuantities/DerivedQuantity.h"
#include "../../DerivedQuantities/Manager.h"
#include "../../InitialisationPhases/Manager.h"
#include "Observations/Observation.h"
#include "Partition/Accessors/Cached/CombinedCategories.h"
#include "Partition/Accessors/CombinedCategories.h"

// namespaces
namespace niwa {
class Selectivity;
namespace observations {
namespace age {

using partition::accessors::CombinedCategoriesPtr;
using partition::accessors::cached::CachedCombinedCategoriesPtr;

/**
 * class definition
 */
class DerivedQuantityObservation : public niwa::Observation {
public:
  // methods
  DerivedQuantityObservation(shared_ptr<Model> model);
  virtual ~DerivedQuantityObservation() = default;
  void         DoValidate() override final;
  virtual void DoBuild() override;
  void         DoReset() override final;
  void         PreExecute() override final;
  void         Execute() override final;
  void         CalculateScore() override final;
  bool         HasYear(unsigned year) const override final { return std::find(years_.begin(), years_.end(), year) != years_.end(); }

protected:
  // members
  string                    derived_quantity_label_;
  DerivedQuantity*          derived_quantity_ = nullptr;
  string                    initialisation_phase_label_;
  unsigned                  initialisation_phase_;
  vector<unsigned>          years_;
  Double                    process_error_value_ = 0;
  parameters::table::Table* obs_table_           = nullptr;
  string                    time_step_label_     = "";
  unsigned                  year_lag_            = 0;

  map<unsigned, vector<double> > obs_by_year_;
  map<unsigned, double>          error_values_by_year_;
};

} /* namespace age */
}  // namespace observations
}  // namespace niwa

#endif /* AGE_OBSERVATIONS_DERIVED_QUANTITY_H_ */
