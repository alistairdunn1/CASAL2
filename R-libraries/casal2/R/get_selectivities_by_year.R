#' @title get_selectivities_by_year
#' @description Extract selectivity-by-year report data from a Casal2 model
#'   output into a tidy data frame.  Handles single-run and multi-run
#'   (\code{-r -i}) MPD output.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD} or named \code{list} of \code{casal2MPD}
#'   objects.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return A data frame with columns \code{bin}, \code{par_set},
#'   \code{selectivity}, \code{type}, \code{selectivity_label}, and \code{year};
#'   or \code{NULL}.
#' @rdname get_selectivities_by_year
#' @export get_selectivities_by_year
"get_selectivities_by_year" <- function(model, ...) {
  UseMethod("get_selectivities_by_year", model)
}

## Convert one year-slice of a selectivity_by_year report to a data frame.
.sel_year_to_df <- function(y_rpt, year_label, par_set) {
  df <- .melt_matrix(y_rpt$Values)
  colnames(df) <- c("bin", "par_set", "selectivity")
  df$type <- y_rpt$sub_type
  df$selectivity_label <- y_rpt$selectivity
  df$year <- year_label
  df$par_set <- par_set # overwrite Var2 with caller-supplied value
  df
}

#' @rdname get_selectivities_by_year
#' @method get_selectivities_by_year casal2MPD
#' @export
"get_selectivities_by_year.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
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
      if (this_report$type != "selectivity_by_year") next
      years <- names(this_report)[names(this_report) != "type"]
      y_rows <- vector("list", length(years))
      for (k in seq_along(years)) {
        y_rows[[k]] <- .sel_year_to_df(this_report[[years[k]]], years[k], par_set = "1")
      }
      rows[[i]] <- .bind_rows_list(y_rows)
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "selectivity_by_year") next
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        this_par_df <- this_report[[dash_i]]
        years <- names(this_par_df)[names(this_par_df) != "type"]
        y_rows <- vector("list", length(years))
        for (k in seq_along(years)) {
          y_rows[[k]] <- .sel_year_to_df(this_par_df[[years[k]]], years[k],
            par_set = iter_labs[dash_i]
          )
        }
        inner[[dash_i]] <- .bind_rows_list(y_rows)
      }
      rows[[i]] <- .bind_rows_list(inner)
    }
  }
  result <- .bind_rows_list(rows)
  if (!is.null(result)) {
    result$bin <- as.numeric(result$bin)
  }
  result
}

#' @rdname get_selectivities_by_year
#' @method get_selectivities_by_year list
#' @export
"get_selectivities_by_year.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_selectivities_by_year.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_selectivities_by_year
#' @method get_selectivities_by_year casal2TAB
#' @export
"get_selectivities_by_year.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  stop("get_selectivities_by_year for casal2TAB has not been implemented")
}
