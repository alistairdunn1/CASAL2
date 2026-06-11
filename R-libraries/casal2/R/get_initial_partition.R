#' @title get_initial_partition
#' @description Extract initialisation-partition report data from a Casal2
#'   model output into a tidy data frame.  Handles single-run and multi-run
#'   (\code{-r -i}) MPD output and named lists of multiple MPD objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a data frame with columns \code{category},
#'   \code{bin}, \code{value}, \code{par_set} (integer index of the parameter
#'   set/minimisation; 1 for single run, ordered factor for multi-run \code{-i}
#'   output), and \code{label}.
#'   \code{casal2TAB} is not yet implemented.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_initial_partition
#' @export get_initial_partition
"get_initial_partition" <- function(model, ...) {
  UseMethod("get_initial_partition", model)
}

#' @rdname get_initial_partition
#' @method get_initial_partition casal2MPD
#' @export
"get_initial_partition.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
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
      if (this_report$type != "initialisation_partition") next
      df <- .melt_matrix(this_report$values)
      colnames(df) <- c("category", "bin", "value")
      df$par_set <- 1L
      df$label <- report_labels[i]
      rows[[i]] <- df
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "initialisation_partition") next
      n_runs <- length(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        df <- .melt_matrix(this_report[[dash_i]]$values)
        colnames(df) <- c("category", "bin", "value")
        df$par_set <- dash_i
        df$label <- report_labels[i]
        inner[[dash_i]] <- df
      }
      rows[[i]] <- .bind_rows_list(inner)
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_initial_partition
#' @method get_initial_partition list
#' @export
"get_initial_partition.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_initial_partition.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_initial_partition
#' @method get_initial_partition casal2TAB
#' @export
"get_initial_partition.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if (is.null(this_report$type) || this_report$type != "initialisation_partition") next

    val_df <- this_report$values
    if (is.null(val_df) || nrow(val_df) == 0L) next
    nms <- colnames(val_df)
    iter_col <- nms[tolower(nms) == "iteration"]
    chain_col <- nms[tolower(nms) == "chain"]
    meta_cols <- c(iter_col, chain_col)

    if (!("initialisation_phase" %in% nms) || !("category" %in% nms)) next
    bin_cols <- setdiff(nms, c("initialisation_phase", "category", meta_cols))
    if (length(bin_cols) == 0L) next

    n_rows <- nrow(val_df)
    n_col <- length(bin_cols)
    iter_base <- if (length(iter_col) == 1L) as.numeric(val_df[[iter_col]]) else seq_len(n_rows)
    chain_base <- if (length(chain_col) == 1L) val_df[[chain_col]] else NULL

    long_df <- data.frame(
      iteration = rep(iter_base, times = n_col),
      initialisation_phase = rep(val_df$initialisation_phase, times = n_col),
      category = rep(val_df$category, times = n_col),
      bin = rep(bin_cols, each = n_rows),
      value = as.numeric(unlist(val_df[, bin_cols, drop = FALSE], use.names = FALSE)),
      label = report_labels[i],
      stringsAsFactors = FALSE
    )
    if (!is.null(chain_base)) {
      long_df$chain <- rep(chain_base, times = n_col)
    }
    rows[[i]] <- long_df
  }
  .bind_rows_list(rows)
}
