#' @title get_timevarying_parameters
#' @description Extract time-varying parameter report data from a Casal2 model
#'   output into a tidy data frame.  Handles single-run and multi-run
#'   (\code{-r -i}) MPD output and named lists of multiple MPD objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD} or named \code{list} of \code{casal2MPD}
#'   objects.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return A data frame with columns from the Casal2 time-varying report (e.g.
#'   \code{year}, \code{parameter}, \code{value}) plus \code{label} (report
#'   label) and \code{par_set} (integer index of the parameter
#'   set/minimisation; 1 for single run, ordered factor for multi-run
#'   \code{-i} output).  Returns \code{NULL} when no matching reports are found.
#' @rdname get_timevarying_parameters
#' @export get_timevarying_parameters
"get_timevarying_parameters" <- function(model, ...) {
  UseMethod("get_timevarying_parameters", model)
}

## Standardise common output column names to match other get_* accessors.
.normalise_timevarying_cols <- function(df) {
  if (is.null(df) || !is.data.frame(df)) {
    return(df)
  }

  nms <- names(df)
  low <- tolower(nms)

  year_idx <- which(low == "year")
  if (length(year_idx) >= 1L) {
    nms[year_idx[1L]] <- "year"
  }

  value_idx <- which(low == "value")
  if (length(value_idx) >= 1L) {
    nms[value_idx[1L]] <- "value"
  } else if (length(year_idx) == 1L && ncol(df) == 2L) {
    ## Common time-varying layout is a two-column year/value table.
    other_idx <- setdiff(seq_along(nms), year_idx[1L])
    nms[other_idx] <- "value"
  }

  names(df) <- nms

  if ("year" %in% names(df)) {
    df$year <- suppressWarnings(as.numeric(as.character(df$year)))
  }

  df
}

#' @rdname get_timevarying_parameters
#' @method get_timevarying_parameters casal2MPD
#' @export
"get_timevarying_parameters.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)

  orig_names <- names(model)
  is_default <- startsWith(orig_names, "__") & endsWith(orig_names, "__")
  stripped <- ifelse(is_default, substring(orig_names, 3L, nchar(orig_names) - 2L), orig_names)
  skip_default <- is_default & (stripped %in% stripped[!is_default])

  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next ## bug-fix: was `reports_label`
    if (skip_default[i]) next
    this_report <- model[[i]]
    if ("type" %in% names(this_report)) {
      ## single run
      if (this_report$type != "time_varying") next
      df <- .normalise_timevarying_cols(this_report$values)
      df$label <- report_labels[i]
      df$par_set <- 1L
      rows[[i]] <- df
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "time_varying") next
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        df <- .normalise_timevarying_cols(this_report[[dash_i]]$values)
        df$label <- report_labels[i]
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
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)

  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if (is.null(this_report$type) || tolower(this_report$type) != "time_varying") next

    df <- .normalise_timevarying_cols(this_report$values)
    if (is.null(df) || !is.data.frame(df) || nrow(df) == 0L) next

    if (!"par_set" %in% names(df)) {
      low <- tolower(names(df))
      iter_idx <- which(low == "iteration")
      if (length(iter_idx) >= 1L) {
        df$par_set <- df[[iter_idx[1L]]]
      } else {
        df$par_set <- 1L
      }
    }

    df$label <- report_labels[i]
    rows[[i]] <- df
  }

  .bind_rows_list(rows)
}
