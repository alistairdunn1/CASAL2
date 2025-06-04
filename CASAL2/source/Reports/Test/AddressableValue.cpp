/**
 * @file AddressableValue.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 2025/04/07
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */

// Headers
#include "AddressableValue.h"

#include "Model/Model.h"
#include "Model/Objects.h"

// Namespaces
namespace niwa::reports::test {

/**
 * Default constructor
 */
AddressableValue::AddressableValue() {
  run_mode_    = (RunMode::Type)(RunMode::kBasic | RunMode::kProjection | RunMode::kTesting);
  model_state_ = State::kExecute;
  skip_tags_   = true;

  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_, "Time Step label");
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "Years")->set_is_optional(true);
  parameters_.Bind<string>(PARAM_ADDRESSABLE, &addressable_label_, "Addressable to monitor");
}

/**
 *
 */
void AddressableValue::DoValidate(shared_ptr<Model> model) {
  parameters_.ValidateVector(PARAM_YEARS)->DefaultToAllModelYears()->IsModelYear();
}

/**
 *
 */
void AddressableValue::DoBuild(shared_ptr<Model> model) {
  addressable_ = model->objects().GetAddressable(addressable_label_);
}

/**
 *
 * Execute the report
 */
void AddressableValue::DoExecute(shared_ptr<Model> model) {
  values_[model->current_year()] = *addressable_;
}

}  // namespace niwa::reports::test
