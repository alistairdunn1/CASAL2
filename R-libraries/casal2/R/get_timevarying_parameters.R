#' @title get_timevarying_parameters
#' @description Extract time-varying parameter report data from a Casal2 model
#'   output into a tidy data frame.  Handles single-run and multi-run
#'   (\code{-r -i}) MPD output and named lists of multiple MPD objects.
#' @author Casal2 Development Team
#'   Bug fix: original source used incorrect variable \code{reports_label}
#'   (should be \code{report_labels}).
#' @param model A \code{casal2MPD} or named \code{list} of \code{casal2MPD}
#'   objects.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return A data frame of time-varying parameter values, or \code{NULL}.
#' @rdname get_timevarying_parameters
#' @export get_timevarying_parameters
"get_timevarying_parameters" <- function(model, ...) {
  UseMethod("get_timevarying_parameters", model)
}

#' @rdname get_timevarying_parameters
#' @method get_timevarying_parameters casal2MPD
#' @export
"get_timevarying_parameters.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next       ## bug-fix: was `reports_label`
    this_report <- model[[i]]
    if ("type" %in% names(this_report)) {
      ## single run
      if (this_report$type != "time_varying") next
      df         <- this_report$values
      df$label   <- report_labels[i]
      df$par_set <- 1L
      rows[[i]]  <- df
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "time_varying") next
      n_runs    <- length(this_report)
      iter_labs <- names(this_report)
      inner     <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        df         <- this_report[[dash_i]]$values
        df$label   <- report_labels[i]
        df$par_set <- iter_labs[dash_i]
        inner[[dash_i]] <- df
      }
      rows[[i]] <- .bind_rows_list(inner)
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_timevarying_parameters
#' @method get_timevarying_parameters list
#' @export
"get_timevarying_parameters.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_timevarying_parameters.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_timevarying_parameters
#' @method get_timevarying_parameters casal2TAB
#' @export
"get_timevarying_parameters.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  stop("get_timevarying_parameters for casal2TAB has not been implemented")
}
