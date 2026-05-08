#' @title get_BH_recruitment
#' @description Extract Beverton-Holt recruitment process output from a Casal2
#'   model into a tidy data frame.  Handles single-run and multi-run
#'   (\code{-r -i}) MPD output and named lists of multiple MPD objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects produced by \code{extract.mpd()} or
#'   \code{extract.tabular()}.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a data frame with columns
#'   \code{model_year}, \code{spawn_event_year}, \code{ycs_years},
#'   \code{standardised_recruitment_multipliers}, \code{recruitment_multipliers},
#'   \code{recruits}, \code{true_ycs}, \code{r0}, \code{b0},
#'   \code{recruit_event_SSB}, \code{recruit_event_SSB_percent}, \code{ssb},
#'   \code{ssb_offset}, \code{label}, and \code{par_set} (integer index of the
#'   parameter set/minimisation; 1 for a single run, ordered factor for
#'   multi-run \code{-i} output).
#'   For \code{casal2TAB} (MCMC) input: a named list with elements
#'   \code{non_multi_column_df} (scalar quantities per iteration: \code{iteration},
#'   scalar component columns, \code{label}) and \code{multi_column_df}
#'   (year-varying quantities in long form: \code{iteration}, component columns,
#'   corresponding \code{_year} columns, \code{label}).  \code{iteration} is the
#'   integer MCMC posterior-sample index.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_BH_recruitment
#' @export get_BH_recruitment
"get_BH_recruitment" <- function(model, ...) {
  UseMethod("get_BH_recruitment", model)
}

## Build a single-run recruitment data frame from a report object.
.bh_df_from_report <- function(rpt, par_set, label) {
  df <- data.frame(
    model_year = rpt$model_year,
    spawn_event_year = rpt$spawn_event_year,
    ycs_years = rpt$spawn_event_year,
    standardised_recruitment_multipliers = rpt$standardised_recruitment_multipliers,
    recruitment_multipliers = rpt$recruitment_multipliers,
    recruits = rpt$recruits,
    true_ycs = rpt$true_ycs,
    r0 = rpt$r0,
    b0 = rpt$b0,
    recruit_event_SSB = rpt$recruit_event_SSB,
    recruit_event_SSB_percent = rpt$recruit_event_SSB_percent,
    ssb = rpt$ssb,
    ssb_offset = rpt$ssb_offset,
    par_set = par_set,
    label = label,
    stringsAsFactors = FALSE
  )
  ## Length-based models may not report 'age'
  df$age <- if (!is.null(rpt$age)) rpt$age else NA_real_
  df
}

#' @rdname get_BH_recruitment
#' @method get_BH_recruitment casal2MPD
#' @export
"get_BH_recruitment.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if ("type" %in% names(this_report)) {
      ## single run
      if (this_report$type != "process") next
      if (this_report$sub_type != "recruitment_beverton_holt") next
      rows[[i]] <- .bh_df_from_report(this_report, par_set = 1L, label = report_labels[i])
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "process") next
      if (this_report[[1L]]$sub_type != "recruitment_beverton_holt") next
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        inner[[dash_i]] <- .bh_df_from_report(this_report[[dash_i]],
          par_set = iter_labs[dash_i],
          label   = report_labels[i]
        )
      }
      df <- .bind_rows_list(inner)
      df$par_set <- factor(df$par_set, ordered = TRUE)
      rows[[i]] <- df
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_BH_recruitment
#' @method get_BH_recruitment list
#' @export
"get_BH_recruitment.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_BH_recruitment.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_BH_recruitment
#' @method get_BH_recruitment casal2TAB
#' @export
"get_BH_recruitment.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  recruit_multi_col_df <- NULL
  recruit_df <- NULL

  for (i in seq_along(model)) {
    this_report <- model[[i]]
    if (this_report$type != "process") next
    if (this_report$sub_type != "recruitment_beverton_holt") next

    vals <- this_report$values
    collabs <- colnames(vals)
    n_iter <- nrow(vals)

    ## First token before "[" identifies the component; duplicates (e.g.
    ## recruitment_multipliers vs standardised_recruitment_multipliers) are
    ## handled by tracking already-assigned columns.
    first_component <- vapply(
      strsplit(collabs, "[", fixed = TRUE),
      `[[`, character(1L), 1L
    )
    components <- unique(first_component)
    cols_saved <- integer(0)

    non_multi_column_df <- data.frame(iteration = seq_len(n_iter))
    multi_column_df <- data.frame(iteration = rep(seq_len(n_iter), times = 1L))

    first_multi <- TRUE

    for (j in seq_along(components)) {
      col_ndx <- which(first_component == components[j])
      ## drop already-saved column indices to avoid double-counting
      col_ndx <- col_ndx[!col_ndx %in% cols_saved]
      if (length(col_ndx) == 0L) next
      cols_saved <- c(cols_saved, col_ndx)

      if (length(col_ndx) == 1L) {
        ## scalar component -- one value per iteration
        non_multi_column_df[[components[j]]] <- as.numeric(vals[, col_ndx])
      } else {
        ## year-varying component -- reshape wide -> long
        sub_mat <- as.matrix(vals[, col_ndx, drop = FALSE])
        ## extract year labels from column names like "recruits[2000]"
        yr_raw <- sub(".*\\[(.*)\\]$", "\\1", colnames(sub_mat))

        long_iter <- rep(seq_len(n_iter), times = ncol(sub_mat))
        long_year <- rep(yr_raw, each = n_iter)
        long_value <- as.numeric(sub_mat)

        if (first_multi) {
          multi_column_df <- data.frame(
            iteration              = long_iter,
            stringsAsFactors       = FALSE
          )
          first_multi <- FALSE
        }
        multi_column_df[[components[j]]] <- long_value
        multi_column_df[[paste0(components[j], "_year")]] <- long_year
      }
    }

    non_multi_column_df$label <- report_labels[i]
    multi_column_df$label <- report_labels[i]

    recruit_df <- rbind(recruit_df, non_multi_column_df)
    recruit_multi_col_df <- rbind(recruit_multi_col_df, multi_column_df)
  }

  list(multi_column_df = recruit_multi_col_df, non_multi_column_df = recruit_df)
}
