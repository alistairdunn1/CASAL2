#' @title get_composition_observations
#' @description Extract composition (proportions-at-age or proportions-at-length)
#'   observation data from a Casal2 model output into a tidy data frame.
#'   Handles both single-run and multi-run MPD output and named lists of
#'   multiple MPD objects.  When different observation reports carry different
#'   column sets only the columns common to all are retained.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects produced by \code{extract.mpd()} or
#'   \code{extract.tabular()}.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a wide data frame whose columns come
#'   directly from the Casal2 report (typically \code{year}, \code{category},
#'   \code{observed}, \code{fits}, \code{residuals}, \code{pearson_residuals},
#'   \code{normalised_residuals}) plus \code{observation_label},
#'   \code{observation_type}, \code{likelihood}, and \code{par_set} (integer
#'   index of the parameter set/minimisation; 1 for single run, ordered factor
#'   for multi-run \code{-i} output).  Only columns present in all reports are
#'   retained.
#'   For \code{casal2TAB} (MCMC) input: a long-form data frame with columns
#'   \code{iteration} (integer MCMC posterior-sample index), \code{year},
#'   \code{category}, \code{bin}, \code{observation_label},
#'   \code{observation_type}, \code{likelihood}, \code{type} (one of
#'   \code{"fits"}, \code{"observed"}, \code{"residuals"},
#'   \code{"pearson_residuals"}, \code{"normalised_residuals"}), and
#'   \code{value}.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_composition_observations
#' @export get_composition_observations
#' @seealso \code{\link{has_composition_observations}}
"get_composition_observations" <- function(model, ...) {
  UseMethod("get_composition_observations", model)
}

#' @title has_composition_observations
#' @description Check whether a Casal2 model output contains any composition
#'   (proportions-at-age / proportions-at-length) observations.
#' @param model A \code{casal2MPD} object.
#' @param \dots Further arguments (currently unused).
#' @return \code{TRUE} if at least one composition observation report is present.
#' @rdname has_composition_observations
#' @export has_composition_observations
"has_composition_observations" <- function(model, ...) {
  obs_types <- c(
    "proportions_at_age", "proportions_at_length",
    "process_removals_by_age", "process_removals_by_length"
  )
  for (i in seq_along(model)) {
    this_report <- model[[i]]
    if ("type" %in% names(this_report) &&
      this_report$type == "observation" &&
      this_report$observation_type %in% obs_types) {
      return(TRUE)
    }
  }
  FALSE
}

#' @rdname get_composition_observations
#' @method get_composition_observations casal2MPD
#' @export
"get_composition_observations.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  obs_types <- c(
    "proportions_at_age", "proportions_at_length",
    "process_removals_by_age", "process_removals_by_length"
  )
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
      ## observation_type and likelihood are constant across runs -- hoist outside loop.
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

#' @rdname get_composition_observations
#' @method get_composition_observations list
#' @export
"get_composition_observations.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_composition_observations.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_composition_observations
#' @method get_composition_observations casal2TAB
#' @export
"get_composition_observations.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  obs_types <- c(
    "proportions_at_age", "proportions_at_length",
    "process_removals_by_age", "process_removals_by_length"
  )
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)

  ## Collect per-report results in a list; bind once at the end (avoids O(n^2) rbind growth).
  report_chunks <- vector("list", length(model))

  field_types <- c("fits", "observed", "residuals", "pearson_residuals", "normalised_residuals")
  ft_pattern <- paste(field_types, collapse = "|")
  ## Column format: "label.(fits|observed|...)[year][category][bin]"
  col_regex <- paste0("\\.(", ft_pattern, ")\\[([^]]+)\\]\\[([^]]+)\\]\\[([^]]+)\\]$")

  for (i in seq_along(model)) {
    this_report <- model[[i]]
    if (is.null(this_report$observation_type)) next
    if (!this_report$observation_type %in% obs_types) next

    val_df <- this_report$values
    if (is.null(val_df) || nrow(val_df) == 0L) next
    collabs <- colnames(val_df)
    n_iter <- nrow(val_df)
    iter_col <- collabs[tolower(collabs) == "iteration"]
    chain_col <- collabs[tolower(collabs) == "chain"]
    iter_base <- if (length(iter_col) == 1L) as.numeric(val_df[[iter_col]]) else seq_len(n_iter)
    chain_base <- if (length(chain_col) == 1L) val_df[[chain_col]] else NULL

    ## Parse all four capture groups in one pass: do.call(rbind) builds a
    ## character matrix directly rather than four separate vapply sweeps.
    m <- regmatches(collabs, regexec(col_regex, collabs))
    parsed <- do.call(rbind, lapply(m, function(x) {
      if (length(x) == 5L) x[2:5] else rep(NA_character_, 4L)
    }))
    ftype_vec <- parsed[, 1L]
    year_vec <- parsed[, 2L]
    category_vec <- parsed[, 3L]
    bin_vec <- parsed[, 4L]

    valid <- !is.na(ftype_vec)
    obs_mask <- valid & ftype_vec == "observed"
    if (!any(obs_mask)) next

    yr_u <- year_vec[obs_mask]
    cat_u <- category_vec[obs_mask]
    bin_u <- bin_vec[obs_mask]
    n_obs <- sum(obs_mask)
    obs_key <- paste(yr_u, cat_u, bin_u, sep = "\r")

    iter_vec <- rep(iter_base, times = n_obs)
    chain_vec <- if (!is.null(chain_base)) rep(chain_base, times = n_obs) else NULL
    year_long <- suppressWarnings(as.numeric(rep(yr_u, each = n_iter)))
    cat_long <- rep(cat_u, each = n_iter)
    bin_long <- suppressWarnings(as.numeric(rep(bin_u, each = n_iter)))

    ft_chunks <- vector("list", length(field_types))
    for (ft in field_types) {
      ft_mask <- valid & ftype_vec == ft
      if (!any(ft_mask)) next
      ft_key <- paste(year_vec[ft_mask], category_vec[ft_mask], bin_vec[ft_mask], sep = "\r")
      idx <- match(obs_key, ft_key)
      ft_cols <- which(ft_mask)

      ## Extract all matched columns as a matrix in a single C-level call,
      ## replacing the previous per-observation R loop.  Column-major unrolling
      ## via as.numeric() matches iter_vec = rep(seq_len(n_iter), times = n_obs).
      val_mat <- matrix(NA_real_, nrow = n_iter, ncol = n_obs)
      valid_j <- which(!is.na(idx))
      if (length(valid_j) > 0L) {
        val_mat[, valid_j] <- as.matrix(val_df[, ft_cols[idx[valid_j]], drop = FALSE])
      }

      ft_chunks[[ft]] <- data.frame(
        iteration         = iter_vec,
        year              = year_long,
        category          = cat_long,
        bin               = bin_long,
        observation_label = report_labels[i],
        observation_type  = this_report$observation_type,
        likelihood        = this_report$likelihood,
        type              = ft,
        value             = as.numeric(val_mat),
        stringsAsFactors  = FALSE
      )
      if (!is.null(chain_vec)) {
        ft_chunks[[ft]]$chain <- chain_vec
      }
    }

    report_chunks[[i]] <- .safe_rbind(ft_chunks)
  }
  .safe_rbind(report_chunks)
}
