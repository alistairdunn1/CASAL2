/**
 * @file Factory.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 6/03/2013
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * The time class represents a moment of time.
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "Factory.h"

#include "../Estimates/Common/Beta.h"
#include "../Estimates/Common/Lognormal.h"
#include "../Estimates/Common/Normal.h"
#include "../Estimates/Common/NormalByStdev.h"
#include "../Estimates/Common/NormalLog.h"
#include "../Estimates/Common/Uniform.h"
#include "../Estimates/Common/UniformLog.h"
#include "../Estimates/Manager.h"
#include "../Model/Managers.h"
#include "../Model/Model.h"

// Namespaces
namespace niwa {
namespace estimates {

/**
 * Create the instance of our object as defined by the two parameters
 * object_type and sub_type.
 *
 * @param object_type The type of object to create (e.g., age_size, process)
 * @param sub_type The child type of the object to create (e.g., ageing, schnute)
 * @return shared_ptr to the object
 */
Estimate* Factory::Create(shared_ptr<Model> model, const string& object_type, const string& sub_type) {
  Estimate* result = nullptr;

  if (object_type == PARAM_ESTIMATE) {
    if (sub_type == PARAM_BETA)
      result = new Beta(model);
    else if (sub_type == PARAM_LOGNORMAL)
      result = new Lognormal(model);
    else if (sub_type == PARAM_NORMAL)
      result = new Normal(model);
    else if (sub_type == PARAM_NORMAL_BY_STDEV)
      result = new NormalByStdev(model);
    else if (sub_type == PARAM_NORMAL_LOG)
      result = new NormalLog(model);
    else if (sub_type == PARAM_UNIFORM)
      result = new Uniform(model);
    else if (sub_type == PARAM_UNIFORM_LOG)
      result = new UniformLog(model);

    if (result)
      model->managers()->estimate()->AddObject(result);
  }

  return result;
}

} /* namespace estimates */
} /* namespace niwa */
