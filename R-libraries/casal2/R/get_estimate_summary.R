#' @title get_estimate_summary
#' @description Extract the \code{estimate_summary} report from a Casal2 model
#'   output into a tidy data frame with one row per estimated parameter.
#'   Handles single-run (\code{-r}) and multi-run (\code{-r -i}) MPD output
#'   and named lists of multiple MPD objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD} or named \code{list} of \code{casal2MPD}
#'   objects produced by \code{extract.mpd()}.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a data frame with columns
#'   \describe{
#'     \item{\code{parameter}}{Full Casal2 parameter label.}
#'     \item{\code{value}}{MPD (maximum posterior density) estimate.}
#'     \item{\code{initial_value}}{Starting value used by the optimiser.}
#'     \item{\code{phase}}{Estimation phase.}
#'     \item{\code{prior}}{Prior distribution type (e.g. \code{"lognormal"}).}
#'     \item{\code{lower_bound}}{Lower bound of the estimate.}
#'     \item{\code{upper_bound}}{Upper bound of the estimate.}
#'     \item{\code{std_dev}}{Standard deviation derived from the diagonal of the
#'       covariance matrix (\code{NA} when not available, e.g. in \code{-r}
#'       mode without a Hessian).}
#'     \item{\code{mcmc_fixed}}{Logical; whether the parameter is fixed during
#'       MCMC.}
#'     \item{\code{hyperparameters}}{Comma-separated string of prior
#'       hyperparameter names and values, e.g.
#'       \code{"mu = 1, sigma = 0.2"}.}
#'     \item{\code{label}}{Report label.}
#'     \item{\code{par_set}}{Integer (1) for a single run; ordered factor of run
#'       labels for multi-run \code{-i} output.}
#'   }
#'   Returns \code{NULL} when no matching report is found.
#'   \code{casal2TAB} is not implemented (tabular mode is not supported by the
#'   Casal2 \code{estimate_summary} report type).
#' @rdname get_estimate_summary
#' @export get_estimate_summary
"get_estimate_summary" <- function(model, ...) {
  UseMethod("get_estimate_summary", model)
}

## Build a data frame from one estimate_summary report block.
.es_df_from_report <- function(rpt, par_set, label) {
  param_names <- names(rpt)[names(rpt) != "type"]
  rows <- vector("list", length(param_names))
  for (k in seq_along(param_names)) {
    p <- rpt[[param_names[k]]]
    hyper_names <- p$hyperparameters
    hyper_vals <- suppressWarnings(as.numeric(p$hyperparameter_values))
    hyper_str <- if (length(hyper_names) > 0L) {
      paste(paste0(hyper_names, " = ", hyper_vals), collapse = ", ")
    } else {
      NA_character_
    }
    rows[[k]] <- data.frame(
      parameter        = p$parameter,
      value            = suppressWarnings(as.numeric(p$value)),
      initial_value    = suppressWarnings(as.numeric(p$initial_value)),
      phase            = suppressWarnings(as.integer(p$phase)),
      prior            = if (!is.null(p$sub_type)) p$sub_type else NA_character_,
      lower_bound      = suppressWarnings(as.numeric(p$lower_bound)),
      upper_bound      = suppressWarnings(as.numeric(p$upper_bound)),
      std_dev          = suppressWarnings(as.numeric(p$std_dev)),
      mcmc_fixed       = if (length(p$mcmc_fixed) > 0L) as.logical(p$mcmc_fixed) else FALSE,
      hyperparameters  = hyper_str,
      label            = label,
      par_set          = par_set,
      stringsAsFactors = FALSE
    )
  }
  .bind_rows_list(rows)
}

#' @rdname get_estimate_summary
#' @method get_estimate_summary casal2MPD
#' @export
"get_estimate_summary.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)

  orig_names <- names(model)
  is_default <- startsWith(orig_names, "__") & endsWith(orig_names, "__")
  stripped <- ifelse(is_default, substring(orig_names, 3L, nchar(orig_names) - 2L), orig_names)
  skip_default <- is_default & (stripped %in% stripped[!is_default])

  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    if (skip_default[i]) next
    this_report <- model[[i]]
    if ("type" %in% names(this_report)) {
      ## single run
      if (this_report$type != "estimate_summary") next
      rows[[i]] <- .es_df_from_report(this_report, par_set = 1L, label = report_labels[i])
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "estimate_summary") next
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        inner[[dash_i]] <- .es_df_from_report(this_report[[dash_i]],
          par_set = iter_labs[dash_i],
          label   = report_labels[i]
        )
      }
      df <- .bind_rows_list(inner)
      df$par_set <- factor(df$par_set, ordered = TRUE)
      rows[[i]] <- df
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_estimate_summary
#' @method get_estimate_summary list
#' @export
"get_estimate_summary.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_estimate_summary.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_estimate_summary
#' @method get_estimate_summary casal2TAB
#' @export
"get_estimate_summary.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  stop("get_estimate_summary for casal2TAB has not been implemented: the Casal2 estimate_summary report does not support tabular mode")
}
