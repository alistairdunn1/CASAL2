#' @title get_projections
#' @description Extract projection report data from a Casal2 tabular model
#'   output into a tidy data frame.
#' @author Casal2 Development Team
#' @param model A \code{casal2TAB}, \code{casal2MPD}, or named \code{list}
#'   object produced by \code{extract.tabular()} or \code{extract.mpd()}.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2TAB} input: a data frame with columns \code{colname},
#'   \code{value}, \code{year}, and \code{param}; or \code{NULL}.
#'   \code{casal2MPD} and list methods are not implemented.
#' @rdname get_projections
#' @export get_projections
"get_projections" <- function(model, ...) {
  UseMethod("get_projections", model)
}

#' @rdname get_projections
#' @method get_projections casal2MPD
#' @export
"get_projections.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  message("get_projections is only implemented for casal2TAB output")
  invisible(NULL)
}

#' @rdname get_projections
#' @method get_projections list
#' @export
"get_projections.list" <- function(model, reformat_labels = TRUE, ...) {
  message("get_projections is only implemented for casal2TAB output")
  invisible(NULL)
}

#' @rdname get_projections
#' @method get_projections casal2TAB
#' @export
"get_projections.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if (is.null(this_report$type) || this_report$type != "project") next
    proj_df   <- this_report$values
    col_names <- colnames(proj_df)
    n_rows    <- nrow(proj_df)
    ## Wide-to-long without reshape2: stack all columns
    proj_long <- data.frame(
      colname          = rep(col_names, each = n_rows),
      value            = unlist(proj_df, use.names = FALSE),
      stringsAsFactors = FALSE
    )
    ## Column names are encoded as "param.year".
    ## do.call(rbind, strsplit(...)) builds a matrix in one pass -- no repeated
    ## vapply iterations over the same strsplit result.
    split_mat       <- do.call(rbind, strsplit(proj_long$colname, ".", fixed = TRUE))
    proj_long$param <- split_mat[, 1L]
    proj_long$year  <- as.numeric(split_mat[, 2L])
    rows[[i]] <- proj_long
  }
  .bind_rows_list(rows)
}
