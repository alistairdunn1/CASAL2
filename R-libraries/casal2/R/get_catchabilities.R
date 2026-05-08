#' @title get_catchabilities
#' @description Extract catchability values from a Casal2 model output into a
#'   tidy data frame.  Handles both single-run (\code{-r}) and multi-run
#'   (\code{-r -i}) MPD output, tabular (\code{casal2TAB}) output, and
#'   named lists of multiple MPD objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects produced by \code{extract.mpd()} or
#'   \code{extract.tabular()}.
#' @param reformat_labels Logical. Reformat default Casal2 report labels by
#'   removing leading/trailing underscores (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a data frame with columns \code{par_set}
#'   (integer index of the parameter set/minimisation; 1 for single run, ordered
#'   factor for multi-run \code{-i} output), \code{label}, and
#'   \code{catchability}.
#'   For \code{casal2TAB} (MCMC) input: a data frame with columns
#'   \code{iteration} (integer MCMC posterior-sample index), \code{parameter}
#'   (catchability parameter name as emitted by Casal2, e.g.
#'   \code{"q[survey1]"}), \code{catchability}, and \code{label}.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_catchabilities
#' @export get_catchabilities
#' @seealso \code{\link{has_catchabilities}}, \code{\link{reformat_default_labels}}
"get_catchabilities" <- function(model, ...) {
  UseMethod("get_catchabilities", model)
}

#' @title has_catchabilities
#' @description Check whether a Casal2 model output contains any catchability reports.
#' @param model A \code{casal2MPD} object.
#' @param \dots Further arguments (currently unused).
#' @return \code{TRUE} if at least one \code{catchability} report is present.
#' @rdname has_catchabilities
#' @export has_catchabilities
"has_catchabilities" <- function(model, ...) {
  for (i in seq_along(model)) {
    this_report <- model[[i]]
    if ("type" %in% names(this_report) && this_report$type == "catchability") {
      return(TRUE)
    }
  }
  FALSE
}

#' @rdname get_catchabilities
#' @method get_catchabilities casal2MPD
#' @export
"get_catchabilities.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if ("type" %in% names(this_report)) {
      ## single run
      if (this_report$type != "catchability") next
      rows[[i]] <- data.frame(
        par_set = 1L, label = report_labels[i],
        catchability = this_report$q,
        stringsAsFactors = FALSE
      )
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "catchability") next
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        inner[[dash_i]] <- data.frame(
          par_set = iter_labs[dash_i],
          label = report_labels[i],
          catchability = this_report[[dash_i]]$q,
          stringsAsFactors = FALSE
        )
      }
      df <- .bind_rows_list(inner)
      df$par_set <- factor(df$par_set, ordered = TRUE)
      rows[[i]] <- df
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_catchabilities
#' @method get_catchabilities list
#' @export
"get_catchabilities.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_catchabilities.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_catchabilities
#' @method get_catchabilities casal2TAB
#' @export
"get_catchabilities.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  complete_df <- NULL

  for (i in seq_along(model)) {
    this_report <- model[[i]]
    if (is.null(this_report$type) || this_report$type != "catchability") next

    val_df <- this_report$values
    collabs <- colnames(val_df)
    n_iter <- nrow(val_df)
    n_col <- ncol(val_df)

    ## Reshape wide -> long in base R
    long_df <- data.frame(
      iteration = rep(seq_len(n_iter), times = n_col),
      parameter = rep(collabs, each = n_iter),
      catchability = as.numeric(unlist(val_df, use.names = FALSE)),
      label = report_labels[i],
      stringsAsFactors = FALSE
    )
    complete_df <- rbind(complete_df, long_df)
  }
  complete_df
}
