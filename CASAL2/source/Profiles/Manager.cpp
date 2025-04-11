/**
 * @file Manager.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 28/03/2014
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "Manager.h"

// namespaces
namespace niwa {
namespace profiles {

/**
 * Default constructor
 */
Manager::Manager() {}
/**
 * Return the profile with the name passed in as a parameter.
 * If no process is found then an empty pointer will
 * be returned.
 *
 * @return A pointer to the profile or empty pointer
 */
Profile* Manager::GetProfile() {
  if (objects_.size() > 1)
    LOG_CODE_ERROR() << "Found " << objects_.size() << " @profile blocks. Casal2 can only be run with one profile block.";
  if (objects_.size() > 0) {
    return objects_[0];
  }
  return nullptr;
}

void Manager::Validate() {
  LOG_CODE_ERROR() << "This method should not be used for this manager";
}

/**
 *
 */
void Manager::Validate(shared_ptr<Model> model) {
  if (model->run_mode() == RunMode::kProfiling && objects_.size() == 0) {
    LOG_FATAL() << "No @profile blocks have been defined, even though this is an Profiling run";
  }

  // TODO: This breaks build
  // if (model->run_mode() != RunMode::kProfiling) {
  //   return;  // No need to validate
  // }

  if (objects_.size() > 1) {
    LOG_FATAL() << "Only one @profile block can be specified in a model configuration, you have specified " << objects_.size();
  }

  for (auto profile : objects_) profile->Validate();
}

} /* namespace profiles */
} /* namespace niwa */
