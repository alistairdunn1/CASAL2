#' @title get_selectivities
#' @description Extract selectivity report data from a Casal2 model output into
#'   a tidy data frame.  Handles single-run and multi-run (\code{-r -i}) MPD
#'   output and named lists of multiple MPD objects.  When different selectivity
#'   reports carry different column sets only the columns common to all are
#'   retained.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a data frame with columns
#'   \code{selectivity}, \code{bin}, \code{selectivity_label}, and \code{par_set}
#'   (integer index of the parameter set/minimisation; 1 for single run, ordered
#'   factor for multi-run \code{-i} output).
#'   For \code{casal2TAB} (MCMC) input: a data frame with columns
#'   \code{iteration} (integer MCMC posterior-sample index), \code{selectivity},
#'   \code{bin}, \code{selectivity_label}, and \code{label}.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_selectivities
#' @export get_selectivities
"get_selectivities" <- function(model, ...) {
  UseMethod("get_selectivities", model)
}

#' @rdname get_selectivities
#' @method get_selectivities casal2MPD
#' @export
"get_selectivities.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
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
      if (this_report$type != "selectivity") next
      df <- data.frame(
        selectivity       = as.numeric(this_report$Values),
        bin               = names(this_report$Values),
        selectivity_label = report_labels[i],
        par_set           = 1L,
        stringsAsFactors  = FALSE
      )
      rows[[i]] <- df
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "selectivity") next
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        inner[[dash_i]] <- data.frame(
          selectivity       = as.numeric(this_report[[dash_i]]$Values),
          bin               = names(this_report[[dash_i]]$Values),
          selectivity_label = report_labels[i],
          par_set           = iter_labs[dash_i],
          stringsAsFactors  = FALSE
        )
      }
      rows[[i]] <- .safe_rbind(inner)
    }
  }
  result <- .safe_rbind(rows)
  if (!is.null(result)) {
    result$bin <- as.numeric(result$bin)
  }
  result
}

#' @rdname get_selectivities
#' @method get_selectivities list
#' @export
"get_selectivities.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_selectivities.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_selectivities
#' @method get_selectivities casal2TAB
#' @export
"get_selectivities.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  complete_df <- NULL

  for (i in seq_along(model)) {
    this_report <- model[[i]]
    if (is.null(this_report$type) || this_report$type != "selectivity") next

    sel_df <- this_report$values
    nms <- colnames(sel_df)
    iter_col <- nms[tolower(nms) == "iteration"]
    chain_col <- nms[tolower(nms) == "chain"]
    meta_cols <- c(iter_col, chain_col)
    collabs <- setdiff(nms, meta_cols)
    if (length(collabs) == 0L) next
    n_iter <- nrow(sel_df)
    n_col <- length(collabs)
    iter_base <- if (length(iter_col) == 1L) as.numeric(sel_df[[iter_col]]) else seq_len(n_iter)
    chain_base <- if (length(chain_col) == 1L) sel_df[[chain_col]] else NULL

    ## Column names have the form:  report_label[selectivity_label].bin
    ## Split on "." to separate the bin value from the rest.
    parts_dot <- strsplit(collabs, ".", fixed = TRUE)
    bin_labs <- vapply(parts_dot, function(x) x[2L], character(1L))

    ## The first part (before ".") is report_label[selectivity_label];
    ## split on "[" and strip the trailing "]" to get selectivity_label.
    sel_part <- vapply(parts_dot, function(x) x[1L], character(1L))
    parts_brk <- strsplit(sel_part, "[", fixed = TRUE)
    sel_labs <- vapply(parts_brk, function(x) {
      s <- x[2L]
      substring(s, 1L, nchar(s) - 1L)
    }, character(1L))

    ## Reshape wide -> long in base R
    iterations <- rep(iter_base, times = n_col)
    sel_values <- as.numeric(unlist(sel_df[, collabs, drop = FALSE], use.names = FALSE))
    sel_lab_long <- rep(sel_labs, each = n_iter)
    bin_long <- as.numeric(rep(bin_labs, each = n_iter))

    long_df <- data.frame(
      iteration         = iterations,
      selectivity       = sel_values,
      bin               = bin_long,
      selectivity_label = sel_lab_long,
      label             = report_labels[i],
      stringsAsFactors  = FALSE
    )
    if (!is.null(chain_base)) {
      long_df$chain <- rep(chain_base, times = n_col)
    }

    complete_df <- rbind(complete_df, long_df)
  }
  complete_df
}
