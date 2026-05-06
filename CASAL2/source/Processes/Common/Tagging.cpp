/**
 * @file Tagging.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 26/01/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * Consolidated implementation for both Age and Length partitioned models.
 */

// headers
#include "Tagging.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim_all.hpp>

#include "Categories/Categories.h"
#include "Penalties/Manager.h"
#include "Selectivities/Manager.h"
#include "Utilities/Math.h"
#include "Utilities/To.h"

// namespaces
namespace niwa::processes::common {

/**
 * Default constructor
 *
 * Why this was changed from type transition to mortality. CASAL includes this in the 'mortality block'
 * CASAL reference see population_section.cpp line: 1924-2006
 * There is mortality in this process, so does make sense
 */
Tagging::Tagging(shared_ptr<Model> model) : Process(model), to_partition_(model), from_partition_(model) {
  process_type_        = ProcessType::kMortality;
  partition_structure_ = PartitionType::kAge | PartitionType::kLength;

  numbers_table_ = parameters_.BindTable(PARAM_NUMBERS, "The data table of numbers to tag");
  numbers_table_->set_requires_columns(false);
  numbers_table_->set_is_optional(true);
  proportions_table_ = parameters_.BindTable(PARAM_PROPORTIONS, "The data table of proportions to tag");
  proportions_table_->set_is_optional(true);

  // clang-format off
  parameters_.Bind<string>(PARAM_FROM, &from_category_labels_, "The categories that are selected for tagging (i.e., transition from)")
    ->flag_is_category(true);
  parameters_.Bind<string>(PARAM_TO, &to_category_labels_, "The categories that are being tagged (i.e., transition to)")
    ->flag_is_category();
  parameters_.Bind<unsigned>(PARAM_MIN_AGE, &min_age_, "The minimum age to transition")
    ->set_is_optional(true);
  parameters_.Bind<unsigned>(PARAM_MAX_AGE, &max_age_, "The maximum age to transition")
    ->set_is_optional(true);
  parameters_.Bind<string>(PARAM_PENALTY, &penalty_label_, "The penalty label")
    ->set_default_value("");
  parameters_.Bind<double>(PARAM_U_MAX, &u_max_, "The maximum exploitation rate, U_max")
    ->set_default_value(0.99);
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "The years to execute the tagging events in");
  parameters_.Bind<Double>(PARAM_INITIAL_MORTALITY, &initial_mortality_, "The initial mortality to apply to tags as a proportion")
    ->set_default_value(0.0);
  parameters_.Bind<string>(PARAM_INITIAL_MORTALITY_SELECTIVITY, &initial_mortality_selectivity_label_, "The initial mortality selectivity label")
    ->set_default_value("");
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_labels_, "Selectivity of the tagging process");
  parameters_.Bind<Double>(PARAM_N, &n_, "The number of individuals tagged")
    ->set_is_optional(true);
  parameters_.Bind<double>(PARAM_TOLERANCE, &tolerance_, "Tolerance for checking the specificed proportions sum to one")
    ->set_default_value(1e-5);
  // clang-format on
}

/**
 * Validate the parameters from the configuration file
 */
void Tagging::DoValidate() {
  LOG_TRACE();

  if (process_profile_ == ProcessProfile::kAge) {
    parameters_.Validate(PARAM_MIN_AGE)->IsAge()->GreaterThanOrEqualTo(model()->min_age())->LessThanOrEqualTo(model()->max_age())->DefaultValue(model()->min_age());
    parameters_.Validate(PARAM_MAX_AGE)->IsAge()->GreaterThanOrEqualTo(model()->min_age())->LessThanOrEqualTo(model()->max_age())->DefaultValue(model()->max_age());
  } else {
    parameters_.ValidateVector(PARAM_FROM)->SameNumberOfElementsAs(PARAM_TO);
  }

  parameters_.Validate(PARAM_U_MAX)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);
  parameters_.ValidateVector(PARAM_YEARS)->IsModelYear()->IsInIncreasingOrder()->DefaultToAllModelYears();
  parameters_.Validate(PARAM_INITIAL_MORTALITY)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);
  parameters_.Validate(PARAM_TOLERANCE)->GreaterThanOrEqualTo(0.0)->LessThanOrEqualTo(1.0);
  parameters_.ValidateVector(PARAM_SELECTIVITIES)->SameNumberOfElementsAs(PARAM_TO);
  parameters_.ValidateVector(PARAM_N)->GreaterThanOrEqualTo(0.0)->ExpandToSameNumberOfElementsAs(PARAM_YEARS);

  if (process_profile_ == ProcessProfile::kAge) {
    age_spread_        = (max_age_ - min_age_) + 1;
    number_categories_ = from_category_labels_.size();
    min_age_offset_    = min_age_ - model()->min_age();
  }

  if (tolerance_ > 0.01) {
    LOG_WARNING_P(PARAM_TOLERANCE) << "Your tolerance is larger than " << 0.01
                                   << " this mean Casal2 may tag less fish than you think it should. It is recommended that you pick a smaller number.";
  }

  // Get our first year
  first_year_ = years_[0];

  if (process_profile_ == ProcessProfile::kAge) {
    // Age-specific validation
    // validate tables
    if (numbers_table_->row_count() != 0 && proportions_table_->row_count() != 0)
      LOG_ERROR() << location() << " cannot have both a " << PARAM_NUMBERS << " and " << PARAM_PROPORTIONS << " table defined. Please use one only.";
    if (numbers_table_->row_count() == 0) {
      if (proportions_table_->row_count() == 0) {
        LOG_ERROR() << location() << " must have either a " << PARAM_NUMBERS << " or " << PARAM_PROPORTIONS << " table defined with appropriate data";
      } else {
        if (!parameters_.Get(PARAM_N)->has_been_defined())
          LOG_ERROR() << location() << " cannot have a " << PARAM_PROPORTIONS << " table without defining " << PARAM_N;
        if (n_.size() != years_.size())
          LOG_FATAL_P(PARAM_N) << "The number of values provided (" << n_.size() << ") does not match the number of years (" << years_.size() << ")";
        // Create n_by_year_ map for proportions table case
        n_by_year_ = utilities::Map::create(years_, n_);
      }
    } else {
      if (parameters_.Get(PARAM_N)->has_been_defined())
        LOG_FATAL_P(PARAM_N) << "If a table of numbers (using subcommand table numbers) is provided, then the subcommand N should be omitted";
    }

    if (from_category_labels_.size() != 1) {
      LOG_FATAL_P(PARAM_FROM) << "This process cannot specify a many-to-many tagging event. If proportions are tagged by category then create a @tag process."
                              << " 'From' category labels size " << from_category_labels_.size();
    }

    split_from_category_labels_ = model()->categories()->SplitCombinedCategory(from_category_labels_[0], parameters_.location(PARAM_FROM));
    if (split_from_category_labels_.size() != to_category_labels_.size()) {
      LOG_FATAL_P(PARAM_FROM) << "The number of 'From' categories (" << split_from_category_labels_.size() << ") does not match the number of 'To' categories ("
                              << to_category_labels_.size() << "). These must be the same.";
    }

    // Load our table data
    if (numbers_table_->row_count() != 0) {
      if (numbers_table_->row_count() != years_.size())
        LOG_FATAL_P(PARAM_NUMBERS) << "do not match the number of years. The number of rows supplied was " << numbers_table_->row_count()
                                   << " and the number of years supplied was " << years_.size() << ". These need to be the same";

      // Load data from numbers table and calculate n
      map<unsigned, Double> n_by_year_local = utilities::Map::create(years_, 0.0);

      // load our table data in to our map
      vector<vector<string>> data    = numbers_table_->data();
      unsigned               year    = 0;
      Double                 n_value = 0.0;

      for (auto iter : data) {
        if (!utilities::To<unsigned>(iter[0], year))
          LOG_ERROR_P(PARAM_NUMBERS) << " value (" << iter[0] << ") could not be converted to an unsigned integer";

        if ((iter.size() - 1) != age_spread_) {
          LOG_FATAL_P(PARAM_NUMBERS) << "for row in year = " << year << ". The ages for this process are defined as " << min_age_ << " to " << max_age_
                                     << ". A column is required for each, which is " << age_spread_ << " columns. This table supplied " << iter.size() - 1 << " values.";
        }

        for (unsigned i = 1; i < iter.size(); ++i) {
          if (!utilities::To<Double>(iter[i], n_value))
            LOG_ERROR_P(PARAM_NUMBERS) << "value (" << iter[i] << ") could not be converted to a Double";
          if (numbers_[year].size() == 0)
            numbers_[year].resize(age_spread_, 0.0);
          n_by_year_local[year] += n_value;
          numbers_[year][i - 1] = n_value;
        }
        // Check years allign
        for (auto iter : numbers_) {
          if (std::find(years_.begin(), years_.end(), iter.first) == years_.end())
            LOG_ERROR_P(PARAM_NUMBERS) << "table contains year " << iter.first << " which is not a valid year defined in this process";
        }
      }
    } else if (proportions_table_->row_count() != 0) {
      if (proportions_table_->row_count() != years_.size()) {
        LOG_FATAL_P(PARAM_NUMBERS) << "do not match the number of years. The number of rows supplied was " << numbers_table_->row_count()
                                   << " and the number of years supplied was " << years_.size() << ". These need to be the same";
      }

      // load our table data in to our map
      vector<vector<string>> data       = proportions_table_->data();
      unsigned               year       = 0;
      Double                 proportion = 0.0;

      for (auto iter : data) {
        if ((iter.size() - 1) != age_spread_) {
          LOG_FATAL_P(PARAM_PROPORTIONS) << "For row in year = " << year << ". The number of age values for this process should be " << age_spread_ << "'. This table supplied '"
                                         << iter.size() - 1 << "'.";
        }

        LOG_ERROR_P(PARAM_PROPORTIONS) << " value (" << iter[0] << ") could not be converted to an unsigned integer";

        Double total_proportion = 0.0;
        if (!utilities::To<unsigned>(iter[0], year))
          LOG_ERROR_P(PARAM_PROPORTIONS) << "value (" << iter[0] << ") could not be converted to an unsigned integer";

        for (unsigned i = 1; i < iter.size(); ++i) {
          if (!utilities::To<Double>(iter[i], proportion))
            LOG_ERROR_P(PARAM_PROPORTIONS) << "value (" << iter[i] << ") could not be converted to a double.";
          if (numbers_[year].size() == 0)
            numbers_[year].resize(age_spread_, 0.0);
          numbers_[year][i - 1] = n_by_year_[year] * proportion;
          if (proportion < 0.0)
            LOG_ERROR_P(PARAM_PROPORTIONS) << "value (" << iter[i] << ") cannot be less than zero.";
          total_proportion += proportion;
        }

        // check the sum of proportions is equal to one
        if (fabs(1.0 - total_proportion) > tolerance_)
          LOG_ERROR_P(PARAM_PROPORTIONS) << "total (" << total_proportion << ") do not sum to 1.0 for year " << year;
      }
      for (auto iter : numbers_) {
        if (std::find(years_.begin(), years_.end(), iter.first) == years_.end())
          LOG_ERROR_P(PARAM_NUMBERS) << "table contains year " << iter.first << " which is not a valid year defined in this process";
      }
    }

    // Create containers for reporting data
    tagged_fish_after_init_mort_.resize(years_.size());
    actual_tagged_fish_to_.resize(years_.size());

    for (unsigned year_ndx = 0; year_ndx < years_.size(); ++year_ndx) {
      tagged_fish_after_init_mort_[year_ndx].resize(split_from_category_labels_.size());
      actual_tagged_fish_to_[year_ndx].resize(to_category_labels_.size());
      for (unsigned from_category_ndx = 0; from_category_ndx < split_from_category_labels_.size(); ++from_category_ndx)
        tagged_fish_after_init_mort_[year_ndx][from_category_ndx].resize(age_spread_, 0.0);
      for (unsigned to_category_ndx = 0; to_category_ndx < to_category_labels_.size(); ++to_category_ndx)
        actual_tagged_fish_to_[year_ndx][to_category_ndx].resize(age_spread_, 0.0);
    }
  } else {
    // Length-specific validation
    // Load data from proportions table using n parameter
    vector<string> columns = proportions_table_->columns();
    if (columns.size() != (model()->get_number_of_length_bins() + 1))
      LOG_ERROR_P(PARAM_PROPORTIONS) << "The number of columns provided (" << columns.size() << ") does not match the model's length bins + 1 ("
                                     << (model()->get_number_of_length_bins() + 1) << ")";
    if (columns[0] != PARAM_YEAR)
      LOG_ERROR_P(PARAM_PROPORTIONS) << "The first column label (" << columns[0] << ") provided must be 'year'";

    // build a map of n data by year
    if (n_.size() == 1) {
      auto val_n = n_[0];
      n_.assign(years_.size(), val_n);
    } else if (n_.size() != years_.size())
      LOG_ERROR_P(PARAM_N) << "The number of values provided (" << n_.size() << ") does not match the number of years (" << years_.size() << ")";

    map<unsigned, Double> n_by_year = utilities::Map::create(years_, n_);
    // load our table data in to our map
    vector<vector<string>> data       = proportions_table_->data();
    unsigned               year       = 0;
    Double                 proportion = 0.0;
    for (auto iter : data) {
      if (!utilities::To<unsigned>(iter[0], year))
        LOG_ERROR_P(PARAM_PROPORTIONS) << "value (" << iter[0] << ") could not be converted to an unsigned integer";
      Double total_proportion = 0.0;
      for (unsigned i = 1; i < iter.size(); ++i) {
        if (!utilities::To<Double>(iter[i], proportion))
          LOG_ERROR_P(PARAM_PROPORTIONS) << "value (" << iter[i] << ") could not be converted to a Double";
        if (numbers_[year].size() == 0)
          numbers_[year].resize(model()->get_number_of_length_bins(), 0.0);
        numbers_[year][i - 1] = n_by_year[year] * proportion;
        total_proportion += proportion;
      }
      if (fabs(1.0 - total_proportion) > tolerance_)
        LOG_ERROR_P(PARAM_PROPORTIONS) << "total (" << total_proportion << ") do not sum to 1.0 for year " << year;
    }

    for (auto iter : numbers_) {
      if (std::find(years_.begin(), years_.end(), iter.first) == years_.end())
        LOG_ERROR_P(PARAM_PROPORTIONS) << "table contains year " << iter.first << " which is not a valid year defined in this process";
    }
  }
}

/**
 * Build relationships between this object and others
 */
void Tagging::DoBuild() {
  LOG_TRACE();

  if (process_profile_ == ProcessProfile::kAge) {
    from_partition_.Init(split_from_category_labels_);
    to_partition_.Init(to_category_labels_);

    LOG_FINE() << "number of 'From' categories = " << from_partition_.size();
    numbers_at_age_by_category_.resize(from_partition_.size());
    selected_numbers_at_age_by_category_.resize(from_partition_.size());
    exploitation_by_age_category_.resize(from_partition_.size());
    tag_to_fish_by_category_age_.resize(from_partition_.size());
    for (unsigned i = 0; i < numbers_at_age_by_category_.size(); ++i) {
      numbers_at_age_by_category_[i].resize(age_spread_, 0.0);
      exploitation_by_age_category_[i].resize(age_spread_, 0.0);
      selected_numbers_at_age_by_category_[i].resize(age_spread_, 0.0);
      tag_to_fish_by_category_age_[i].resize(age_spread_, 0.0);
    }
  } else {
    from_partition_.Init(from_category_labels_);
    to_partition_.Init(to_category_labels_);
  }

  if (penalty_label_ != "")
    penalty_ = model()->managers()->penalty()->GetPenalty(penalty_label_);
  else
    LOG_WARNING() << location() << "No penalty has been specified. Attempting to tag fish above u_max of the vulnerable population will not affect the objective function";

  selectivities::Manager& selectivity_manager = *model()->managers()->selectivity();

  if (process_profile_ == ProcessProfile::kAge) {
    for (unsigned i = 0; i < selectivity_labels_.size(); ++i) {
      Selectivity* selectivity = selectivity_manager.GetSelectivity(selectivity_labels_[i]);
      LOG_FINEST() << "selectivity : " << selectivity_labels_[i];
      if (!selectivity)
        LOG_ERROR_P(PARAM_SELECTIVITIES) << "- the selectivity with label '" << selectivity_labels_[i] << "' was not found";
      selectivities_[split_from_category_labels_[i]] = selectivity;
    }

    if (initial_mortality_selectivity_label_ != "") {
      initial_mortality_selectivity_ = selectivity_manager.GetSelectivity(initial_mortality_selectivity_label_);
      LOG_FINEST() << "initial_mortality_selectivity_ : " << initial_mortality_selectivity_;
      if (!initial_mortality_selectivity_)
        LOG_ERROR_P(PARAM_INITIAL_MORTALITY_SELECTIVITY) << "- the selectivity with label '" << initial_mortality_selectivity_ << "' was not found";
    }

    tagged_fish_by_year_.resize(years_.size(), 0.0);
    for (unsigned year_ndx = 0; year_ndx < years_.size(); ++year_ndx) {
      for (unsigned age_ndx = 0; age_ndx < age_spread_; ++age_ndx) tagged_fish_by_year_[year_ndx] += numbers_[years_[year_ndx]][age_ndx];
    }

    tag_to_fish_by_age_.resize(age_spread_, 0.0);
    vulnerable_fish_by_age_.resize(age_spread_, 0.0);
    final_exploitation_by_age_.resize(age_spread_, 0.0);
  } else {
    for (unsigned i = 0; i < selectivity_labels_.size(); ++i) {
      Selectivity* selectivity = selectivity_manager.GetSelectivity(selectivity_labels_[i]);
      if (!selectivity)
        LOG_ERROR() << "Selectivity label " << selectivity_labels_[i] << " was not found";
      selectivities_[from_category_labels_[i]] = selectivity;
    }
    if (initial_mortality_selectivity_label_ != "")
      initial_mortality_selectivity_ = selectivity_manager.GetSelectivity(initial_mortality_selectivity_label_);
  }
}

/**
 * Execute this process
 */
void Tagging::DoExecute() {
  LOG_FINE() << label_;
  unsigned current_year = model()->current_year();

  if (model()->state() == State::kInitialise)
    return;
  if ((std::find(years_.begin(), years_.end(), current_year) == years_.end()))
    return;

  if (process_profile_ == ProcessProfile::kAge) {
    // Age-specific execution
    auto     iter      = find(years_.begin(), years_.end(), current_year);
    unsigned year_ndx  = distance(years_.begin(), iter);
    auto     from_iter = from_partition_.begin();
    auto     to_iter   = to_partition_.begin();

    // Do the transition with mortality on the fish we're moving
    LOG_FINE() << "year_ndx = " << year_ndx << " year = " << current_year;
    LOG_FINEST() << "numbers_.size(): " << numbers_.size();
    LOG_FINEST() << "numbers_[current_year].size(): " << numbers_[current_year].size();
    LOG_FINE() << "number of ages: " << age_spread_ << " in year " << current_year;

    unsigned from_category_iter = 0;
    for (; from_iter != from_partition_.end(); from_iter++, from_category_iter++) {
      LOG_FINE() << "category counter = " << from_category_iter;
      fill(selected_numbers_at_age_by_category_[from_category_iter].begin(), selected_numbers_at_age_by_category_[from_category_iter].end(), 0.0);
      fill(exploitation_by_age_category_[from_category_iter].begin(), exploitation_by_age_category_[from_category_iter].end(), 0.0);
      fill(tag_to_fish_by_category_age_[from_category_iter].begin(), tag_to_fish_by_category_age_[from_category_iter].end(), 0.0);
    }

    from_iter          = from_partition_.begin();
    from_category_iter = 0;

    for (; from_iter != from_partition_.end(); from_iter++, from_category_iter++) {
      LOG_FINE() << "category counter = " << from_category_iter;
      for (unsigned age_ndx = 0; age_ndx < age_spread_; ++age_ndx) {
        tag_to_fish_by_category_age_[from_category_iter][age_ndx]
            = numbers_[years_[year_ndx]][age_ndx] * selectivities_[(*from_iter)->name_]->GetAgeResult(min_age_ + age_ndx, (*from_iter)->age_length_);
      }
    }

    from_iter          = from_partition_.begin();
    from_category_iter = 0;

    for (; from_iter != from_partition_.end(); from_iter++, from_category_iter++, to_iter++) {
      Double amount = 0.0;
      LOG_FINE() << "category = " << (*from_iter)->name_;
      for (unsigned age_ndx = 0; age_ndx < age_spread_; ++age_ndx) {
        LOG_FINE() << "age = " << age_ndx;
        exploitation_by_age_category_[from_category_iter][age_ndx] = tag_to_fish_by_category_age_[from_category_iter][age_ndx] / (*from_iter)->data_[min_age_offset_ + age_ndx];
        if (exploitation_by_age_category_[from_category_iter][age_ndx] > u_max_) {
          exploitation_by_age_category_[from_category_iter][age_ndx] = u_max_;
          // flag penalty
          if (penalty_) {
            LOG_FINEST() << " exploitation expected = " << exploitation_by_age_category_[from_category_iter][age_ndx]
                         << " available = " << (*from_iter)->data_[min_age_offset_ + age_ndx];
            penalty_->Trigger((*from_iter)->data_[min_age_offset_ + age_ndx],
                              (*from_iter)->data_[min_age_offset_ + age_ndx] * exploitation_by_age_category_[from_category_iter][age_ndx]);
          }
        }
        // fish to move
        amount = (*from_iter)->data_[min_age_offset_ + age_ndx] * exploitation_by_age_category_[from_category_iter][age_ndx];
        LOG_FINEST() << " exploitation = " << exploitation_by_age_category_[from_category_iter][age_ndx] << " amount - " << amount;
        actual_tagged_fish_to_[year_ndx][from_category_iter][age_ndx] += amount;
        // account for mortality
        if ((initial_mortality_selectivity_label_ != "") && (initial_mortality_ > 0.0))
          amount *= (1.0 - (initial_mortality_ * initial_mortality_selectivity_->GetAgeResult((*from_iter)->min_age_ + age_ndx, (*to_iter)->age_length_)));
        else if ((initial_mortality_selectivity_label_ == "") && (initial_mortality_ > 0.0))
          amount *= (1.0 - initial_mortality_);

        tagged_fish_after_init_mort_[year_ndx][from_category_iter][age_ndx] += amount;
        // Just do it!
        (*from_iter)->data_[min_age_offset_ + age_ndx] -= amount;
        (*to_iter)->data_[min_age_offset_ + age_ndx] += amount;

        if ((*from_iter)->data_[min_age_offset_ + age_ndx] < 0.0)
          LOG_CODE_ERROR() << "The process tag_by_age (with label " << label_ << ") caused a negative partition " << (*from_iter)->name_ << " "
                           << " age = " << age_ndx + (*from_iter)->min_age_ << " numbers at age = " << (*from_iter)->data_[min_age_offset_ + age_ndx]
                           << " tagged fish = " << amount;
      }
    }
  } else {
    // Length-specific execution
    if (model()->current_year() < first_year_)
      return;

    auto from_iter = from_partition_.begin();
    auto to_iter   = to_partition_.begin();

    LOG_FINEST() << "numbers_.size(): " << numbers_.size();
    LOG_FINEST() << "numbers_[current_year].size(): " << numbers_[current_year].size();

    for (unsigned i = 0; i < numbers_[current_year].size(); ++i) LOG_FINEST() << "numbers_[current_year][" << i << "]: " << numbers_[current_year][i];

    Double total_stock_with_selectivities = 0.0;
    LOG_FINE() << "length_spread: " << model()->get_number_of_length_bins() << " in year " << current_year;

    for (unsigned i = 0; i < model()->get_number_of_length_bins(); ++i) {
      // Calculate the Exploitation rate
      from_iter = from_partition_.begin();
      to_iter   = to_partition_.begin();
      LOG_FINEST() << "selectivity.size(): " << selectivities_.size();
      for (auto iter : selectivities_) LOG_FINE() << "selectivity: " << iter.first;

      total_stock_with_selectivities = 0.0;
      for (; from_iter != from_partition_.end(); from_iter++, to_iter++) {
        Double stock_amount = (*from_iter)->data_[i] * selectivities_[(*from_iter)->name_]->GetLengthResult(i);
        total_stock_with_selectivities += stock_amount;

        LOG_FINEST() << "name: " << (*from_iter)->name_ << " at length index " << i;
        LOG_FINEST() << "selectivity value: " << selectivities_[(*from_iter)->name_]->GetLengthResult(i);
        LOG_FINEST() << "population: " << (*from_iter)->data_[i];
        LOG_FINEST() << "amount added to total_stock_with_selectivities: " << stock_amount;
        LOG_FINEST() << "**";
      }
      LOG_FINEST() << "total_stock_with_selectivities: " << total_stock_with_selectivities << " at length ndx " << i;

      // Migrate the exploited amount
      from_iter = from_partition_.begin();
      to_iter   = to_partition_.begin();
      for (; from_iter != from_partition_.end(); from_iter++, to_iter++) {
        LOG_FINE() << "--";
        LOG_FINE() << "Working with categories: from " << (*from_iter)->name_ << "; to " << (*to_iter)->name_;
        string category_label = (*from_iter)->name_;

        if (numbers_[current_year][i] == 0)
          continue;

        Double current = numbers_[current_year][i] * ((*from_iter)->data_[i] * selectivities_[category_label]->GetLengthResult(i) / total_stock_with_selectivities);

        Double exploitation = current / utilities::math::ZeroFun((*from_iter)->data_[i] * selectivities_[category_label]->GetLengthResult(i));
        if (exploitation > u_max_) {
          LOG_FINE() << "Exploitation(" << exploitation << ") triggered u_max(" << u_max_ << ") with current(" << current << ")";

          current = (*from_iter)->data_[i] * selectivities_[category_label]->GetLengthResult(i) * u_max_;
          LOG_FINE() << "tagging amount overridden with " << current << " = " << (*from_iter)->data_[i] << " * " << selectivities_[category_label]->GetLengthResult(i) << " * "
                     << u_max_;

          if (penalty_)
            penalty_->Trigger(numbers_[current_year][i], current);
        }

        LOG_FINE() << "total_stock_with_selectivities: " << total_stock_with_selectivities;
        LOG_FINE() << "numbers/n: " << numbers_[current_year][i];
        LOG_FINE() << (*from_iter)->name_ << " population at length ndx " << i << ": " << (*from_iter)->data_[i];
        LOG_FINE() << "selectivity: " << selectivities_[category_label]->GetLengthResult(i);
        if (exploitation > u_max_) {
          LOG_FINE() << "exploitation: " << u_max_ << " (u_max)";
          LOG_FINE() << "tagging amount: " << current << " = " << (*from_iter)->data_[i] << " * " << selectivities_[category_label]->GetLengthResult(i) << " * " << u_max_;
        } else {
          LOG_FINE() << "exploitation: " << exploitation << "; calculated as " << current << " / (" << (*from_iter)->data_[i] << " * "
                     << selectivities_[category_label]->GetLengthResult(i) << ")";
          LOG_FINE() << "tagging amount: " << current << " = " << numbers_[current_year][i] << " * " << (*from_iter)->data_[i] << " * "
                     << selectivities_[category_label]->GetLengthResult(i) << " / " << total_stock_with_selectivities;
        }

        if (current <= 0.0)
          continue;

        Double current_with_mortality = current - (current * initial_mortality_);
        LOG_FINEST() << "tagging_amount_with_mortality: " << current_with_mortality << "; initial mortality: " << initial_mortality_;
        if (initial_mortality_selectivity_) {
          current_with_mortality *= initial_mortality_selectivity_->GetLengthResult(i);
          LOG_FINEST() << "tagging_amount_with_mortality: " << current_with_mortality << "; initial_mortality_selectivity : " << initial_mortality_selectivity_->GetLengthResult(i);
        }
        LOG_FINEST() << "Removing " << current << " from " << (*from_iter)->name_;
        LOG_FINEST() << "Adding " << current_with_mortality << " to " << (*to_iter)->name_;
        (*from_iter)->data_[i] -= current;
        (*to_iter)->data_[i] += current_with_mortality;
      }
    }

    for (unsigned year : years_) {
      if (numbers_.find(year) == numbers_.end())
        LOG_ERROR_P(PARAM_YEARS) << "value (" << year << ") does not have a corresponding entry in the numbers or proportions table";
    }
  }
}

/**
 * Fill the report cache
 */
void Tagging::FillReportCache(ostringstream& cache) {
  if (process_profile_ == ProcessProfile::kAge) {
    LOG_FINE() << "report age distribution of tagged individuals by source and destination";
    for (unsigned category_ndx = 0; category_ndx < to_category_labels_.size(); ++category_ndx) {
      cache << "tags-after-initial_mortality-" << to_category_labels_[category_ndx] << " " << REPORT_R_DATAFRAME_ROW_LABELS << "\n";
      cache << "year ";
      for (unsigned age = min_age_; age <= max_age_; ++age) cache << age << " ";
      cache << REPORT_EOL;
      for (unsigned year_ndx = 0; year_ndx < years_.size(); ++year_ndx) {
        cache << years_[year_ndx] << " ";
        for (unsigned age_ndx = 0; age_ndx < age_spread_; ++age_ndx) cache << AS_DOUBLE(tagged_fish_after_init_mort_[year_ndx][category_ndx][age_ndx]) << " ";
        cache << REPORT_EOL;
      }
    }

    for (unsigned category_ndx = 0; category_ndx < to_category_labels_.size(); ++category_ndx) {
      cache << "tag-releases-" << to_category_labels_[category_ndx] << " " << REPORT_R_DATAFRAME_ROW_LABELS << "\n";
      cache << "year ";
      for (unsigned age = min_age_; age <= max_age_; ++age) cache << age << " ";
      cache << REPORT_EOL;
      for (unsigned year_ndx = 0; year_ndx < years_.size(); ++year_ndx) {
        cache << years_[year_ndx] << " ";
        for (unsigned age_ndx = 0; age_ndx < age_spread_; ++age_ndx) cache << AS_DOUBLE(actual_tagged_fish_to_[year_ndx][category_ndx][age_ndx]) << " ";
        cache << REPORT_EOL;
      }
    }
  }
  // Length models don't have a FillReportCache in the original
}

} /* namespace niwa::processes::common */
