#' @title get_covariance
#' @description Extract the covariance matrix from a Casal2 MPD model output.
#'   For multi-run (\code{-r -i}) output only the first run's covariance matrix
#'   is returned (covariance matrices from individual optimisation runs are not
#'   stacked).
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD} or \code{casal2TAB} object produced by
#'   \code{extract.mpd()} or \code{extract.tabular()}.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}); currently unused for covariance extraction but
#'   included for interface consistency.
#' @param \dots Further arguments (currently unused).
#' @return For \\code{casal2MPD} input: a matrix containing the covariance
#'   matrix, or \\code{NULL} when no matching report is found.
#'   For \\code{list} input: a named list of covariance matrices (one per model).
#'   \\code{casal2TAB} is not implemented.
#' @rdname get_covariance
#' @export get_covariance
"get_covariance" <- function(model, ...) {
  UseMethod("get_covariance", model)
}

#' @rdname get_covariance
#' @method get_covariance casal2MPD
#' @export
"get_covariance.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if ("type" %in% names(this_report)) {
      if (this_report$type != "covariance_matrix") next
      return(this_report$covariance_matrix)
    } else {
      if (this_report[[1L]]$type != "covariance_matrix") next
      return(this_report[[1L]]$covariance_matrix)
    }
  }
  invisible(NULL)
}

#' @rdname get_covariance
#' @method get_covariance list
#' @export
"get_covariance.list" <- function(model, reformat_labels = TRUE, ...) {
  labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  result <- vector("list", length(model))
  names(result) <- labels
  for (i in seq_along(model)) {
    if (!inherits(model[[i]], "casal2MPD")) {
      stop("This function only works on a named list with elements of class = 'casal2MPD'")
    }
    result[[i]] <- get_covariance.casal2MPD(model[[i]], reformat_labels = reformat_labels)
  }
  result
}

#' @rdname get_covariance
#' @method get_covariance casal2TAB
#' @export
"get_covariance.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  stop("get_covariance for casal2TAB has not been implemented")
}
