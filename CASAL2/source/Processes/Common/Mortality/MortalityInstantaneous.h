/**
 * @file MortalityInstantaneous.h
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 28/07/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * This mortality process is a combination of the constant and event mortality processes.
 * Consolidated implementation for both Age and Length partitioned models.
 */
#ifndef SOURCE_PROCESSES_COMMON_MORTALITYINSTANTANEOUS_H_
#define SOURCE_PROCESSES_COMMON_MORTALITYINSTANTANEOUS_H_

// headers
#include "AgeWeights/AgeWeight.h"
#include "Model/Model.h"
#include "Partition/Accessors/Categories.h"
#include "Penalties/Common/Process.h"
#include "Processes/Age/Mortality.h"
#include "Selectivities/Selectivity.h"
#include "Utilities/Map.h"
#include "Utilities/Types.h"

// namespaces
namespace niwa::processes::common {

namespace accessor = niwa::partition::accessors;
using utilities::OrderedMap;

// classes
class MortalityInstantaneous : public age::Mortality {
  /**
   * FisheryData holds all the information related to a fishery
   */
  struct FisheryData {
    unsigned         fishery_ndx_     = 0;
    string           label_           = "";
    string           time_step_label_ = "";
    unsigned         time_step_index_ = 0;
    Double           u_max_           = 0.0;
    string           penalty_label_   = "";
    Penalty*         penalty_         = nullptr;
    vector<unsigned> years_           = {};
    // These objects want to be a map as more useful for projection methods
    map<unsigned, Double> catches_              = {};
    map<unsigned, Double> actual_catches_       = {};
    map<unsigned, Double> exploitation_by_year_ = {};
    map<unsigned, Double> uobs_by_year_         = {};

    Double vulnerability_ = 0.0;
    Double uobs_fishery_  = 0.0;
    Double exploitation_  = 0.0;

    // Age-specific options
    bool catch_as_biomass_ = false;
    bool catch_as_u_       = false;
  };

  struct CategoryData {
    string               category_label_           = "";
    partition::Category* category_                 = nullptr;
    Double*              m_                        = nullptr;
    vector<Double>       exploitation_             = {};
    vector<Double>       exp_values_half_m_        = {};
    vector<Double>       m_values_                 = {};  // Age: m_at_age_, Length: not used (computed inline)
    string               selectivity_label_        = "";
    Selectivity*         selectivity_              = nullptr;
    vector<Double>       selectivity_values_       = {};
    bool                 used_in_current_timestep_ = false;
    unsigned             category_ndx_             = 0;
    // Age-specific
    bool       catch_as_biomass_ = false;
    bool       catch_as_amount_  = false;
    AgeWeight* age_weight_       = nullptr;
    string     age_weight_label_ = "";
  };

  /**
   * FisheryCategoryData is used to store 1 Fishery x Category x Selectivity
   */
  struct FisheryCategoryData {
    FisheryCategoryData(FisheryData& x, CategoryData& y) : fishery_(x), category_(y) {};
    FisheryData&   fishery_;
    CategoryData&  category_;
    string         fishery_label_      = "";
    string         category_label_     = "";
    string         selectivity_label_  = "";
    Selectivity*   selectivity_        = nullptr;
    vector<Double> selectivity_values_ = {};
  };

public:
  // methods
  explicit MortalityInstantaneous(shared_ptr<Model> model);
  virtual ~MortalityInstantaneous() = default;
  void DoValidate() override final;
  void DoBuild() override final;
  void DoReset() override final;
  void DoExecute() override final;
  void RebuildCache() override final;
  void FillReportCache(ostringstream& cache) override final;
  void FillTabularReportCache(ostringstream& cache, bool first_run) override final;

  // Accessor methods for observations (shared across Age and Length)
  bool             check_categories_in_methods_for_removal_obs(vector<string> methods, vector<string> category_labels);
  bool             check_years_in_methods_for_removal_obs(vector<unsigned> years, vector<string> methods);
  bool             check_methods_for_removal_obs(vector<string> methods);
  vector<unsigned> get_fishery_ndx_for_catch_at(vector<string> fishery_labels);
  vector<unsigned> get_category_ndx_for_catch_at(vector<string> category_labels);
  vector<unsigned> get_year_ndx_for_catch_at(vector<unsigned> years);

  // accessors
  vector<Double>& get_catch_at_by_year_fishery_category(unsigned year_ndx, unsigned fishery_ndx, unsigned category_ndx) {
    return removals_by_year_fishery_category_[year_ndx][fishery_ndx][category_ndx];
  };
  const vector<string>& category_labels() const { return category_labels_; }

private:
  map<string, CategoryData*> category_data_;
  vector<CategoryData>       categories_;

  // members
  vector<string>              category_labels_;
  vector<FisheryCategoryData> fishery_categories_;
  map<string, FisheryData>    fisheries_;
  parameters::table::Table*   catches_table_ = nullptr;
  parameters::table::Table*   method_table_  = nullptr;
  accessor::Categories        partition_;
  Double                      current_m_        = 0.0;
  bool                        is_catch_biomass_ = true;

  // members from mortality event
  string              penalty_label_ = "";
  penalties::Process* penalty_       = nullptr;
  string              unit_          = "";

  // members from natural mortality
  vector<Double>             m_input_;
  OrderedMap<string, Double> m_;
  vector<double>             time_step_ratios_temp_;
  map<unsigned, double>      time_step_ratios_;
  vector<string>             selectivity_labels_;
  vector<Selectivity*>       selectivities_;

  // Members for observations
  utilities::Vector4                 removals_by_year_fishery_category_;  // process_years_ x method_labs x category_labels_ x age/length
  vector<string>                     fishery_labels_;
  vector<unsigned>                   process_years_;
  map<string, vector<string>>        fishery_category_check_;  // fishery x categories (Age-specific validation)
  map<string, map<unsigned, Double>> fishery_catch_;           // fishery x year = catch (Age-specific validation)

  // Members for reporting
  vector<unsigned> time_steps_to_skip_applying_F_mortality_;

  // Age-specific options
  bool use_age_weight_    = true;
  bool use_catch_biomass_ = true;
  bool use_u_             = true;
};

}  // namespace niwa::processes::common

#endif /* SOURCE_PROCESSES_COMMON_MORTALITYINSTANTANEOUS_H_ */
