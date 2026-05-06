/**
 * @file RecruitmentConstant.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 14/12/2012
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * Consolidated implementation of constant recruitment for both Age and
 * Length partitioned models.
 */

// Headers
#include "RecruitmentConstant.h"

#include "Categories/Categories.h"
#include "Logging/Logging.h"
#include "Utilities/Map.h"
#include "Utilities/Math.h"

// Namespaces
namespace niwa::processes::common {

using niwa::partition::accessors::CategoriesWithAge;

/**
 * Default Constructor
 */
RecruitmentConstant::RecruitmentConstant(shared_ptr<Model> model) : Process(model), partition_length_(model) {
  // clang-format off
  parameters_.Bind<string>(PARAM_CATEGORIES, &category_labels_, "The categories")
    ->flag_is_category();
  parameters_.Bind<Double>(PARAM_PROPORTIONS, &proportions_, "The proportion for each category")
    ->set_is_optional(true);
  parameters_.Bind<unsigned>(PARAM_AGE, &age_, "The age at recruitment")
    ->set_is_optional(true);
  parameters_.Bind<Double>(PARAM_LENGTH_BINS, &length_bins_, 
    "The length bin that recruits are uniformly distributed over at the time of recruitment. Needs to be consistent with @model length bin inputs")
    ->set_is_optional(true);
  parameters_.Bind<Double>(PARAM_R0, &r0_, "R0, the recruitment used for annual recruits and initialise the model");
  // clang-format on

  RegisterAsAddressable(PARAM_R0, &r0_);
  RegisterAsAddressable(PARAM_PROPORTIONS, &proportions_categories_);

  process_type_        = ProcessType::kRecruitment;
  partition_structure_ = PartitionType::kAge | PartitionType::kLength;
}

/**
 * Validate the parameters for this process
 *
 * 1. Check for the required parameters
 * 2. Assign the label from the parameters
 * 3. Assign remaining local parameters
 */
void RecruitmentConstant::DoValidate() {
  LOG_TRACE();

  // Age-specific validation
  if (process_profile_ == ProcessProfile::kAge) {
    parameters_.Validate(PARAM_AGE)->IsAge()->DefaultValue(model()->min_age());
  }

  // Length-specific validation
  if (process_profile_ == ProcessProfile::kLength) {
    parameters_.ValidateVector(PARAM_LENGTH_BINS)->IsLengthBin()->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES);
  }

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
  LOG_TRACE();

  if (process_profile_ == ProcessProfile::kAge) {
    partition_age_ = CategoriesWithAgePtr(new CategoriesWithAge(model(), category_labels_, age_));
  } else {
    partition_length_.Init(category_labels_);
  }
}

/**
 * Execute the constant recruitment process
 */
void RecruitmentConstant::DoExecute() {
  LOG_TRACE();

  if (process_profile_ == ProcessProfile::kAge) {
    /**
     * Calculate new proportion totals to account for dynamic categories
     */
    Double total_proportions = 0.0;
    for (auto iterator = partition_age_->begin(); iterator != partition_age_->end(); ++iterator) {
      total_proportions += proportions_categories_[iterator->first];
    }

    /**
     * Update our partition with new recruitment values
     */
    for (auto iterator = partition_age_->begin(); iterator != partition_age_->end(); ++iterator) {
      *iterator->second += (proportions_categories_[iterator->first] / total_proportions) * r0_;
    }
  } else {
    // Length-based execution
    // Calculate new proportion totals to account for dynamic categories
    Double total_proportions = 0.0;
    for (auto category : partition_length_) total_proportions += proportions_categories_[category->name_];

    if (length_bins_.empty()) {
      LOG_ERROR_P(PARAM_LENGTH_BINS) << "At least one length bin must be specified for length-based recruitment";
      return;
    }

    // Update our partition with new recruitment values
    for (auto category : partition_length_) {
      if (category->data_.size() != model()->length_bins().size())
        LOG_CODE_ERROR() << "This function was written when categories were forced to have the same length bins as models. This is no longer the case.";

      r0_by_length_bin_     = (r0_ * (proportions_categories_[category->name_]) / total_proportions) / length_bins_.size();
      unsigned length_index = 0;
      for (auto length_bin : length_bins_) {
        auto model_length_iter = std::find(model()->length_bins().begin(), model()->length_bins().end(), length_bin);
        if (model_length_iter == model()->length_bins().end()) {
          LOG_ERROR_P(PARAM_LENGTH_BINS) << "Length bin " << length_bin << " is not defined in the model length bins";
          continue;
        }
        length_index = std::distance(model()->length_bins().begin(), model_length_iter);
        if (length_index >= category->data_.size()) {
          LOG_CODE_ERROR() << "Length index " << length_index << " is out of range for category " << category->name_;
          continue;
        }
        LOG_FINEST() << "putting " << r0_by_length_bin_ << " in category " << category->name_ << " in length bin " << model()->length_bins()[length_index];
        category->data_[length_index] += r0_by_length_bin_;
      }
    }
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

}  // namespace niwa::processes::common
