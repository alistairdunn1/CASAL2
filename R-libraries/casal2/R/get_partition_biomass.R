#' @title get_partition_biomass
#' @description Extract partition-biomass report data from a Casal2 model
#'   output into a tidy data frame. Handles single-run and multi-run
#'   (\code{-r -i}) MPD output and tabular output from
#'   \code{extract.tabular()}.
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
#'   For \code{casal2TAB} input: a data frame with columns \code{iteration},
#'   \code{year}, \code{time_step}, \code{category}, \code{bin},
#'   \code{value}, \code{label}, and (when present in tabular input)
#'   \code{chain}.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_partition_biomass
#' @export get_partition_biomass
"get_partition_biomass" <- function(model, ...) {
  UseMethod("get_partition_biomass", model)
}

## Build a long-form data frame from all year-steps within one partition-biomass report.
.partition_biomass_years_to_df <- function(rpt, par_set, label) {
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

#' @rdname get_partition_biomass
#' @method get_partition_biomass casal2MPD
#' @export
"get_partition_biomass.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
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
      if (this_report$type != "partition_biomass") next
      rows[[i]] <- .partition_biomass_years_to_df(this_report, par_set = 1L, label = report_labels[i])
    } else {
      if (this_report[[1L]]$type != "partition_biomass") next
      n_runs <- length(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        inner[[dash_i]] <- .partition_biomass_years_to_df(this_report[[dash_i]], par_set = dash_i, label = report_labels[i])
      }
      rows[[i]] <- .bind_rows_list(inner)
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_partition_biomass
#' @method get_partition_biomass list
#' @export
"get_partition_biomass.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get("get_partition_biomass.casal2MPD", mode = "function"), reformat_labels = reformat_labels)
}

#' @rdname get_partition_biomass
#' @method get_partition_biomass casal2TAB
#' @export
"get_partition_biomass.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if (is.null(this_report$type) || this_report$type != "partition_biomass") next

    val_df <- this_report$values
    if (is.null(val_df) || nrow(val_df) == 0L) next
    nms <- colnames(val_df)
    iter_col <- nms[tolower(nms) == "iteration"]
    chain_col <- nms[tolower(nms) == "chain"]
    meta_cols <- c(iter_col, chain_col)

    if (!("year" %in% nms) || !("time_step" %in% nms) || !("category" %in% nms)) next
    bin_cols <- setdiff(nms, c("year", "time_step", "category", meta_cols))
    if (length(bin_cols) == 0L) next

    n_rows <- nrow(val_df)
    n_col <- length(bin_cols)
    iter_base <- if (length(iter_col) == 1L) as.numeric(val_df[[iter_col]]) else seq_len(n_rows)
    chain_base <- if (length(chain_col) == 1L) val_df[[chain_col]] else NULL

    long_df <- data.frame(
      iteration = rep(iter_base, times = n_col),
      year = rep(val_df$year, times = n_col),
      time_step = rep(val_df$time_step, times = n_col),
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
