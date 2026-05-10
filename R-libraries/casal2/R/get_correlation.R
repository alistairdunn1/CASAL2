#' @title get_correlation
#' @description Extract the correlation matrix from a Casal2 MPD model output.
#'   For multi-run (\code{-r -i}) output only the first run's correlation matrix
#'   is returned (correlation matrices from individual optimisation runs are not
#'   stacked).
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD} or \code{casal2TAB} object produced by
#'   \code{extract.mpd()} or \code{extract.tabular()}.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}); currently unused for correlation extraction but
#'   included for interface consistency.
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a matrix containing the correlation
#'   matrix, or \code{NULL} when no matching report is found.
#'   For \code{list} input: a named list of correlation matrices (one per model).
#'   \code{casal2TAB} is not implemented.
#' @rdname get_correlation
#' @export get_correlation
"get_correlation" <- function(model, ...) {
  UseMethod("get_correlation", model)
}

#' @rdname get_correlation
#' @method get_correlation casal2MPD
#' @export
"get_correlation.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if ("type" %in% names(this_report)) {
      if (this_report$type != "correlation_matrix") next
      return(this_report$correlation_matrix)
    } else {
      if (this_report[[1L]]$type != "correlation_matrix") next
      return(this_report[[1L]]$correlation_matrix)
    }
  }
  invisible(NULL)
}

#' @rdname get_correlation
#' @method get_correlation list
#' @export
"get_correlation.list" <- function(model, reformat_labels = TRUE, ...) {
  labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  result <- vector("list", length(model))
  names(result) <- labels
  for (i in seq_along(model)) {
    if (!inherits(model[[i]], "casal2MPD")) {
      stop("This function only works on a named list with elements of class = 'casal2MPD'")
    }
    result[[i]] <- get_correlation.casal2MPD(model[[i]], reformat_labels = reformat_labels)
  }
  result
}

#' @rdname get_correlation
#' @method get_correlation casal2TAB
#' @export
"get_correlation.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  stop("get_correlation for casal2TAB has not been implemented")
}
