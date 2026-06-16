#' @title has_* helpers for get_* accessors
#' @description Boolean helpers that mirror the package's \\code{get_*}
#'   functions and return \\code{TRUE} when the corresponding \\code{get_*}
#'   call returns non-empty content.
#' @param model A \\code{casal2MPD}, \\code{casal2TAB}, or named \\code{list}
#'   of \\code{casal2MPD} objects.
#' @param ... Arguments passed through to the paired \\code{get_*} function.
#' @return Logical scalar.
#' @name has_accessors
NULL

.has_get_result <- function(x) {
  if (is.null(x)) {
    return(FALSE)
  }
  if (is.data.frame(x) || is.matrix(x)) {
    return(nrow(x) > 0L)
  }
  if (is.atomic(x) || is.list(x)) {
    return(length(x) > 0L)
  }
  TRUE
}

.call_get <- function(fun_name, ...) {
  get(fun_name, mode = "function")(...)
}

#' @rdname has_accessors
#' @export
has_BH_recruitment <- function(model, ...) .has_get_result(.call_get("get_BH_recruitment", model, ...))

#' @rdname has_accessors
#' @export
has_abundance_observations <- function(model, ...) .has_get_result(.call_get("get_abundance_observations", model, ...))

#' @rdname has_accessors
#' @export
has_block <- function(model, ...) .has_get_result(.call_get("get_block", model, ...))

#' @rdname has_accessors
#' @export
has_catchabilities <- function(model, ...) .has_get_result(.call_get("get_catchabilities", model, ...))

#' @rdname has_accessors
#' @export
has_category_transitions <- function(model, ...) .has_get_result(.call_get("get_category_transitions", model, ...))

#' @rdname has_accessors
#' @export
has_composition_mean_bin <- function(model, ...) .has_get_result(.call_get("get_composition_mean_bin", model, ...))

#' @rdname has_accessors
#' @export
has_composition_observations <- function(model, ...) .has_get_result(.call_get("get_composition_observations", model, ...))

#' @rdname has_accessors
#' @export
has_correlation <- function(model, ...) .has_get_result(.call_get("get_correlation", model, ...))

#' @rdname has_accessors
#' @export
has_covariance <- function(model, ...) .has_get_result(.call_get("get_covariance", model, ...))

#' @rdname has_accessors
#' @export
has_derived_quantities <- function(model, ...) .has_get_result(.call_get("get_derived_quantities", model, ...))

#' @rdname has_accessors
#' @export
has_dqs <- function(model, ...) .has_get_result(.call_get("get_dqs", model, ...))

#' @rdname has_accessors
#' @export
has_error_value_table <- function(model, ...) .has_get_result(.call_get("get_error_value_table", model, ...))

#' @rdname has_accessors
#' @export
has_estimate_summary <- function(model, ...) .has_get_result(.call_get("get_estimate_summary", model, ...))

#' @rdname has_accessors
#' @export
has_estimated_values <- function(model, ...) .has_get_result(.call_get("get_estimated_values", model, ...))

#' @rdname has_accessors
#' @export
has_fisheries <- function(model, ...) .has_get_result(.call_get("get_fisheries", model, ...))

#' @rdname has_accessors
#' @export
has_growth <- function(model, ...) .has_get_result(.call_get("get_growth", model, ...))

#' @rdname has_accessors
#' @export
has_high_correlations <- function(model, ...) .has_get_result(.call_get("get_high_correlations", model, ...))

#' @rdname has_accessors
#' @export
has_initial_partition <- function(model, ...) .has_get_result(.call_get("get_initial_partition", model, ...))

#' @rdname has_accessors
#' @export
has_label <- function(model, ...) .has_get_result(.call_get("get_label", model, ...))

#' @rdname has_accessors
#' @export
has_objective_function <- function(model, ...) .has_get_result(.call_get("get_objective_function", model, ...))

#' @rdname has_accessors
#' @export
has_partition <- function(model, ...) .has_get_result(.call_get("get_partition", model, ...))

#' @rdname has_accessors
#' @export
has_partition_biomass <- function(model, ...) .has_get_result(.call_get("get_partition_biomass", model, ...))

#' @rdname has_accessors
#' @export
has_priors <- function(model, ...) .has_get_result(.call_get("get_priors", model, ...))

#' @rdname has_accessors
#' @export
has_profile <- function(model, ...) .has_get_result(.call_get("get_profile", model, ...))

#' @rdname has_accessors
#' @export
has_projections <- function(model, ...) .has_get_result(.call_get("get_projections", model, ...))

#' @rdname has_accessors
#' @export
has_selectivities <- function(model, ...) .has_get_result(.call_get("get_selectivities", model, ...))

#' @rdname has_accessors
#' @export
has_selectivities_by_year <- function(model, ...) .has_get_result(.call_get("get_selectivities_by_year", model, ...))

#' @rdname has_accessors
#' @export
has_timevarying_parameters <- function(model, ...) .has_get_result(.call_get("get_timevarying_parameters", model, ...))
