/**
 * @file ConstantRecruitment.cpp
 * @author  C.Marsh
 * @version 1.0
 * @date 12/18/2017
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// Headers
#include "RecruitmentConstant.h"

#include "../../Categories/Categories.h"
#include "../../Logging/Logging.h"
#include "../../Utilities/Math.h"

// Namespaces
namespace niwa {
namespace processes {
namespace length {

/**
 * Default constructor
 */
RecruitmentConstant::RecruitmentConstant(shared_ptr<Model> model) : Process(model), partition_(model) {
  parameters_.Bind<string>(PARAM_CATEGORIES, &category_labels_, "The categories")->flag_is_category();
  parameters_.Bind<Double>(PARAM_PROPORTIONS, &proportions_, "The proportions")->set_is_optional(true);
  parameters_.Bind<Double>(PARAM_LENGTH_BINS, &length_bins_,
                           "The length bin that recruits are uniformly distributed over at the time of recruitment. Needs to be consistent with @model length bin inputs");
  parameters_.Bind<Double>(PARAM_R0, &r0_, "R0");

  RegisterAsAddressable(PARAM_R0, &r0_);
  RegisterAsAddressable(PARAM_PROPORTIONS, &proportions_categories_);

  process_type_        = ProcessType::kRecruitment;
  partition_structure_ = PartitionType::kLength;
}

/**
 * Validate the parameters for this process
 *
 * 1. Check for the required parameters
 * 2. Assign our label from the parameters
 * 3. Assign remaining local parameters
 */
void RecruitmentConstant::DoValidate() {
  LOG_TRACE();

  parameters_.ValidateVector(PARAM_PROPORTIONS)
      ->GreaterThanOrEqualTo(0.0)
      ->LessThanOrEqualTo(1.0)
      ->SumToOne()
      ->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)
      ->SameNumberOfElementsAs(PARAM_CATEGORIES);
  parameters_.Validate(PARAM_R0)->GreaterThan(0.0);
  parameters_.ValidateVector(PARAM_LENGTH_BINS)->IsLengthBin()->ExpandToSameNumberOfElementsAs(PARAM_CATEGORIES)->SameNumberOfElementsAs(PARAM_CATEGORIES);

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
  partition_.Init(category_labels_);
}

/**
 * Execute the constant recruitment process
 */
void RecruitmentConstant::DoExecute() {
  LOG_TRACE();
  // Calculate new proportion totals to account for dynamic categories
  Double total_proportions = 0.0;
  for (auto category : partition_) total_proportions += proportions_categories_[category->name_];

  // Update our partition with new recruitment values
  for (auto category : partition_) {
    if (category->data_.size() != model_->length_bins().size())
      LOG_CODE_ERROR() << "This function was written when categories were forced to have the same length bins as models. This is no longer the case.";

    r0_by_length_bin_     = (r0_ * (proportions_categories_[category->name_]) / total_proportions) / length_bins_.size();
    unsigned length_index = 0;
    for (auto length_bin : length_bins_) {
      length_index = std::distance(std::find(model_->length_bins().begin(), model_->length_bins().end(), length_bin), model_->length_bins().begin());
      LOG_FINEST() << "putting " << r0_by_length_bin_ << " in category " << category->name_ << " in length bin " << model_->length_bins()[length_index];
      category->data_[length_index] += r0_by_length_bin_;
    }
  }
}

} /* namespace length */
}  // namespace processes
} /* namespace niwa */
