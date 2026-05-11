#' @title get_category_transitions
#' @description Extract category-transition process data from a Casal2 model
#'   output into a tidy data frame.  Handles single-run and multi-run
#'   (\code{-r -i}) MPD output and named lists of multiple MPD objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD} or named \code{list} of
#'   \code{casal2MPD} objects.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a data frame with columns \code{par_set}
#'   (integer index of the parameter set/minimisation; 1 for single run, ordered
#'   factor for multi-run \code{-i} output), \code{label}, \code{from},
#'   \code{to}, \code{proportions}, and \code{selectivity}.
#'   \code{casal2TAB} is not yet implemented.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_category_transitions
#' @export get_category_transitions
"get_category_transitions" <- function(model, ...) {
  UseMethod("get_category_transitions", model)
}

#' @rdname get_category_transitions
#' @method get_category_transitions casal2MPD
#' @export
"get_category_transitions.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
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
      if (tolower(this_report$type) != "process") next
      if (tolower(this_report$sub_type) != "transition_category") next
      rows[[i]] <- data.frame(
        par_set          = 1L,
        label            = report_labels[i],
        from             = this_report$from,
        to               = this_report$to,
        proportions      = this_report$proportions,
        selectivity      = this_report$selectivities,
        stringsAsFactors = FALSE
      )
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "process") next
      if (this_report[[1L]]$sub_type != "transition_category") next
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        rpt <- this_report[[dash_i]]
        inner[[dash_i]] <- data.frame(
          par_set          = iter_labs[dash_i],
          label            = report_labels[i],
          from             = rpt$from,
          to               = rpt$to,
          proportions      = rpt$proportions,
          selectivity      = rpt$selectivities,
          stringsAsFactors = FALSE
        )
      }
      rows[[i]] <- .bind_rows_list(inner)
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_category_transitions
#' @method get_category_transitions list
#' @export
"get_category_transitions.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_category_transitions.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_category_transitions
#' @method get_category_transitions casal2TAB
#' @export
"get_category_transitions.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  stop("get_category_transitions for casal2TAB has not been implemented")
}
