/**
 * @file RecruitmentConstant.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 14/12/2012
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * This class represents a constant recruitment class. Every year
 * fish will be recruited (bred/born etc) in to 1 age for a collection
 * of categories (Age-based) or distributed across specified length bins
 * (Length-based).
 *
 * This is a consolidated implementation that supports both Age and Length
 * partitioned models.
 */
#ifndef PROCESSES_COMMON_RECRUITMENTCONSTANT_H_
#define PROCESSES_COMMON_RECRUITMENTCONSTANT_H_

// Headers
#include "Partition/Accessors/Categories.h"
#include "Partition/Accessors/CategoriesWithAge.h"
#include "Processes/Process.h"
#include "Utilities/Types.h"

// Namespaces
namespace niwa::processes::common {

using niwa::partition::accessors::CategoriesWithAgePtr;
using niwa::utilities::Double;
namespace accessor = niwa::partition::accessors;
using utilities::OrderedMap;

/**
 * Class definition
 */
class RecruitmentConstant : public niwa::Process {
public:
  // Methods
  explicit RecruitmentConstant(shared_ptr<Model> model);
  virtual ~RecruitmentConstant() = default;
  void DoValidate() override final;
  void DoBuild() override final;
  void DoReset() override final {};
  void DoExecute() override final;
  void FillReportCache(ostringstream& cache) override final;
  void FillTabularReportCache(ostringstream& cache, bool first_run) override final;

  // accessors
  const vector<string>& category_labels() const { return category_labels_; }

private:
  // Members
  vector<string>             category_labels_;
  vector<Double>             proportions_;
  OrderedMap<string, Double> proportions_categories_;
  Double                     r0_;

  // Age-specific members
  unsigned             age_;
  CategoriesWithAgePtr partition_age_;

  // Length-specific members
  vector<Double>       length_bins_;
  accessor::Categories partition_length_;
  Double               r0_by_length_bin_;
};

} /* namespace niwa::processes::common */
#endif /* PROCESSES_COMMON_RECRUITMENTCONSTANT_H_ */
