#' @title get_derived_quantities
#' @description Extract derived quantity values from a Casal2 model output into
#'   a tidy data frame.  Handles single-run and multi-run (\code{-r -i}) MPD
#'   output, tabular (\code{casal2TAB}) output, and named lists of multiple MPD
#'   objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a data frame with columns \code{par_set}
#'   (integer index of the parameter set/minimisation; 1 for single run, ordered
#'   factor for multi-run \code{-i} output), \code{year}, \code{value},
#'   \code{initialisation_value}, \code{initialisation_value_2}, \code{dq_label},
#'   and \code{label}.
#'   For \code{casal2TAB} (MCMC) input: a data frame with columns
#'   \code{iteration} (integer MCMC posterior-sample index), \code{value},
#'   \code{label}, \code{dq_label}, \code{year}, and \code{type}.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_derived_quantities
#' @export get_derived_quantities
#' @aliases get_dqs
"get_derived_quantities" <- function(model, ...) {
  UseMethod("get_derived_quantities", model)
}

#' @rdname get_derived_quantities
#' @export get_dqs
"get_dqs" <- function(model, ...) {
  UseMethod("get_derived_quantities", model)
}

## Build a data frame of all DQ types within one (possibly multi-run) report block.
.dq_from_single_run <- function(dq_report, par_set, label) {
  dq_types <- names(dq_report)[names(dq_report) != "type"]
  rows <- vector("list", length(dq_types))
  for (k in seq_along(dq_types)) {
    dq <- dq_report[[dq_types[k]]]
    df <- data.frame(
      par_set              = par_set,
      year                 = as.numeric(names(dq$values)),
      value                = dq$values,
      initialisation_value = as.numeric(dq$`initialisation_phase[1]`),
      dq_label             = dq_types[k],
      label                = label,
      stringsAsFactors     = FALSE
    )
    df$initialisation_value_2 <- if (!is.null(dq$`initialisation_phase[2]`)) {
      as.numeric(dq$`initialisation_phase[2]`)
    } else {
      NA_real_
    }
    rows[[k]] <- df
  }
  .bind_rows_list(rows)
}

#' @rdname get_derived_quantities
#' @method get_derived_quantities casal2MPD
#' @export
"get_derived_quantities.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
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
      if (this_report$type != "derived_quantity") next
      rows[[i]] <- .dq_from_single_run(this_report, par_set = 1L, label = report_labels[i])
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "derived_quantity") next
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        inner[[dash_i]] <- .dq_from_single_run(this_report[[dash_i]],
          par_set = iter_labs[dash_i],
          label   = report_labels[i]
        )
      }
      df <- .bind_rows_list(inner)
      df$par_set <- factor(df$par_set, ordered = TRUE)
      rows[[i]] <- df
    }
  }
  result <- .bind_rows_list(rows)
  if (!is.null(result)) {
    result$initialisation_value <- as.numeric(result$initialisation_value)
    result$year <- as.numeric(result$year)
  }
  result
}

#' @rdname get_derived_quantities
#' @method get_derived_quantities list
#' @export
"get_derived_quantities.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_derived_quantities.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_derived_quantities
#' @method get_derived_quantities casal2TAB
#' @export
"get_derived_quantities.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if (is.null(this_report$type) || this_report$type != "derived_quantity") next
    ## Column names follow the pattern: label[dq_name[year]]
    ## Splitting on "[" gives: c("label", "dq_name]", "year]", "]").
    ## Build a matrix (one row per column name) then index directly -- one pass,
    ## no repeated vapply calls over the same strsplit result.
    col_labs <- colnames(this_report$values)
    parts_mat <- do.call(rbind, strsplit(col_labs, "[", fixed = TRUE))
    first <- parts_mat[, 1L]
    second <- substr(parts_mat[, 2L], 1L, nchar(parts_mat[, 2L]) - 1L)
    third <- substr(parts_mat[, 3L], 1L, nchar(parts_mat[, 3L]) - 1L)
    ## Rename columns to "dq_label-year-label" for convenient downstream splitting
    colnames(this_report$values) <- paste(second, third, first, sep = "-")
    ## Convert wide-to-long using base R (avoids reshape2 dependency)
    val_mat <- as.matrix(this_report$values)
    long_df <- .melt_matrix(val_mat) # Var1=row, Var2=col, value
    colnames(long_df) <- c("iteration", "colname", "value")
    long_df$label <- report_labels[i]
    split_cols <- strsplit(long_df$colname, "-", fixed = TRUE)
    long_df$dq_label <- vapply(split_cols, `[[`, character(1L), 1L)
    long_df$year <- as.numeric(vapply(split_cols, `[[`, character(1L), 2L))
    long_df$type <- vapply(split_cols, `[[`, character(1L), 3L)
    long_df$colname <- NULL
    rows[[i]] <- long_df
  }
  .bind_rows_list(rows)
}
