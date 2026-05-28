#' @title get_projections
#' @description Extract projection report data from a Casal2 tabular model
#'   output into a tidy data frame.
#' @author Casal2 Development Team
#' @param model A \code{casal2TAB}, \code{casal2MPD}, or named \code{list}
#'   object produced by \code{extract.tabular()} or \code{extract.mpd()}.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2TAB} input: a data frame with columns
#'   \code{iteration}, \code{colname}, \code{value}, \code{year},
#'   \code{param}, \code{label}, and (when present in tabular input)
#'   \code{chain}; or \code{NULL}.
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
    proj_df <- this_report$values
    nms <- colnames(proj_df)
    iter_col <- nms[tolower(nms) == "iteration"]
    chain_col <- nms[tolower(nms) == "chain"]
    meta_cols <- c(iter_col, chain_col)
    value_cols <- setdiff(nms, meta_cols)
    if (length(value_cols) == 0L) next

    iter_base <- if (length(iter_col) == 1L) as.numeric(proj_df[[iter_col]]) else seq_len(nrow(proj_df))
    chain_base <- if (length(chain_col) == 1L) proj_df[[chain_col]] else NULL

    ## Projection value columns are encoded as "param.year".
    ## Guard against non-conforming columns by keeping only those with a dot.
    dot_mask <- grepl("\\.", value_cols)
    if (!any(dot_mask)) next
    value_cols <- value_cols[dot_mask]

    n_rows <- nrow(proj_df)
    n_col <- length(value_cols)
    ## Wide-to-long without reshape2: stack all columns
    proj_long <- data.frame(
      iteration        = rep(iter_base, times = n_col),
      colname          = rep(value_cols, each = n_rows),
      value            = unlist(proj_df[, value_cols, drop = FALSE], use.names = FALSE),
      stringsAsFactors = FALSE
    )
    if (!is.null(chain_base)) {
      proj_long$chain <- rep(chain_base, times = n_col)
    }
    proj_long$label <- report_labels[i]
    ## Column names are encoded as "param.year".
    ## do.call(rbind, strsplit(...)) builds a matrix in one pass -- no repeated
    ## vapply iterations over the same strsplit result.
    split_mat <- do.call(rbind, strsplit(proj_long$colname, ".", fixed = TRUE))
    proj_long$param <- split_mat[, 1L]
    proj_long$year <- as.numeric(split_mat[, 2L])
    rows[[i]] <- proj_long
  }
  .bind_rows_list(rows)
}
