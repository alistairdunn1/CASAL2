/**
 * @file Maturation.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 11/09/2013
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "Maturation.h"

#include "Categories/Categories.h"
#include "Selectivities/Manager.h"
#include "Selectivities/Selectivity.h"
#include "Utilities/Map.h"

// namespaces
namespace niwa {
namespace processes {
namespace age {

/**
 * Default constructor
 */
Maturation::Maturation(shared_ptr<Model> model) : Process(model), from_partition_(model), to_partition_(model) {
  // clang-format off
  parameters_.Bind<string>(PARAM_FROM, &from_category_names_, "The list of categories to mature from")
    ->flag_is_category();
  parameters_.Bind<string>(PARAM_TO, &to_category_names_, "The list of categories to mature to")
    ->flag_is_category();
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_names_, "The list of selectivities to use for maturation");
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years to be associated with the maturity rates");
  parameters_.Bind<Double>(PARAM_RATES, &rates_, "The rates to mature for each year");
  parameters_.Bind<string>(PARAM_MISSING_YEARS_METHOD, &missing_years_method_, "How to handle rate value for missing years")
    ->set_default_value(PARAM_ERROR);
  parameters_.Bind<string>(PARAM_PROJECTION_YEARS_METHOD, &missing_years_method_, "How to handle rate value for missing years")
    ->set_default_value(PARAM_FINAL_YEAR);
  // clang-format on

  RegisterAsAddressable(PARAM_RATES, &rates_by_years_);

  process_type_        = ProcessType::kMaturation;
  partition_structure_ = PartitionType::kAge;
}

/**
 * Validate the values from the configuration file
 */
void Maturation::DoValidate() {
  LOG_TRACE();
  parameters_.ValidateVector(PARAM_FROM)->SameNumberOfElementsAs(PARAM_TO);

  parameters_.ValidateVector(PARAM_SELECTIVITIES)->ExpandToSameNumberOfElementsAs(PARAM_FROM)->SameNumberOfElementsAs(PARAM_FROM);
  parameters_.ValidateVector(PARAM_RATES)->ExpandToSameNumberOfElementsAs(PARAM_YEARS)->SameNumberOfElementsAs(PARAM_YEARS);
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->DefaultToAllModelYears();
  parameters_.Validate(PARAM_MISSING_YEARS_METHOD)->IsInList({PARAM_ERROR, PARAM_ZERO, PARAM_FINAL_YEAR});
  parameters_.Validate(PARAM_PROJECTION_YEARS_METHOD)->IsInList({PARAM_ZERO, PARAM_FINAL_YEAR});

  if (missing_years_method_ == PARAM_ERROR) {
    parameters_.ValidateVector(PARAM_YEARS)->NumberOfElements(model()->years().size());
  }

  rates_by_years_     = utilities::Map::create(years_, rates_);
  missing_years_zero_ = missing_years_method_ == PARAM_ZERO;

  //  // Validate Categories
  niwa::Categories* categories = model()->categories();

  // Validate that each from and to category have the same age range.
  for (unsigned i = 0; i < from_category_names_.size(); ++i) {
    if (categories->min_age(from_category_names_[i]) != categories->min_age(to_category_names_[i])) {
      LOG_ERROR_P(PARAM_FROM) << ": 'from' category " << from_category_names_[i] << " does not"
                              << " have the same age range as the 'to' category " << to_category_names_[i];
    }

    if (categories->max_age(from_category_names_[i]) != categories->max_age(to_category_names_[i])) {
      LOG_ERROR_P(PARAM_FROM) << ": 'from' category " << from_category_names_[i] << " does not"
                              << " have the same age range as the 'to' category " << to_category_names_[i];
    }
  }
}

/**
 * Build any runtime relationships
 * - Build the partition accessors
 * - Verify the selectivities are valid
 * - Get pointers to the selectivities
 */
void Maturation::DoBuild() {
  LOG_TRACE();

  from_partition_.Init(from_category_names_);
  to_partition_.Init(to_category_names_);

  for (string label : selectivity_names_) {
    Selectivity* selectivity = model()->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_SELECTIVITIES) << ": Selectivity label " << label << " was not found.";
    selectivities_.push_back(selectivity);
  }

  // check there is a year for all model years
  for (auto this_year : model()->years()) {
    if (find(years_.begin(), years_.end(), this_year) == years_.end())
      LOG_WARNING() << "At " << location() << " the model year " << this_year << " was not found in " << PARAM_YEARS
                    << ". This is potentially an error. Please check the input configuration files";
  }

  // We have some missing years, fill them in based on the method
  if (model()->years().size() != years_.size() && !missing_years_zero_) {
    Double final_year_rate = rates_by_years_.rbegin()->second;

    for (auto this_year : model()->years()) {
      if (find(years_.begin(), years_.end(), this_year) == years_.end()) {
        rates_by_years_[this_year] = final_year_rate;
      }
    }
  }

  projection_rate_ = projection_years_method_ == PARAM_FINAL_YEAR ? rates_by_years_.rbegin()->second : 0.0;
}

/**
 * Execute the maturation rate process
 */
void Maturation::DoExecute() {
  auto   from_iter = from_partition_.begin();
  auto   to_iter   = to_partition_.begin();
  Double amount    = 0.0;

  unsigned current_year = model()->current_year();
  Double   rate         = 0.0;

  // if year is missing for projection then we grab the last one
  if (rates_by_years_.find(current_year) == rates_by_years_.end())
    rate = projection_rate_;
  else
    rate = rates_by_years_[current_year];  // Be aware this would add value if it doesn't exist.

  for (unsigned i = 0; from_iter != from_partition_.end() && to_iter != to_partition_.end(); ++from_iter, ++to_iter, ++i) {
    unsigned min_age = (*from_iter)->min_age_;

    for (unsigned offset = 0; offset < (*from_iter)->data_.size(); ++offset) {
      amount = rate * selectivities_[i]->GetAgeResult(min_age + offset, (*from_iter)->age_length_) * (*from_iter)->data_[offset];
      if (amount == 0) {
        continue;
      }
      (*from_iter)->data_[offset] -= amount;
      (*to_iter)->data_[offset] += amount;
    }
  }
}

/**
 * Fill the report cache
 * @description A method for reporting process information
 * @param cache a cache object to print to
 */
void Maturation::FillReportCache(ostringstream& cache) {}

/**
 * @Fill the tabular report cache
 * @description A method for reporting tabular process information
 * @param cache a cache object to print to
 * @param first_run whether to print the header
 */
void Maturation::FillTabularReportCache(ostringstream& cache, bool first_run) {}

} /* namespace age */
} /* namespace processes */
} /* namespace niwa */
