/**
 * @file Students_t.h
 * @author  A Dunn
 * @version 1.0
 * @section LICENSE
 *
 * @section DESCRIPTION
 *
 */
#ifndef ESTIMATES_STUDENTST_H_
#define ESTIMATES_STUDENTST_H_

// Headers
#include "../../Estimates/Estimate.h"

// namespaces
namespace niwa {
namespace estimates {

/**
 * Class definition
 */
class Students_t : public niwa::Estimate {
public:
  // Methods
  Students_t() = delete;
  explicit Students_t(shared_ptr<Model> model);
  virtual ~Students_t() = default;
  void           DoValidate() override final {};
  Double         GetScore() override final;
  vector<Double> GetPriorValues() override final;
  vector<string> GetPriorLabels() override final;

private:
  // Members
  Double   mu_;       // location parameter
  Double   sigma_;    // scale parameter
  unsigned df_ = 3u;  // degrees of freedom
  // Constants
};

} /* namespace estimates */
} /* namespace niwa */
#endif /* ESTIMATES_STUDENTST_H_ */
