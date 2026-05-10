#' @title get_partition
#' @description Extract partition report data from a Casal2 model output into a
#'   tidy data frame.  Handles single-run and multi-run (\code{-r -i}) MPD
#'   output and named lists of multiple MPD objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a data frame with columns \code{category},
#'   \code{bin}, \code{value}, \code{year}, \code{time_step}, \code{par_set}
#'   (integer index of the parameter set/minimisation; 1 for single run, ordered
#'   factor for multi-run \code{-i} output), and \code{label}.
#'   \code{casal2TAB} is not yet implemented.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_partition
#' @export get_partition
"get_partition" <- function(model, ...) {
  UseMethod("get_partition", model)
}

## Build a long-form data frame from all year-steps within one partition report.
.partition_years_to_df <- function(rpt, par_set, label) {
  years <- names(rpt)[names(rpt) != "type"]
  rows <- vector("list", length(years))
  for (k in seq_along(years)) {
    y_rpt <- rpt[[years[k]]]
    df <- .melt_matrix(y_rpt$values)
    colnames(df) <- c("category", "bin", "value")
    df$year <- years[k]
    df$time_step <- y_rpt$time_step
    df$par_set <- par_set
    df$label <- label
    rows[[k]] <- df
  }
  .bind_rows_list(rows)
}

#' @rdname get_partition
#' @method get_partition casal2MPD
#' @export
"get_partition.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
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
      if (this_report$type != "partition") next
      rows[[i]] <- .partition_years_to_df(this_report, par_set = 1L, label = report_labels[i])
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "partition") next
      n_runs <- length(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        inner[[dash_i]] <- .partition_years_to_df(this_report[[dash_i]], par_set = dash_i, label = report_labels[i])
      }
      rows[[i]] <- .bind_rows_list(inner)
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_partition
#' @method get_partition list
#' @export
"get_partition.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_partition.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_partition
#' @method get_partition casal2TAB
#' @export
"get_partition.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  stop("get_partition for casal2TAB has not been implemented")
}
