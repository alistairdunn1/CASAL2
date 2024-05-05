/**
 * @file HarvestStrategyConstantCatch.cpp
 * @author  A Dunn
 * @date 01/04/2024
 * @section LICENSE
 *
 */

// headers
#include "HarvestStrategyConstantCatch.h"

#include "../../DerivedQuantities/DerivedQuantity.h"
#include "../../DerivedQuantities/Manager.h"
#include "../../Model/Managers.h"
#include "../../Model/Model.h"
#include "../../Reports/Manager.h"
#include "../../Utilities/Math.h"
#include "../../Utilities/RandomNumberGenerator.h"
#include "../../Utilities/To.h"
#include "InitialisationPhases/Manager.h"

// namespaces
namespace niwa {
namespace projects {

namespace math = niwa::utilities::math;

/**
 * Default constructor
 */
HarvestStrategyConstantCatch::HarvestStrategyConstantCatch(shared_ptr<Model> model) : Project(model), model_(model) {
  // clang-format off
  parameters_.Bind<string>(PARAM_BIOMASS_INDEX, &biomass_index_label_, "The biomass index label (i.e., the derived quantity label)", "");
  parameters_.Bind<Double>(PARAM_CATCH, &catch_, "The catch to apply", "", 0.0)->set_lower_bound(0, true);
  parameters_.Bind<Double>(PARAM_ALPHA, &alpha_, "The proportional change in biomass multiplier to change to the catch to apply", "", 1.0)->set_lower_bound(0, false);
  parameters_.Bind<Double>(PARAM_MIN_DELTA, &min_delta_, "The minimum difference (proportion) in catch required before it is updated", "", 0.0)->set_lower_bound(0, true);
  parameters_.Bind<Double>(PARAM_MAX_DELTA, &max_delta_, "The maximum difference (proportion) in catch that can be applied (no maximum = 0)", "", 0.0)->set_lower_bound(0, true);
  parameters_.Bind<unsigned>(PARAM_YEAR_DELTA, &year_delta_, "The number of years between updates", "", 1)->set_lower_bound(1, true);
  parameters_.Bind<unsigned>(PARAM_YEAR_LAG, &year_lag_, "The lag (years) of the derived_quantity that is used for the calculation of the catch", "", 1)->set_lower_bound(1, true);
  parameters_.Bind<Double>(PARAM_CURRENT_CATCH, &current_catch_, "The current catch to apply at the start of the projections (applied until first_year)", "", 0)->set_lower_bound(0, true);
  parameters_.Bind<Double>(PARAM_MULTIPLIER, &multiplier_, "Multiplier that is applied to the projected value", "", 1.0)->set_lower_bound(0, false);
  parameters_.Bind<unsigned>(PARAM_FIRST_YEAR, &first_year_, "The first year in which to consider an update using the harvest strategy rule", "", 0);
  // clang-format on
}

/**
 * Validate
 */
void HarvestStrategyConstantCatch::DoValidate() {
  // if only one multiplier supplied then assume its the same for all years
  if (multiplier_.size() == 1) {
    multiplier_.resize(years_.size(), multiplier_[0]);
  }
  if (multiplier_.size() != 0) {
    if (multiplier_.size() != years_.size()) {
      LOG_FATAL_P(PARAM_MULTIPLIER) << "Supply a multiplier for each year. Values for " << multiplier_.size() << " years were provided, but " << years_.size()
                                    << " years are required";
    }
    multiplier_by_year_ = utilities::Map::create(years_, multiplier_);
  } else {
    Double val          = 1.0;
    multiplier_by_year_ = utilities::Map::create(years_, val);
  }

  if (first_year_ > model_->projection_final_year()) {
    LOG_ERROR_P(PARAM_FIRST_YEAR) << "The first_year cannot be greater than the projection_final year";
  }
}

/**
 * Build
 */
void HarvestStrategyConstantCatch::DoBuild() {
  biomass_index_ = model_->managers()->derived_quantity()->GetDerivedQuantity(biomass_index_label_);
  if (!biomass_index_) {
    LOG_ERROR_P(PARAM_BIOMASS_INDEX) << "The " << PARAM_BIOMASS_INDEX << " derived_quantity (" << biomass_index_label_ << ") was not found.";
  }
  update_counter_ = 0;
  last_catch_     = current_catch_;
}

/**
 * Reset
 */
void HarvestStrategyConstantCatch::DoReset() {
  update_counter_ = 0;
  last_catch_     = current_catch_;
}

/**
 *  Update the parameter with the harvest control rule
 */
void HarvestStrategyConstantCatch::DoUpdate() {
  int    index_year       = model_->current_year() - year_lag_;
  Double biomass          = biomass_index_->GetValue(index_year);
  Double previous_biomass = biomass_index_->GetValue(index_year - year_lag_);
  value_                  = last_catch_;

  if (model_->current_year() >= first_year_)
    update_counter_++;

  bool do_update = ((model_->current_year() == first_year_) || ((model_->current_year() > first_year_) && (update_counter_ % (int)year_delta_) == 0));

  LOG_FINE() << "HarvestStrategyConstantCatch: u=" << catch_ << ", biomass_index=" << biomass << " in year=" << model_->current_year() << " using index_year=" << index_year
             << " with update_counter=" << update_counter_ << ", year_delta=" << year_delta_ << ", and result of test=" << do_update;

  if (do_update) {  // its in year_delta, so do an update
    this_catch_  = (((biomass - previous_biomass) / previous_biomass * alpha_) + 1) * catch_ * multiplier_by_year_[model_->current_year()];
    Double delta = (this_catch_ - last_catch_) / math::ZeroFun(last_catch_, math::ZERO);
    Double sign  = (delta >= 0) ? 1.0 : -1.0;

    LOG_FINE() << "HarvestStrategyConstantCatch: catch=" << this_catch_;

    if (fabs(AS_DOUBLE(delta)) >= AS_DOUBLE(min_delta_)) {                           // change is greater than the min_delta_
      if (max_delta_ <= 0.0 || (fabs(AS_DOUBLE(delta)) <= AS_DOUBLE(max_delta_))) {  // change is less than than the max_delta_ (but acccount for special case of max_delta_ = 0)
        // update the catch
        value_          = this_catch_;
        last_catch_     = value_;
        update_counter_ = 0;
      } else {  // change is greater than than the max_delta_
        // update the catch with a maximum of max_delta_
        value_          = last_catch_ * (1.0 + sign * max_delta_);
        last_catch_     = value_;
        update_counter_ = 0;
      }
    } else {
      value_ = last_catch_;
    }
  }

  LOG_FINE() << "HarvestStrategyConstantCatch: DoUpdateFunc in year=" << model_->current_year() << ", and with biomass_index=" << biomass << " and catch value=" << value_;

  (this->*DoUpdateFunc_)(value_, true, model_->current_year());
}

} /* namespace projects */
} /* namespace niwa */
