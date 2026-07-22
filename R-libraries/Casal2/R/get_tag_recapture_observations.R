#' @title get_tag_recapture_observations
#' @description Extract tag-recapture observation data from a Casal2 model
#'   output into a tidy data frame. Handles both single-run and multi-run
#'   (\code{-r -i}) MPD output and named lists of multiple MPD objects.
#'   When different reports contain different column sets, only columns common
#'   to all are retained.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects produced by \code{extract.mpd()} or
#'   \code{extract.tabular()}.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a wide data frame whose columns come
#'   directly from the Casal2 report plus \code{observation_label},
#'   \code{observation_type}, \code{likelihood}, and \code{par_set}.
#'   For \code{casal2TAB} input: a long-form data frame with columns
#'   \code{iteration}, \code{parameter}, \code{value}, \code{category},
#'   \code{observation_label}, \code{observation_type}, and
#'   \code{likelihood}.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_tag_recapture_observations
#' @export get_tag_recapture_observations
get_tag_recapture_observations <- function(model, ...) {
  UseMethod("get_tag_recapture_observations", model)
}

#' @rdname get_tag_recapture_observations
#' @method get_tag_recapture_observations casal2MPD
#' @export
get_tag_recapture_observations.casal2MPD <- function(model, reformat_labels = TRUE, ...) {
  obs_types <- c("tag_recapture_by_length_for_growth", "tag_recapture_by_length", "tag_recapture_by_age")
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
      if (this_report$type != "observation") next
      if (!this_report$observation_type %in% obs_types) next
      df <- this_report$Values
      df$observation_label <- report_labels[i]
      df$observation_type <- this_report$observation_type
      df$likelihood <- this_report$likelihood
      df$par_set <- 1L
      rows[[i]] <- df
    } else {
      ## multi-run (-i)
      if (this_report[[1L]]$type != "observation") next
      if (!this_report[[1L]]$observation_type %in% obs_types) next
      n_runs <- length(this_report)
      obs_type_i <- this_report[[1L]]$observation_type
      likelihood_i <- this_report[[1L]]$likelihood
      inner <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        df <- this_report[[dash_i]]$Values
        df$observation_label <- report_labels[i]
        df$observation_type <- obs_type_i
        df$likelihood <- likelihood_i
        df$par_set <- dash_i
        inner[[dash_i]] <- df
      }
      rows[[i]] <- .safe_rbind(inner)
    }
  }
  .safe_rbind(rows)
}

#' @rdname get_tag_recapture_observations
#' @method get_tag_recapture_observations list
#' @export
get_tag_recapture_observations.list <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_tag_recapture_observations.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_tag_recapture_observations
#' @method get_tag_recapture_observations casal2TAB
#' @export
get_tag_recapture_observations.casal2TAB <- function(model, reformat_labels = TRUE, ...) {
  obs_types <- c("tag_recapture_by_length_for_growth", "tag_recapture_by_length", "tag_recapture_by_age")
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)

  report_chunks <- vector("list", length(model))
  for (i in seq_along(model)) {
    this_report <- model[[i]]
    if (is.null(this_report$observation_type)) next
    if (!this_report$observation_type %in% obs_types) next

    val_df <- this_report$values
    if (is.null(val_df) || nrow(val_df) == 0L) next
    if (is.null(colnames(val_df))) next

    col_match <- match(colnames(val_df), unique(colnames(val_df)))
    col_count <- ave(col_match, col_match, FUN = seq_along)

    val_molten <- .melt_matrix(val_df)
    colnames(val_molten) <- c("iteration", "parameter", "value")
    val_molten$iteration <- suppressWarnings(as.numeric(val_molten$iteration))

    category_by_col <- if (!is.null(this_report$categories)) this_report$categories[col_count] else NA_character_
    val_molten$category <- rep(category_by_col, each = nrow(val_df))
    val_molten$observation_label <- report_labels[i]
    val_molten$observation_type <- this_report$observation_type
    val_molten$likelihood <- this_report$likelihood
    report_chunks[[i]] <- val_molten
  }
  .bind_rows_list(report_chunks)
}
