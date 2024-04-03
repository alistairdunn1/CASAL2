/**
 * @file HarvestStrategyRampU.cpp
 * @author  A Dunn
 * @date 01/04/2024
 * @section LICENSE
 *
 */

// headers
#include "HarvestStrategyRampU.h"

#include "../../DerivedQuantities/DerivedQuantity.h"
#include "../../DerivedQuantities/Manager.h"
#include "../../Model/Managers.h"
#include "../../Model/Model.h"
#include "../../Reports/Manager.h"
#include "../../Utilities/RandomNumberGenerator.h"
#include "../../Utilities/To.h"
#include "InitialisationPhases/Manager.h"

// namespaces
namespace niwa {
namespace projects {

/**
 * Default constructor
 */
HarvestStrategyRampU::HarvestStrategyRampU(shared_ptr<Model> model) : Project(model), model_(model) {
  // clang-format off
  parameters_.Bind<string>(PARAM_BIOMASS_INDEX, &biomass_index_label_, "The biomass index label (i.e., the derived quantity label)", "");
  parameters_.Bind<Double>(PARAM_U, &u_, "The exploitation rates to apply", "", 0.0)->set_lower_bound(0, true);
  parameters_.Bind<Double>(PARAM_REFERENCE_POINTS, &reference_points_, "The reference points for each exploitation rate", "", 0.0)->set_lower_bound(0, true);
  parameters_.Bind<string>(PARAM_REFERENCE_INDEX, &reference_index_label_, "The biomass index for reference points", "");
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
void HarvestStrategyRampU::DoValidate() {
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
  // check us = index
  if (u_.size() < 2) {
    LOG_ERROR_P(PARAM_U) << "should be a vector of at least 2 elements";
  }
  if (reference_points_.size() < 2) {
    LOG_ERROR_P(PARAM_REFERENCE_POINTS) << "should be a vector of at least 2 elements";
  }
  if (u_.size() != reference_points_.size()) {
    LOG_ERROR_P(PARAM_REFERENCE_POINTS) << "is not the same length as '" << PARAM_U << "'. These should be the same length. The " << PARAM_U << " parameter has " << u_.size()
                                        << " elements and the " << PARAM_REFERENCE_POINTS << " parameter has " << reference_points_.size() << " elements";
  }
  // check reference_points_ are in strictly increasing order
  for (unsigned i = 1; i < reference_points_.size(); ++i) {
    if (reference_points_[i - 1] >= reference_points_[i])
      LOG_ERROR_P(PARAM_REFERENCE_POINTS) << "values must be in strictly increasing order. Value " << reference_points_[i - 1] << " is not less than " << reference_points_[i];
  }
}

/**
 * Build
 */
void HarvestStrategyRampU::DoBuild() {
  biomass_index_   = model_->managers()->derived_quantity()->GetDerivedQuantity(biomass_index_label_);
  reference_index_ = model_->managers()->derived_quantity()->GetDerivedQuantity(reference_index_label_);
  if (!biomass_index_) {
    LOG_ERROR_P(PARAM_BIOMASS_INDEX) << "The " << PARAM_BIOMASS_INDEX << " derived_quantity (" << biomass_index_label_ << ") was not found.";
  }
  if (!reference_index_) {
    LOG_ERROR_P(PARAM_REFERENCE_INDEX) << "The " << PARAM_REFERENCE_INDEX << " derived_quantity (" << reference_index_label_ << ") was not found.";
  }
  update_counter_ = 0;
  last_catch_     = current_catch_;
}

/**
 * Reset
 */
void HarvestStrategyRampU::DoReset() {
  update_counter_ = 0;
  last_catch_     = current_catch_;
}

/**
 *  Update the parameter with the harvest control rule
 */
void HarvestStrategyRampU::DoUpdate() {
  int    index_year  = model_->current_year() - year_lag_;
  Double ref_biomass = reference_index_->GetValue(index_year) / reference_index_->GetValue(0);
  Double biomass     = biomass_index_->GetValue(index_year);
  Double u           = 0.0;
  value_             = last_catch_;

  if (model_->current_year() >= first_year_)
    update_counter_++;

  bool do_update = ((model_->current_year() == first_year_) || ((model_->current_year() > first_year_) && (update_counter_ % (int)year_delta_) == 0));

  if (do_update) {  // its in year_delta, so do an update
    // get u, given the reference biomass
    if (ref_biomass > reference_points_[0]) {
      if (ref_biomass < reference_points_[reference_points_.size() - 1]) {
        for (unsigned i = 0; i < reference_points_.size(); ++i) {
          if (reference_points_[i] > ref_biomass) {
            Double a = 0.0;
            Double b = 0.0;
            Double f = 0.0;
            a        = u_[i - 1];
            b        = u_[i];
            f        = (ref_biomass - reference_points_[i - 1]) / (reference_points_[i] - reference_points_[i - 1]);
            u        = a * (1.0 - f) + (b * f);
            break;
          }
        }
      } else {
        u = u_[u_.size() - 1];
      }
    } else {
      u = u_[0];
    }

    LOG_FINE() << "HarvestStrategyRampU: u=" << u << ", biomass_index=" << biomass << " reference_index=" << ref_biomass << " in year=" << model_->current_year()
               << " using index_year=" << index_year << " with update_counter=" << update_counter_ << ", year_delta=" << year_delta_ << ", and result of test=" << do_update;

    this_catch_  = biomass * u;
    Double delta = (this_catch_ - last_catch_) / last_catch_;
    Double sign  = (delta >= 0) ? 1.0 : -1.0;

    LOG_FINE() << "HarvestStrategyRampU: catch=" << this_catch_;

    if (fabs(AS_DOUBLE(delta)) > AS_DOUBLE(min_delta_)) {                         // change is greater than the min_delta_
      if (fabs(AS_DOUBLE(delta)) < AS_DOUBLE(max_delta_) || max_delta_ <= 0.0) {  // change is less than than the max_delta_ (but acccount for special case of max_delta_ = 0)
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
    }
  }

  LOG_FINE() << "HarvestStrategyRampU: DoUpdateFunc in year=" << model_->current_year() << ", and with biomass_index=" << biomass << " and catch value=" << value_;

  (this->*DoUpdateFunc_)(value_, true, model_->current_year());
}

} /* namespace projects */
} /* namespace niwa */
