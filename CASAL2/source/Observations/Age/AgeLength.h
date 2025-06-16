/**
 * @file AgeLength.h
 * @author  C.Marsh
 * @version 1.0
 * @date 2022
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * The time class represents a moment of time.
 *
 */
#ifndef AGE_LENGTH_OBSERVATIONS_H_
#define AGE_LENGTH_OBSERVATIONS_H_

// Headers
#include "AgeLengths/AgeLength.h"
#include "AgeingErrors/AgeingError.h"
#include "Observations/Observation.h"
#include "Partition/Accessors/Cached/CombinedCategories.h"
#include "Partition/Accessors/CombinedCategories.h"

// Namespace
namespace niwa {
class Selectivity;
class AgeLength;
namespace observations::age {

// Enumerated Types
enum class SampleType {
  kAge    = 0,
  kLength = 1,
  kRandom = 2,
};

using partition::accessors::CombinedCategoriesPtr;
using partition::accessors::cached::CachedCombinedCategoriesPtr;

/**
 * Class Definition
 */
class AgeLength : public niwa::Observation {
public:
  // Methods
  explicit AgeLength(shared_ptr<Model> model);
  virtual ~AgeLength() = default;
  void         DoValidate() override final;
  virtual void DoBuild() override;
  void         DoReset() override final {};
  void         PreExecute() override final;
  void         Execute() override final;
  void         CalculateScore() override final;
  bool         HasYear(unsigned year) const override final { return year == year_; }

protected:
  // Members
  unsigned                    year_                                     = 0;
  vector<unsigned>            individual_ages_                          = {};
  vector<double>              individual_lengths_                       = {};
  vector<string>              selectivity_labels_                       = {};
  vector<Selectivity*>        selectivities_                            = {};
  string                      time_step_label_                          = {};
  bool                        plus_group_                               = false;
  unsigned                    age_spread_                               = 0;
  string                      sample_type_                              = {};
  SampleType                  actual_sample_type_                       = SampleType::kAge;
  vector<vector<Double>>      ageing_error_matrix_                      = {};
  CachedCombinedCategoriesPtr cached_partition_                         = nullptr;
  CombinedCategoriesPtr       partition_                                = nullptr;
  vector<Double>              process_error_values_                     = {};
  string                      ageing_error_label_                       = {};
  AgeingError*                ageing_error_                             = nullptr;
  Double                      time_step_proportion_                     = 0.0;
  vector<Double>              quantiles_                                = {};
  vector<Double>              quantile_breaks_                          = {};
  vector<double>              unique_lengths_                           = {};
  vector<vector<Double>>      numbers_at_age_                           = {};
  unsigned                    n_fish_                                   = 0;
  vector<string>              split_category_labels_                    = {};
  vector<string>              split_numerator_categories_               = {};
  bool                        apply_ageing_error_                       = false;
  vector<Double>              numbers_by_unique_size_                   = {};
  vector<string>              numerator_categories_                     = {};
  vector<Double>              numbers_at_age_numerator_                 = {};
  vector<bool>                vector_of_cached_categories_in_numerator_ = {};
  niwa::AgeLength*            age_length_ptr_                           = nullptr;
  Selectivity*                numerator_selectivity_                    = nullptr;
  string                      selectivity_label_for_numerator_          = "";
  // vectors to do lookups on combined categories populated during DoBuild()
};

}  // namespace observations::age
}  // namespace niwa
#endif /* AGE_LENGTH_OBSERVATIONS_H_ */