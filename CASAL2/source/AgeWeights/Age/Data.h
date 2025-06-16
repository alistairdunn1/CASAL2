/**
 * @file Data.h
 * @author  C.Marsh
 * @date 16/11/2017
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * Data Ageweight is a user defined table that describes the mean weight for each age group in every year.
 */
#ifndef AGEWEIGHT_DATA_H_
#define AGEWEIGHT_DATA_H_

// headers
#include "../../AgeWeights/AgeWeight.h"

// namespaces
namespace niwa::ageweights {

// classes
class Data : public AgeWeight {
public:
  // methods
  explicit Data(shared_ptr<Model> model);
  virtual ~Data() {};
  void DoValidate() override final;
  void DoBuild() override final;
  void DoReset() override final {};
  void DoRebuildCache() override final {};  // This should never happen. i.e time vary data type.

  // accessors
  Double mean_weight_at_age_by_year(unsigned year, unsigned age) override final;

private:
  // members
  parameters::table::Table*            data_table_                = nullptr;
  map<unsigned, vector<Double>>        data_by_year_              = {};
  map<unsigned, map<unsigned, Double>> mean_data_by_year_and_age_ = {};
  map<unsigned, Double>                initial_                   = {};
  vector<unsigned>                     steps_to_figure_           = {};
  unsigned                             number_time_steps_         = 0;
  unsigned                             final_year_                = 0;
  vector<unsigned>                     years_                     = {};
  vector<unsigned>                     age_                       = {};
  string                               units_                     = "";
  string                               equilibrium_method_        = "";
  Double                               unit_multipier_            = 1.0;
};

}  // namespace niwa::ageweights

#endif /* AGEWEIGHT_DATA_H_ */
