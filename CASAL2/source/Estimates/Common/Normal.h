/**
 * @file Normal.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 8/03/2013
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
#ifndef ESTIMATES_NORMAL_H_
#define ESTIMATES_NORMAL_H_

// Headers
#include "../../Estimates/Estimate.h"

// namespaces
namespace niwa {
namespace estimates {

/**
 * Class definition
 */
class Normal : public niwa::Estimate {
public:
  // Methods
  Normal() = delete;
  explicit Normal(shared_ptr<Model> model);
  virtual ~Normal() = default;
  void   DoValidate() override final{};
  Double GetScore() override final;
  vector<Double>   GetPriorValues() override final;
  vector<string>   GetPriorLabels() override final;
  
private:
  // Members
  Double mu_;
  Double cv_;
};

} /* namespace estimates */
} /* namespace niwa */
#endif /* ESTIMATES_NORMAL_H_ */
