#' @title get_error_value_table
#' @description Iterate over all observation reports in a Casal2 model output
#'   and return a summary of error values, process errors, and adjusted errors
#'   by year and observation.  Handles single-run (\code{-r}), multi-run
#'   (\code{-r -i}), and named-list inputs.
#' @author Casal2 Development Team (ported from r4Casal2 by Craig Marsh,
#'   A Dunn)
#' @param model A \code{casal2MPD} or named \code{list} of \code{casal2MPD}
#'   objects produced by \code{extract.mpd()}.
#' @param as.table Logical (default \code{TRUE}).  When \code{TRUE} the three
#'   error metrics are each returned as a wide data frame (rows = observation,
#'   columns = year).  When \code{FALSE} the raw long-form data frame is
#'   returned instead.
#' @param reformat_labels Logical (default \code{TRUE}).  Reformat default
#'   Casal2 report labels.
#' @param \dots Further arguments (currently unused).
#' @return When \code{as.table = FALSE}: a long-form data frame with columns
#'   \describe{
#'     \item{\code{label}}{Observation report label.}
#'     \item{\code{type}}{Observation type (e.g. \code{"proportions_at_age"}).
#'       Types beginning with \code{"process_removals_by"} are re-labelled as
#'       \code{"fishery_..."}.}
#'     \item{\code{likelihood}}{Likelihood name.}
#'     \item{\code{error_value_multiplier}}{Error value multiplier scalar from
#'       the report (\code{NA} if absent).}
#'     \item{\code{likelihood_multiplier}}{Likelihood multiplier scalar from the
#'       report (\code{NA} if absent).}
#'     \item{\code{year}}{Model year.}
#'     \item{\code{error_value}}{Mean error value across bins for that year.}
#'     \item{\code{process_error}}{Mean process error across bins for that year.}
#'     \item{\code{adjusted_error}}{Mean adjusted error across bins for that year.}
#'     \item{\code{par_set}}{Integer index of the parameter set (1 for a
#'       single-run model; ordered factor of run labels for multi-run
#'       \code{-i} output).}
#'   }
#'   When \code{as.table = TRUE}: a named list with elements
#'   \describe{
#'     \item{\code{observation_error}}{Wide data frame; rows are observations,
#'       columns are years, cells are mean \code{error_value}.}
#'     \item{\code{process_error}}{Wide data frame; same shape,
#'       \code{process_error} values.}
#'     \item{\code{adjusted_error}}{Wide data frame; same shape,
#'       \code{adjusted_error} values.}
#'     \item{\code{error_value_multiplier}}{Vector of unique error value
#'       multiplier values from the reports.}
#'     \item{\code{likelihood_multiplier}}{Vector of unique likelihood
#'       multiplier values from the reports.}
#'   }
#'   Returns \code{NULL} when no observation reports are found.
#' @rdname get_error_value_table
#' @export get_error_value_table
"get_error_value_table" <- function(model, as.table = TRUE, ...) {
  UseMethod("get_error_value_table", model)
}

## ---------------------------------------------------------------------------
## Internal helper: wide-pivot one column of a long data frame.
## id_cols  -- character vector of columns that identify each row
## val_col  -- name of the column to spread
## year_col -- name of the year column
## all_years -- sorted numeric/integer vector of all years to include
## ---------------------------------------------------------------------------
.ev_to_wide <- function(df, id_cols, val_col, year_col, all_years) {
  id_df <- unique(df[, id_cols, drop = FALSE])
  rownames(id_df) <- NULL
  mat <- matrix(
    NA_real_,
    nrow = nrow(id_df),
    ncol = length(all_years),
    dimnames = list(NULL, as.character(all_years))
  )
  for (r in seq_len(nrow(id_df))) {
    mask <- rep(TRUE, nrow(df))
    for (col in id_cols) {
      mask <- mask & (df[[col]] == id_df[[col]][r])
    }
    sub <- df[mask, , drop = FALSE]
    yi <- match(as.character(sub[[year_col]]), as.character(all_years))
    mat[r, yi] <- sub[[val_col]]
  }
  cbind(id_df, as.data.frame(mat, stringsAsFactors = FALSE))
}

#' @rdname get_error_value_table
#' @method get_error_value_table casal2MPD
#' @export
"get_error_value_table.casal2MPD" <- function(model, as.table = TRUE,
                                              reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) {
    reformat_default_labels(names(model))
  } else {
    names(model)
  }

  ## Identify default-generated reports (__label__) whose stripped name already
  ## exists as a user-defined (non-default) report.  These are skipped to avoid
  ## duplicating observations that already appear under the user-defined label.
  orig_names <- names(model)
  is_default <- startsWith(orig_names, "__") & endsWith(orig_names, "__")
  stripped <- ifelse(is_default,
    substring(orig_names, 3L, nchar(orig_names) - 2L),
    orig_names
  )
  skip_default <- is_default & (stripped %in% stripped[!is_default])

  rows <- vector("list", length(model))

  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    if (skip_default[i]) next
    this_report <- model[[i]]

    if ("type" %in% names(this_report)) {
      ## --- single run --------------------------------------------------------
      if (this_report$type != "observation") next

      rows[[i]] <- .ev_rows_from_report(
        this_report,
        label   = report_labels[i],
        par_set = 1L
      )
    } else {
      ## --- multi-run (-i) ----------------------------------------------------
      if (this_report[[1L]]$type != "observation") next

      n_runs <- length(this_report)
      run_names <- names(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        inner[[dash_i]] <- .ev_rows_from_report(
          this_report[[dash_i]],
          label   = report_labels[i],
          par_set = dash_i
        )
      }
      run_df <- .safe_rbind(inner)
      if (!is.null(run_df)) {
        ps_levels <- if (!is.null(run_names)) run_names else as.character(seq_len(n_runs))
        run_df$par_set <- factor(run_df$par_set,
          levels = seq_len(n_runs),
          labels = ps_levels, ordered = TRUE
        )
      }
      rows[[i]] <- run_df
    }
  }

  complete_df <- .safe_rbind(rows)
  if (is.null(complete_df)) {
    return(NULL)
  }

  ## re-label process-removals types
  chg <- grepl("^process_removals_by", complete_df$type)
  complete_df$type[chg] <- paste0("fishery_", substring(complete_df$type[chg], 22L))

  if (!as.table) {
    return(complete_df)
  }

  ## --- wide pivot -----------------------------------------------------------
  all_years <- sort(unique(complete_df$year))
  id_cols <- c(
    "label", "type", "likelihood",
    "error_value_multiplier", "likelihood_multiplier", "par_set"
  )

  list(
    observation_error = .ev_to_wide(
      complete_df, id_cols, "error_value",
      "year", all_years
    ),
    process_error = .ev_to_wide(
      complete_df, id_cols, "process_error",
      "year", all_years
    ),
    adjusted_error = .ev_to_wide(
      complete_df, id_cols, "adjusted_error",
      "year", all_years
    ),
    error_value_multiplier = unique(complete_df$error_value_multiplier),
    likelihood_multiplier = unique(complete_df$likelihood_multiplier)
  )
}

## ---------------------------------------------------------------------------
## Extract one observation report into a data frame of per-year error metrics.
## Returns NULL if the report is not an observation.
## ---------------------------------------------------------------------------
.ev_rows_from_report <- function(rpt, label, par_set) {
  if (!"type" %in% names(rpt) || rpt$type != "observation") {
    return(NULL)
  }

  vals <- rpt$Values
  if (is.null(vals) || !all(c(
    "year", "error_value", "process_error",
    "adjusted_error"
  ) %in% names(vals))) {
    return(NULL)
  }

  ## per-year mean across bins
  agg <- aggregate(
    cbind(error_value, process_error, adjusted_error) ~ year,
    data    = vals,
    FUN     = mean,
    na.rm   = TRUE
  )

  ev_mult <- if (!is.null(rpt$error_value_multiplier)) rpt$error_value_multiplier else NA_real_
  lk_mult <- if (!is.null(rpt$likelihood_multiplier)) rpt$likelihood_multiplier else NA_real_

  data.frame(
    label                  = label,
    type                   = rpt$observation_type,
    likelihood             = rpt$likelihood,
    error_value_multiplier = ev_mult,
    likelihood_multiplier  = lk_mult,
    year                   = agg$year,
    error_value            = agg$error_value,
    process_error          = agg$process_error,
    adjusted_error         = agg$adjusted_error,
    par_set                = par_set,
    stringsAsFactors       = FALSE
  )
}

#' @rdname get_error_value_table
#' @method get_error_value_table list
#' @export
"get_error_value_table.list" <- function(model, as.table = TRUE,
                                         reformat_labels = TRUE, ...) {
  .list_method(model, get_error_value_table.casal2MPD,
    reformat_labels = reformat_labels
  )
}

#' @rdname get_error_value_table
#' @method get_error_value_table default
#' @export
"get_error_value_table.default" <- function(model, as.table = TRUE,
                                            reformat_labels = TRUE, ...) {
  get_error_value_table.casal2MPD(model,
    as.table        = as.table,
    reformat_labels = reformat_labels,
    ...
  )
}
