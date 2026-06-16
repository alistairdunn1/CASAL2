% Generated manually for has_* accessors
\name{has_accessors}
\alias{has_accessors}
\alias{has_BH_recruitment}
\alias{has_abundance_observations}
\alias{has_block}
\alias{has_catchabilities}
\alias{has_category_transitions}
\alias{has_composition_mean_bin}
\alias{has_composition_observations}
\alias{has_correlation}
\alias{has_covariance}
\alias{has_derived_quantities}
\alias{has_dqs}
\alias{has_error_value_table}
\alias{has_estimate_summary}
\alias{has_estimated_values}
\alias{has_fisheries}
\alias{has_growth}
\alias{has_high_correlations}
\alias{has_initial_partition}
\alias{has_label}
\alias{has_objective_function}
\alias{has_partition}
\alias{has_partition_biomass}
\alias{has_priors}
\alias{has_profile}
\alias{has_projections}
\alias{has_selectivities}
\alias{has_selectivities_by_year}
\alias{has_timevarying_parameters}
\title{has_* helpers for get_* accessors}
\usage{
has_BH_recruitment(model, ...)
has_abundance_observations(model, ...)
has_block(model, ...)
has_catchabilities(model, ...)
has_category_transitions(model, ...)
has_composition_mean_bin(model, ...)
has_composition_observations(model, ...)
has_correlation(model, ...)
has_covariance(model, ...)
has_derived_quantities(model, ...)
has_dqs(model, ...)
has_error_value_table(model, ...)
has_estimate_summary(model, ...)
has_estimated_values(model, ...)
has_fisheries(model, ...)
has_growth(model, ...)
has_high_correlations(model, ...)
has_initial_partition(model, ...)
has_label(model, ...)
has_objective_function(model, ...)
has_partition(model, ...)
has_partition_biomass(model, ...)
has_priors(model, ...)
has_profile(model, ...)
has_projections(model, ...)
has_selectivities(model, ...)
has_selectivities_by_year(model, ...)
has_timevarying_parameters(model, ...)
}
\arguments{
  \item{model}{A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
  \code{casal2MPD} objects.}

  \item{...}{Arguments passed through to the paired \code{get_*} function.}
}
\value{Logical scalar.}
\description{
Boolean helpers that mirror the package's \code{get_*} functions and return
\code{TRUE} when the corresponding \code{get_*} call returns non-empty content.
}
