/*
 * Constant.Mock.h
 *
 *  Created on: 28/01/2019
 *      Author: Zaita
 */

#ifndef SOURCE_TIMEVARYING_COMMON_CONSTANT_MOCK_H_
#define SOURCE_TIMEVARYING_COMMON_CONSTANT_MOCK_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Constant.h"

namespace niwa {
namespace timevarying {

class MockConstant : public timevarying::Constant {
public:
  // methods
  MockConstant(shared_ptr<Model> model) : Constant(model){};
  virtual ~MockConstant() = default;

  // setters
  void set_values(vector<Double> values) { values_ = values; }
};

} /* namespace timevarying */
} /* namespace niwa */

#endif /* SOURCE_TIMEVARYING_COMMON_CONSTANT_MOCK_H_ */
