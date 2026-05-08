#' @title get_fisheries
#' @description Extract mortality-instantaneous process (fishery) output from a
#'   Casal2 model into a tidy data frame.  Handles single-run and multi-run
#'   (\code{-r -i}) MPD output and named lists of multiple MPD objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a data frame with columns \code{year},
#'   \code{fishing_pressure}, \code{exploitation_rate}, \code{catch},
#'   \code{actual_catch}, \code{fishery}, \code{par_set} (integer index of the
#'   parameter set/minimisation; 1 for single run, ordered factor for multi-run
#'   \code{-i} output), and \code{label}.
#'   For \code{casal2TAB} (MCMC) input when \code{m_by_age = FALSE}: a data frame
#'   with columns \code{iteration} (integer MCMC posterior-sample index),
#'   \code{fishery}, \code{year}, \code{exploitation_rate}, \code{fishing_pressure},
#'   \code{catch}, \code{actual_catch}, and \code{label}.
#'   When \code{m_by_age = TRUE}: columns \code{iteration}, \code{category},
#'   \code{age}, \code{m_by_age}, and \code{label}.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_fisheries
#' @export get_fisheries
"get_fisheries" <- function(model, ...) {
  UseMethod("get_fisheries", model)
}

## Extract fishery data frame from one instantaneous-mortality process report.
.fisheries_from_report <- function(rpt, par_set, label) {
  f_ndx <- grepl("fishing_pressure\\[", names(rpt))
  exploit_ndx <- grepl("exploitation_rate", names(rpt))
  catch_ndx <- grepl("^catch", names(rpt))
  actual_catch_ndx <- grepl("actual_catch", names(rpt))
  if (!any(f_ndx)) {
    return(NULL)
  }
  ## Derive fishery names from the bracket notation "fishing_pressure[fishery_name]"
  f_names <- names(rpt)[f_ndx]
  start_i <- regexpr("\\[", f_names) + 1L
  stop_i <- regexpr("\\]", f_names) - 1L
  fisheries <- substring(f_names, start_i, stop_i)
  n_fisheries <- length(fisheries)
  ## Pre-compute index vectors -- which() inside a loop recomputes each iteration.
  f_idx <- which(f_ndx)
  exploit_idx <- which(exploit_ndx)
  catch_idx <- which(catch_ndx)
  actual_catch_idx <- which(actual_catch_ndx)
  rows <- vector("list", n_fisheries)
  for (f in seq_len(n_fisheries)) {
    rows[[f]] <- data.frame(
      year              = rpt$year,
      fishing_pressure  = rpt[[f_idx[f]]],
      exploitation_rate = rpt[[exploit_idx[f]]],
      catch             = rpt[[catch_idx[f]]],
      actual_catch      = rpt[[actual_catch_idx[f]]],
      fishery           = fisheries[f],
      par_set           = par_set,
      label             = label,
      stringsAsFactors  = FALSE
    )
  }
  .bind_rows_list(rows)
}

#' @rdname get_fisheries
#' @method get_fisheries casal2MPD
#' @export
"get_fisheries.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if ("type" %in% names(this_report)) {
      ## single run
      if (tolower(this_report$type) != "process") next
      if (tolower(this_report$sub_type) != "mortality_instantaneous") next
      rows[[i]] <- .fisheries_from_report(this_report,
        par_set = 1L,
        label = report_labels[i]
      )
    } else {
      ## multi-run (-i)
      if (tolower(this_report[[1L]]$type) != "process") next
      if (tolower(this_report[[1L]]$sub_type) != "mortality_instantaneous") next
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        inner[[dash_i]] <- .fisheries_from_report(this_report[[dash_i]],
          par_set = iter_labs[dash_i],
          label   = report_labels[i]
        )
      }
      rows[[i]] <- .bind_rows_list(inner)
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_fisheries
#' @method get_fisheries list
#' @export
"get_fisheries.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_fisheries.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_fisheries
#' @method get_fisheries casal2TAB
#' @param m_by_age Logical. If \code{TRUE} return the natural-mortality-by-age
#'   data; if \code{FALSE} (default) return the standard fishery catch/effort
#'   columns.
#' @export
"get_fisheries.casal2TAB" <- function(model, reformat_labels = TRUE, m_by_age = FALSE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  complete_df <- NULL

  for (i in seq_along(model)) {
    this_report <- model[[i]]
    if (is.null(this_report$type) || this_report$type != "process") next
    if (is.null(this_report$sub_type) || this_report$sub_type != "mortality_instantaneous") next

    fish_vals <- this_report$values
    collabs <- colnames(fish_vals)
    n_iter <- nrow(fish_vals)

    ## Parse column names: field[fishery_or_category][year_or_age]
    parts <- strsplit(collabs, "[", fixed = TRUE)
    field_vec <- vapply(parts, `[[`, character(1L), 1L)
    second_vec <- vapply(parts, function(x) substring(x[2L], 1L, nchar(x[2L]) - 1L), character(1L))
    third_vec <- vapply(parts, function(x) substring(x[3L], 1L, nchar(x[3L]) - 1L), character(1L))

    ## Separate m_by_age columns from regular fishery columns
    mba_mask <- field_vec == "m_by_age"
    fishery_mask <- !mba_mask

    ## ---- regular fishery data ----
    if (any(fishery_mask) && !m_by_age) {
      fv <- second_vec[fishery_mask]
      yv <- third_vec[fishery_mask]
      fldv <- field_vec[fishery_mask]
      orig_cols <- which(fishery_mask)

      fisheries_unique <- unique(fv)
      rows <- vector("list", length(fisheries_unique))

      for (k in seq_along(fisheries_unique)) {
        fsh <- fisheries_unique[k]
        fsh_mask <- fv == fsh
        flds_here <- unique(fldv[fsh_mask])
        yr_vals <- yv[fsh_mask & fldv == flds_here[1L]]
        n_years <- length(yr_vals)

        base_df <- data.frame(
          iteration        = rep(seq_len(n_iter), times = n_years),
          fishery          = fsh,
          year             = suppressWarnings(as.numeric(rep(yr_vals, each = n_iter))),
          stringsAsFactors = FALSE
        )
        for (fld in flds_here) {
          fld_cols <- orig_cols[fsh_mask & fldv == fld]
          base_df[[fld]] <- as.numeric(unlist(fish_vals[, fld_cols, drop = FALSE], use.names = FALSE))
        }
        rows[[k]] <- base_df
      }

      long_df <- do.call(rbind, rows)
      long_df$label <- report_labels[i]
      ## normalise actual_catches -> actual_catch for consistency
      if ("actual_catches" %in% colnames(long_df) && !"actual_catch" %in% colnames(long_df)) {
        colnames(long_df)[colnames(long_df) == "actual_catches"] <- "actual_catch"
      }
      complete_df <- rbind(complete_df, long_df)
    }

    ## ---- m_by_age data ----
    if (any(mba_mask) && m_by_age) {
      cv <- second_vec[mba_mask] ## category
      av <- third_vec[mba_mask] ## age
      orig_cols <- which(mba_mask)

      cats_unique <- unique(cv)
      rows <- vector("list", length(cats_unique))

      for (k in seq_along(cats_unique)) {
        cat_k <- cats_unique[k]
        cat_mask <- cv == cat_k
        age_vals <- av[cat_mask]
        n_ages <- length(age_vals)
        cat_cols <- orig_cols[cat_mask]

        rows[[k]] <- data.frame(
          iteration        = rep(seq_len(n_iter), times = n_ages),
          category         = cat_k,
          age              = suppressWarnings(as.numeric(rep(age_vals, each = n_iter))),
          m_by_age         = as.numeric(unlist(fish_vals[, cat_cols, drop = FALSE], use.names = FALSE)),
          stringsAsFactors = FALSE
        )
      }

      long_df <- do.call(rbind, rows)
      long_df$label <- report_labels[i]
      complete_df <- rbind(complete_df, long_df)
    }
  }
  complete_df
}
