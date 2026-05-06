/**
 * @file TransitionCategoryByAge.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 10/12/2014
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 */

// headers
#include "TransitionCategoryByAge.h"

#include <limits>
#include <numeric>

#include "../../Utilities/Math.h"
#include "Categories/Categories.h"
#include "Model/Model.h"
#include "Penalties/Manager.h"
#include "Selectivities/Manager.h"
#include "Utilities/To.h"

// namespaces
namespace niwa {
namespace processes {
namespace age {

/**
 * Default constructor
 */
TransitionCategoryByAge::TransitionCategoryByAge(shared_ptr<Model> model) : Process(model), to_partition_(model), from_partition_(model) {
  process_type_        = ProcessType::kTransition;
  partition_structure_ = PartitionType::kAge;

  n_table_ = parameters_.BindTable(PARAM_N, "The table of N data");
  n_table_->set_required_columns({PARAM_YEAR}, true);

  // clang-format off
  parameters_.Bind<string>(PARAM_FROM, &from_category_labels_, "The categories to transition from")->flag_is_category();
  parameters_.Bind<string>(PARAM_TO, &to_category_labels_, "The categories to transition to")->flag_is_category();
  parameters_.Bind<unsigned>(PARAM_MIN_AGE, &min_age_, "The minimum age to transition");
  parameters_.Bind<unsigned>(PARAM_MAX_AGE, &max_age_, "The maximum age to transition");
  parameters_.Bind<string>(PARAM_PENALTY, &penalty_label_, "The penalty label")->set_default_value("");
  parameters_.Bind<Double>(PARAM_U_MAX, &u_max_, "The maximum exploitation rate ($U_{max}$)")->set_default_value(0.99); // 
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years to execute the transition in");

  // clang-format on
}

/**
 * Validate the parameters
 */
void TransitionCategoryByAge::DoValidate() {
  parameters_.ValidateVector(PARAM_FROM)->SameNumberOfElementsAs(PARAM_TO)->IsUniqueFrom(PARAM_TO);
  parameters_.ValidateVector(PARAM_YEARS)->IsInIncreasingOrder()->IsModelYear()->DefaultToAllModelYears();
  parameters_.Validate(PARAM_MIN_AGE)->GreaterThanOrEqualTo(model()->min_age());
  parameters_.Validate(PARAM_MAX_AGE)->GreaterThanOrEqualTo(model()->min_age())->LessThanOrEqualTo(model()->max_age());
  parameters_.Validate(PARAM_U_MAX)->GreaterThan(0.0)->LessThanOrEqualTo(1.0);

  unsigned age_spread = (max_age_ - min_age_) + 1;

  /**
   * Load our N data in to the map
   */
  vector<string> columns = n_table_->columns();
  if (columns.size() != age_spread + 1)
    LOG_ERROR_P(PARAM_N) << "the number of columns provided (" << columns.size() << ") does not match the model's age spread + 1 (" << (age_spread + 1) << ")";
  if (columns[0] != PARAM_YEAR)
    LOG_ERROR_P(PARAM_N) << "the first column label (" << columns[0] << ") provided must be 'year'";

  map<unsigned, unsigned> age_index;
  for (unsigned i = 1; i < columns.size(); ++i) {
    unsigned age = 0;
    if (!utilities::To<unsigned>(columns[i], age))
      LOG_ERROR() << "";
    age_index[age] = i;
  }

  // load our table data in to our map
  vector<vector<string>> data    = n_table_->data();
  unsigned               year    = 0;
  Double                 n_value = 0.0;
  for (auto iter : data) {
    if (!utilities::To<unsigned>(iter[0], year))
      LOG_ERROR_P(PARAM_N) << "value (" << iter[0] << ") could not be converted to an unsigned integer";
    for (unsigned i = 1; i < iter.size(); ++i) {
      if (!utilities::To<Double>(iter[i], n_value))
        LOG_ERROR_P(PARAM_N) << "value (" << iter[i] << ") could not be converted to a Double.";
      if (n_[year].size() == 0)
        n_[year].resize(age_spread, 0.0);
      n_[year][i - 1] = n_value;
    }
  }
  // Validate no categories are in both to_ and from_
  for (unsigned i = 0; i < to_category_labels_.size(); ++i) {
    for (unsigned j = 0; j < from_category_labels_.size(); ++j) {
      if (to_category_labels_[i] == from_category_labels_[j]) {
        LOG_ERROR_P(PARAM_TO) << ": A 'from' category (" << from_category_labels_[j] << ") cannot be the same as a 'to' category (" << to_category_labels_[i] << ")";
      }
    }
  }
}

/**
 * Build the partition objects
 */
void TransitionCategoryByAge::DoBuild() {
  from_partition_.Init(from_category_labels_);
  to_partition_.Init(to_category_labels_);

  if (penalty_label_ != "")
    penalty_ = model()->managers()->penalty()->GetPenalty(penalty_label_);
  if (selectivity_label_ != "")
    selectivity_ = model()->managers()->selectivity()->GetSelectivity(selectivity_label_);
}

/**
 * Execute the process
 */
void TransitionCategoryByAge::DoExecute() {
  unsigned current_year = model()->current_year();
  if (std::find(years_.begin(), years_.end(), current_year) == years_.end())
    return;

  LOG_FINEST() << "n_.size(): " << n_.size();
  LOG_FINEST() << "n_[current_year].size()" << n_[current_year].size();
  for (unsigned i = 0; i < n_[current_year].size(); ++i) LOG_FINEST() << "n_[current_year][" << i << "]: " << n_[current_year][i];

  Double   total_stock;
  unsigned age_spread = (max_age_ = min_age_) + 1;
  LOG_FINEST() << "age_spread: " << age_spread << " in year " << current_year;

  for (unsigned i = 0; i < age_spread; ++i) {
    /**
     * Calculate the Exploration rate
     */
    auto from_iter = from_partition_.begin();
    auto to_iter   = to_partition_.begin();

    total_stock = 0.0;
    for (; from_iter != from_partition_.end(); from_iter++, to_iter++) {
      unsigned total_stock_offset = (min_age_ - (*from_iter)->min_age_) + i;
      LOG_FINE() << "total_stock_offset: " << total_stock_offset << " (" << (*from_iter)->min_age_ << " - " << min_age_ << ") + " << i;
      total_stock += (*from_iter)->data_[total_stock_offset];
    }
    LOG_FINE() << "total_stock: " << total_stock << " at age " << min_age_ + i;

    Double exploitation = n_[current_year][i] / utilities::math::ZeroFun(total_stock);
    LOG_FINE() << "exploitation: " << exploitation << "; n: " << n_[current_year][i];
    if (exploitation > u_max_) {
      exploitation = u_max_;
      if (penalty_)
        penalty_->Trigger(n_[current_year][i], total_stock * u_max_);
    } else if (exploitation < 0.0)
      exploitation = 0.0;

    /**
     * Migrate the exploited amount
     */
    from_iter = from_partition_.begin();
    to_iter   = to_partition_.begin();
    for (; from_iter != from_partition_.end(); from_iter++, to_iter++) {
      unsigned offset  = (min_age_ - (*from_iter)->min_age_) + i;
      Double   current = (*from_iter)->data_[offset] * exploitation;
      LOG_FINE() << "current: " << current << "; exploitation: " << exploitation;
      if (current <= 0.0)
        continue;

      (*from_iter)->data_[offset] -= current;
      (*to_iter)->data_[offset] += current;
    }
  }
}

} /* namespace age */
} /* namespace processes */
} /* namespace niwa */
