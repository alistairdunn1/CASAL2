/**
 * @file Accessor.cpp
 * @author  Scott Rasmussen (scott@zaita.com)
 * @date 14/01/2020
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/�2019 - www.niwa.co.nz
 */

// headers
#include "Accessor.h"

#include "../Model/Model.h"

// namespaces
namespace niwa {
namespace partition {

/**
 * Default constructor
 */
Accessor::Accessor(shared_ptr<Model> model) : model_(model) {}

/**
 * Init our accessor
 */
void Accessor::Init(const vector<string>& category_labels) {
  LOG_TRACE();

  unsigned start_year = model()->start_year();
  unsigned final_year = model()->final_year();

  if (model()->run_mode() == RunMode::kProjection)
    final_year = model()->projection_final_year();

  LOG_FINEST() << "Model details: start_year: " << start_year << "; final_year: " << final_year;
  LOG_FINEST() << "Categories: " << category_labels.size();

  Partition& partition = model()->partition();
  for (string category_label : category_labels) {
    partition::Category& category = partition.category(category_label);
    for (unsigned year = start_year; year <= final_year; ++year) {
      if (std::find(category.years_.begin(), category.years_.end(), year) == category.years_.end())
        continue;  // Not valid in this year

      categories_[year].push_back(PartitionIterable(&category, category.data_));
    }
  }
}

//
Accessor::DataType::iterator Accessor::begin() {
  return categories_[model()->current_year()].begin();
}

Accessor::DataType::iterator Accessor::end() {
  return categories_[model()->current_year()].end();
}

}  // namespace partition
}  // namespace niwa
