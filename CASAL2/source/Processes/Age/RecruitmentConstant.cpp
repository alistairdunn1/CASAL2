/**
 * @file ConstantRecruitment.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 14/12/2012
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "RecruitmentConstant.h"

#include "../../Utilities/Math.h"
#include "Categories/Categories.h"
#include "Logging/Logging.h"
#include "Utilities/Map.h"

// Namespaces
namespace niwa::processes::age {

using niwa::partition::accessors::CategoriesWithAge;

/**
 * Default Constructor
 */
RecruitmentConstant::RecruitmentConstant(shared_ptr<Model> model) : Process(model) {
  // clang-format off
  parameters_.Bind<string>(PARAM_CATEGORIES, &category_labels_, "The categories")
    ->flag_is_category();
  parameters_.Bind<Double>(PARAM_PROPORTIONS, &proportions_, "The proportion for each category")
    ->set_is_optional(true);
  parameters_.Bind<unsigned>(PARAM_AGE, &age_, "The age at recruitment");
  parameters_.Bind<Double>(PARAM_R0, &r0_, "R0, the recruitment used for annual recruits and initialise the model");
  // clang-format on

  RegisterAsAddressable(PARAM_R0, &r0_);
  RegisterAsAddressable(PARAM_PROPORTIONS, &proportions_categories_);

  process_type_        = ProcessType::kRecruitment;
  partition_structure_ = PartitionType::kAge;
}

/**
 * Validate the parameters for this process
 *
 * 1. Check for the required parameters
 * 2. Assign the label from the parameters
 * 3. Assign remaining local parameters
 */
void RecruitmentConstant::DoValidate() {
  parameters_.Validate(PARAM_AGE)->IsAge()->DefaultValue(model_->min_age());
  parameters_.Validate(PARAM_R0)->GreaterThan(0.0);
  parameters_.ValidateVector(PARAM_PROPORTIONS)
      ->GreaterThanOrEqualTo(0.0)
      ->LessThanOrEqualTo(1.0)
      ->SumToOne()
      ->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)
      ->SameNumberOfElementsAs(PARAM_CATEGORIES);

  // Default the proportions if not defined
  if (proportions_.size() == 0 && category_labels_.size() > 0) {
    Double proportion = category_labels_.size() / 1.0;
    proportions_.assign(category_labels_.size(), proportion);
  }

  proportions_categories_ = utilities::OrderedMap<std::string, Double>::create(category_labels_, proportions_);
}

/**
 * Build any runtime relationships to other objects in the system.
 */
void RecruitmentConstant::DoBuild() {
  partition_ = CategoriesWithAgePtr(new CategoriesWithAge(model_, category_labels_, age_));
}

/**
 * Execute the constant recruitment process
 */
void RecruitmentConstant::DoExecute() {
  /**
   * Calculate new proportion totals to account for dynamic categories
   */
  Double total_proportions = 0.0;
  for (auto iterator = partition_->begin(); iterator != partition_->end(); ++iterator) {
    total_proportions += proportions_categories_[iterator->first];
  }

  /**
   * Update our partition with new recruitment values
   */
  for (auto iterator = partition_->begin(); iterator != partition_->end(); ++iterator) {
    *iterator->second += (proportions_categories_[iterator->first] / total_proportions) * r0_;
  }
}

/**
 * Fill the report cache
 * @description A method for reporting process information
 * @param cache a cache object to print to
 */
void RecruitmentConstant::FillReportCache(ostringstream& cache) {}

/**
 * Fill the tabular report cache
 * @description A method for reporting tabular process information
 * @param cache a cache object to print to
 * @param first_run whether to print the header
 *
 */
void RecruitmentConstant::FillTabularReportCache(ostringstream& cache, bool first_run) {}

}  // namespace niwa::processes::age
