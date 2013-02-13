/**
 * @file ConstantRecruitment.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 14/12/2012
 * @section LICENSE
 *
 * Copyright NIWA Science �2012 - www.niwa.co.nz
 *
 * @section DESCRIPTION
 *
 * The time class represents a moment of time.
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */
#ifndef CONSTANTRECRUITMENT_H_
#define CONSTANTRECRUITMENT_H_

// Headers
#include "Partition/Accessors/CategoriesWithAge.h"
#include "Processes/Process.h"
#include "Utilities/Types.h"

// Namespaces
namespace isam {
namespace processes {

using isam::utilities::Double;
using isam::partition::accessors::CategoriesWithAgePtr;

/**
 * Class definition
 */
class RecruitmentConstant : public isam::Process {
public:
  // Methods
  RecruitmentConstant();
  virtual                     ~RecruitmentConstant() = default;
  void                        Validate();
  void                        Build();
  void                        Execute();

private:
  // Members
  vector<string>              category_names_;
  map<string, Double>         proportions_;
  unsigned                    r0_;
  unsigned                    age_;
  CategoriesWithAgePtr        partition_;
};

} /* namespace processes */
} /* namespace isam */
#endif /* CONSTANTRECRUITMENT_H_ */
