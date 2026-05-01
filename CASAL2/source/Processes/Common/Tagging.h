/**
 * @file Tagging.h
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 26/01/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * Tagging process - moves fish from one category to another based on
 * tagging data. Consolidated implementation for both Age and Length models.
 * Age models use type "tag_by_age", Length models use type "tagging".
 */
#ifndef SOURCE_PROCESSES_COMMON_TAGGING_H_
#define SOURCE_PROCESSES_COMMON_TAGGING_H_

// headers
#include "Partition/Accessors/Categories.h"
#include "Penalties/Penalty.h"
#include "Processes/Process.h"
#include "Selectivities/Selectivity.h"
#include "Utilities/Types.h"

// namespaces
namespace niwa::processes::common {

namespace accessor = niwa::partition::accessors;
using utilities::OrderedMap;

/**
 * Class definition
 */
class Tagging : public niwa::Process {
public:
  // methods
  explicit Tagging(shared_ptr<Model> model);
  virtual ~Tagging() = default;
  void DoValidate() override final;
  void DoBuild() override final;
  void DoReset() override final {};
  void DoExecute() override final;
  void FillReportCache(ostringstream& cache) override final;

private:
  // members
  vector<string>                from_category_labels_;
  vector<string>                to_category_labels_;
  accessor::Categories          to_partition_;
  accessor::Categories          from_partition_;
  vector<unsigned>              years_;
  vector<string>                selectivity_labels_;
  vector<string>                split_from_category_labels_;
  map<string, Selectivity*>     selectivities_;
  string                        penalty_label_                       = "";
  Penalty*                      penalty_                             = nullptr;
  double                        u_max_                               = 0;
  double                        tolerance_                           = 0;
  Double                        initial_mortality_                   = 0;
  string                        initial_mortality_selectivity_label_ = "";
  Selectivity*                  initial_mortality_selectivity_       = nullptr;
  vector<Double>                n_;
  parameters::table::Table*     numbers_table_     = nullptr;
  parameters::table::Table*     proportions_table_ = nullptr;
  unsigned                      first_year_        = 0;
  map<unsigned, vector<Double>> numbers_;
  map<unsigned, Double>         n_by_year_;

  // Age-specific members
  unsigned               number_categories_ = 0;
  unsigned               min_age_           = 0;
  unsigned               max_age_           = 0;
  unsigned               age_spread_        = 0;
  unsigned               min_age_offset_    = 0;
  vector<vector<Double>> numbers_at_age_by_category_;           // dims category x ages
  vector<vector<Double>> selected_numbers_at_age_by_category_;  // dims category x ages
  vector<vector<Double>> exploitation_by_age_category_;         // category x age bins
  vector<Double>         exploitation_by_age_;
  vector<Double>         final_exploitation_by_age_;
  vector<vector<Double>> proportion_by_age_;            // year x length bins
  vector<Double>         tagged_fish_by_year_;          // year bins
  vector<Double>         tag_to_fish_by_age_;           // age bins
  vector<vector<Double>> tag_to_fish_by_category_age_;  // category x age bins
  vector<Double>         vulnerable_fish_by_age_;       // age bins
  // Containers for reporting
  vector<vector<vector<Double>>> tagged_fish_after_init_mort_;  // n_years x n_from_categories x n_ages
  vector<vector<vector<Double>>> actual_tagged_fish_to_;        // n_years x n_to_categories x n_ages
};

} /* namespace niwa::processes::common */

#endif /* SOURCE_PROCESSES_COMMON_TAGGING_H_ */
