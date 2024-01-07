/**
 * @file Students_t.cpp
 * @author  A Dunn
 * @section LICENSE
 *
 */

// headers
#include "Students_t.h"

// namespaces
namespace niwa {
namespace estimates {
namespace creators {

/**
 * Default constructor
 */
Students_t::Students_t(shared_ptr<Model> model) : Creator(model) {
  parameters_.Bind<Double>(PARAM_MU, &mu_, "The Student's t location (mu) parameter", "");
  parameters_.Bind<Double>(PARAM_SIGMA, &sigma_, "The Student's t scale (sigma) parameter", "");
  parameters_.Bind<unsigned>(PARAM_DF, &df_, "The Student's t degrees of freedom (df) parameter", "");
}

/**
 * Copy the mu and cv parameters
 */
void Students_t::DoCopyParameters(niwa::Estimate* estimate, unsigned index) {
  estimate->parameters().CopyFrom(parameters_, PARAM_MU, index);
  estimate->parameters().CopyFrom(parameters_, PARAM_SIGMA, index);
  estimate->parameters().CopyFrom(parameters_, PARAM_DF, index);
}

} /* namespace creators */
} /* namespace estimates */
} /* namespace niwa */
