/**
 * @file Students_t.h
 * @author  A Dunn
 * @section LICENSE
 *
 * @section DESCRIPTION
 *
 * << Add Description >>
 */
#ifndef ESTIMATES_CREATORS_STUDENTS_T_H_
#define ESTIMATES_CREATORS_STUDENTS_T_H_

// headers
#include "../../../Estimates/Creators/Creator.h"

// namespaces
namespace niwa {
namespace estimates {
namespace creators {

/**
 *
 */
class Students_t : public estimates::Creator {
public:
  Students_t() = delete;
  explicit Students_t(shared_ptr<Model> model);
  virtual ~Students_t() = default;
  void DoCopyParameters(niwa::Estimate* estimate, unsigned index) override final;

private:
  // members
  vector<Double> mu_;
  vector<Double> sigma_;
  vector<unsigned> df_;
};

} /* namespace creators */
} /* namespace estimates */
} /* namespace niwa */

#endif /* STUDENTS_T_H_ */
