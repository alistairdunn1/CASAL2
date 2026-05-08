#' @title get_estimated_values
#' @description Extract estimated parameter values from a Casal2 model output
#'   into a tidy data frame.  Handles single-run and multi-run (\code{-r -i})
#'   MPD output, tabular (\code{casal2TAB}) output, and named lists of multiple
#'   MPD objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a data frame with columns \code{par_set}
#'   (integer index of the parameter set/minimisation; 1 for single run, ordered
#'   factor for multi-run \code{-i} output), \code{parameter}, and \code{value}.
#'   For \code{casal2TAB} (MCMC) input: a data frame with columns
#'   \code{iteration} (integer MCMC posterior-sample index), \code{parameter},
#'   \code{value}, and \code{label}.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_estimated_values
#' @export get_estimated_values
"get_estimated_values" <- function(model, ...) {
  UseMethod("get_estimated_values", model)
}

#' @rdname get_estimated_values
#' @method get_estimated_values casal2MPD
#' @export
"get_estimated_values.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if ("type" %in% names(this_report)) {
      ## single run
      if (this_report$type != "estimate_value") next
      rows[[i]] <- data.frame(
        par_set = 1L,
        parameter = names(this_report$values),
        value = as.numeric(this_report$values),
        stringsAsFactors = FALSE
      )
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "estimate_value") next
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        inner[[dash_i]] <- data.frame(
          par_set = iter_labs[dash_i],
          parameter = names(this_report[[dash_i]]$values),
          value = as.numeric(this_report[[dash_i]]$values),
          stringsAsFactors = FALSE
        )
      }
      df <- .bind_rows_list(inner)
      df$par_set <- factor(df$par_set, ordered = TRUE)
      rows[[i]] <- df
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_estimated_values
#' @method get_estimated_values list
#' @export
"get_estimated_values.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_estimated_values.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_estimated_values
#' @method get_estimated_values casal2TAB
#' @export
"get_estimated_values.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if (is.null(this_report$type) || this_report$type != "estimate_value") next
    ## val_df: rows = iterations, columns = parameter names
    val_df <- this_report$values
    params <- colnames(val_df)
    n_iter <- nrow(val_df)
    ## Wide-to-long without reshape2
    df <- data.frame(
      iteration = rep(seq_len(n_iter), times = length(params)),
      parameter = rep(params, each = n_iter),
      value = unlist(val_df, use.names = FALSE),
      label = report_labels[i],
      stringsAsFactors = FALSE
    )
    rows[[i]] <- df
  }
  .bind_rows_list(rows)
}
