/**
 * @file Lognormal.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 24/09/2014
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "Lognormal.h"

// namespaces
namespace niwa {
namespace estimates {
namespace creators {

/**
 * Default constructor
 */
Lognormal::Lognormal(shared_ptr<Model> model) : Creator(model) {
  parameters_.Bind<Double>(PARAM_MU, &mu_, "The lognormal prior mean (mu) parameter", "");
  parameters_.Bind<Double>(PARAM_CV, &cv_, "The lognormal variance (cv) parameter", "");
}

/**
 * Copy the mu and cv parameters
 */
void Lognormal::DoCopyParameters(niwa::Estimate* estimate, unsigned index) {
  estimate->parameters().CopyFrom(parameters_, PARAM_MU, index);
  estimate->parameters().CopyFrom(parameters_, PARAM_CV, index);
}

} /* namespace creators */
} /* namespace estimates */
} /* namespace niwa */
