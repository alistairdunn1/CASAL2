#' @title get_growth
#' @description Extract age-length or growth-increment report data from a Casal2
#'   model output into a tidy data frame.  Handles single-run and multi-run
#'   (\code{-r -i}) MPD output and named lists of multiple MPD objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a data frame with columns \code{age},
#'   \code{year}, \code{time_step}, \code{cvs_by_age}, \code{mean_length_at_age},
#'   \code{mean_weight_at_age}, \code{label}, and \code{par_set} (integer index
#'   of the parameter set/minimisation; 1 for single run, ordered factor for
#'   multi-run \code{-i} output).
#'   \code{casal2TAB} is not yet implemented.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_growth
#' @export get_growth
"get_growth" <- function(model, ...) {
  UseMethod("get_growth", model)
}

## Convert an age_length report block to a data frame.
.age_length_to_df <- function(rpt, par_set, label) {
  years <- names(rpt)[names(rpt) != "type"]
  rows <- vector("list", length(years))
  for (k in seq_along(years)) {
    y_rpt <- rpt[[years[k]]]
    rows[[k]] <- data.frame(
      age                = y_rpt$age,
      year               = as.numeric(years[k]),
      time_step          = y_rpt$time_step,
      cvs_by_age         = y_rpt$cvs_by_age,
      mean_length_at_age = y_rpt$mean_length_at_age,
      mean_weight_at_age = y_rpt$mean_weight_at_age,
      label              = label,
      par_set            = par_set,
      stringsAsFactors   = FALSE
    )
  }
  .bind_rows_list(rows)
}

## Convert a growth_increment report block to a data frame.
.growth_increment_to_df <- function(rpt, par_set, label) {
  years <- names(rpt)[names(rpt) != "type"]
  rows <- vector("list", length(years))
  for (k in seq_along(years)) {
    y_rpt <- rpt[[years[k]]]
    df <- y_rpt$values
    df$year <- as.numeric(years[k])
    df$label <- label
    df$distribution <- y_rpt$distribution
    df$time_step <- y_rpt$time_step
    df$par_set <- par_set
    rows[[k]] <- df
  }
  .bind_rows_list(rows)
}

#' @rdname get_growth
#' @method get_growth casal2MPD
#' @export
"get_growth.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if ("type" %in% names(this_report)) {
      ## single run
      rpt_type <- this_report$type
      if (rpt_type == "age_length") {
        df <- .age_length_to_df(this_report, par_set = 1L, label = report_labels[i])
        df$par_set <- 1L
        rows[[i]] <- df
      } else if (rpt_type == "growth_increment") {
        df <- .growth_increment_to_df(this_report, par_set = 1L, label = report_labels[i])
        df$par_set <- 1L
        rows[[i]] <- df
      }
    } else {
      ## multi-run (-i)
      rpt_type <- this_report[[1L]]$type
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      inner <- vector("list", n_runs)
      if (rpt_type == "age_length") {
        for (dash_i in seq_len(n_runs)) {
          inner[[dash_i]] <- .age_length_to_df(this_report[[dash_i]],
            par_set = iter_labs[dash_i],
            label   = report_labels[i]
          )
        }
      } else if (rpt_type == "growth_increment") {
        for (dash_i in seq_len(n_runs)) {
          inner[[dash_i]] <- .growth_increment_to_df(this_report[[dash_i]],
            par_set = iter_labs[dash_i],
            label   = report_labels[i]
          )
        }
      } else {
        next
      }
      rows[[i]] <- .bind_rows_list(inner)
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_growth
#' @method get_growth list
#' @export
"get_growth.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_growth.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_growth
#' @method get_growth casal2TAB
#' @export
"get_growth.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  stop("get_growth for casal2TAB has not been implemented")
}
