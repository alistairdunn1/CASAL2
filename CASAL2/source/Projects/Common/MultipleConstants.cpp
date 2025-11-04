/**
 * @file MultipleConstants.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 28/05/2014
 * @section LICENSE
 *
 * Copyright NIWA Science �2014 - www.niwa.co.nz
 *
 */

// headers
#include "MultipleConstants.h"

#include "../../Model/Managers.h"
#include "../../Utilities/To.h"
#include "../../Utilities/Vector.h"

// namespaces
namespace niwa {
namespace projects {

/**
 * Default constructor
 */
MultipleConstants::MultipleConstants(shared_ptr<Model> model) : Project(model) {
  data_table_ = parameters_.BindTable(PARAM_VALUES, "Table of values for each -i input. Rows are -i value columns are for each year to project for.");
  parameters_.Bind<Double>(PARAM_MULTIPLIER, &multiplier_, "Multiplier that is applied to the projected value")->set_is_optional(true);
}

/**
 * Validate
 */
void MultipleConstants::DoValidate() {
  LOG_TRACE();
  parameters_.ValidateVector(PARAM_MULTIPLIER)
      ->DefaultValue(1.0, years_.size())
      ->ExpandToSameNumberOfElementsAs(PARAM_YEARS)
      ->SameNumberOfElementsAs(PARAM_YEARS)
      ->GreaterThanOrEqualTo(0.0);
  multiplier_by_year_ = utilities::Map::create(years_, multiplier_);
}

/**
 * Build
 */
void MultipleConstants::DoBuild() {
  LOG_TRACE();
  // basic validation
  vector<vector<string>>& data = data_table_->data();
  LOG_FINE() << "In MultipleConstants projections: -i count " << model_->get_addressable_values_count();

  if (data.size() != (model_->get_addressable_values_count()))
    LOG_ERROR_P(PARAM_VALUES) << "- the number of rows supplied was " << data.size() << ", but " << model_->get_addressable_values_count()
                              << " were expected. There should be a row for each row of data in supplied free parameter (the -i/-I file)";

  projection_values_.resize(data.size());
  // Build our projected data values
  unsigned counter = 0;
  for (vector<string> row : data) {
    for (unsigned i = 0; i < row.size(); ++i) {
      projection_values_[counter][years_[i]] = utilities::ToInline<string, Double>(row[i]);
      LOG_FINE() << "In MultipleConstants projections: year = " << years_[i] << ", value = " << projection_values_[counter][years_[i]] << " \n";
    }
    counter++;
  }

  // Basic validation
  for (unsigned ndx = 0; ndx < projection_values_.size(); ++ndx) {
    if (projection_values_[ndx].size() != years_.size())
      LOG_ERROR_P(PARAM_VALUES) << "- row " << ndx + 1 << " has " << projection_values_[ndx].size()
                                << " columns, but should have the same number of columns as the number of years (" << years_.size() << ")";
  }
}

/**
 * Reset
 */
void MultipleConstants::DoReset() {}

/**
 * Update
 */
void MultipleConstants::DoUpdate() {
  value_ = projection_values_[model_->get_current_addressable_value()][model_->current_year()] * multiplier_by_year_[model_->current_year()];
  LOG_FINE() << "In MultipleConstants projections: setting Value to: " << value_ << ", with multiplier: " << multiplier_by_year_[model_->current_year()] << " and dash -i index "
             << model_->get_current_addressable_value() + 1 << " for year = " << model_->current_year();
  (this->*DoUpdateFunc_)(value_, true, model_->current_year());
}

} /* namespace projects */
} /* namespace niwa */
