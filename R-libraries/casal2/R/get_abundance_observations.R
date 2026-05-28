#' @title get_abundance_observations
#' @description Extract abundance or biomass observation data from a Casal2
#'   model output into a tidy data frame.  Handles both single-run (\code{-r})
#'   and multi-run (\code{-r -i}) MPD output and named lists of multiple MPD
#'   objects.  When reports from different observations contain different column
#'   sets (e.g. some include residuals and some do not), only the columns common
#'   to all are retained.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects produced by \code{extract.mpd()} or
#'   \code{extract.tabular()}.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return For \code{casal2MPD} input: a wide data frame whose columns come
#'   directly from the Casal2 report (typically \code{year}, \code{observed},
#'   \code{fits}, \code{residuals}, \code{pearson_residuals},
#'   \code{normalised_residuals}, \code{U_CI}, \code{L_CI}) plus
#'   \code{observation_label}, \code{observation_type}, \code{likelihood}, and
#'   \code{par_set} (integer index of the parameter set/minimisation; 1 for
#'   single run, ordered factor for multi-run \code{-i} output).  Only columns
#'   present in all reports are retained.
#'   For \code{casal2TAB} (MCMC) input: a long-form data frame with columns
#'   \code{iteration} (integer MCMC posterior-sample index), \code{year},
#'   \code{observation_label}, \code{observation_type}, \code{likelihood},
#'   \code{type} (one of \code{"fits"}, \code{"observed"}, \code{"residuals"},
#'   \code{"pearson_residuals"}, \code{"normalised_residuals"}), and
#'   \code{value}.
#'   Returns \code{NULL} when no matching reports are found.
#' @rdname get_abundance_observations
#' @export get_abundance_observations
#' @seealso \code{\link{has_abundance_observations}}
"get_abundance_observations" <- function(model, ...) {
  UseMethod("get_abundance_observations", model)
}

#' @title has_abundance_observations
#' @description Check whether a Casal2 model output contains any abundance or
#'   biomass observations.
#' @param model A \code{casal2MPD} object.
#' @param \dots Further arguments (currently unused).
#' @return \code{TRUE} if at least one abundance/biomass observation report is present.
#' @rdname has_abundance_observations
#' @export has_abundance_observations
"has_abundance_observations" <- function(model, ...) {
  obs_types <- c("biomass", "abundance")
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

#' @rdname get_abundance_observations
#' @method get_abundance_observations casal2MPD
#' @export
"get_abundance_observations.casal2MPD" <- function(model, reformat_labels = TRUE, ...) {
  obs_types <- c("biomass", "abundance")
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
  complete_df <- .safe_rbind(rows)
  if (is.null(complete_df)) {
    return(NULL)
  }
  ## calculate L_CI / U_CI -- implemented for normal and lognormal likelihoods
  complete_df$U_CI <- NA_real_
  complete_df$L_CI <- NA_real_
  ## normal
  normal_ndx <- complete_df$likelihood == "normal"
  if (any(normal_ndx)) {
    total_sigma <- complete_df$observed * complete_df$adjusted_error
    complete_df$U_CI[normal_ndx] <- complete_df$observed[normal_ndx] + 1.96 * total_sigma[normal_ndx]
    complete_df$L_CI[normal_ndx] <- complete_df$observed[normal_ndx] - 1.96 * total_sigma[normal_ndx]
  }
  ## lognormal
  lognormal_ndx <- complete_df$likelihood == "lognormal"
  if (any(lognormal_ndx)) {
    total_sigma <- sqrt(log(1 + complete_df$adjusted_error[lognormal_ndx]^2))
    Mean <- log(complete_df$observed[lognormal_ndx]) - 0.5 * total_sigma^2
    complete_df$U_CI[lognormal_ndx] <- exp(Mean + 1.96 * total_sigma)
    complete_df$L_CI[lognormal_ndx] <- exp(Mean - 1.96 * total_sigma)
  }
  return(complete_df)
}

#' @rdname get_abundance_observations
#' @method get_abundance_observations list
#' @export
"get_abundance_observations.list" <- function(model, reformat_labels = TRUE, ...) {
  .list_method(model, get_abundance_observations.casal2MPD, reformat_labels = reformat_labels)
}

#' @rdname get_abundance_observations
#' @method get_abundance_observations casal2TAB
#' @export
"get_abundance_observations.casal2TAB" <- function(model, reformat_labels = TRUE, ...) {
  obs_types <- c("biomass", "abundance")
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  complete_df <- NULL

  field_types <- c("fits", "observed", "residuals", "pearson_residuals", "normalised_residuals")
  ft_pattern <- paste(field_types, collapse = "|")
  ## Column format: "label.(fits|observed|...)[year][category][bin]"
  ## For biomass/abundance, bin is always "1"
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

    ## Parse field_type, year, category, bin from each column name
    m <- regmatches(collabs, regexec(col_regex, collabs))

    ftype_vec <- vapply(m, function(x) if (length(x) >= 2L) x[2L] else NA_character_, character(1L))
    year_vec <- vapply(m, function(x) if (length(x) >= 3L) x[3L] else NA_character_, character(1L))
    category_vec <- vapply(m, function(x) if (length(x) >= 4L) x[4L] else NA_character_, character(1L))
    bin_vec <- vapply(m, function(x) if (length(x) >= 5L) x[5L] else NA_character_, character(1L))

    valid <- !is.na(ftype_vec)
    obs_mask <- valid & ftype_vec == "observed"
    if (!any(obs_mask)) next

    yr_u <- year_vec[obs_mask]
    cat_u <- category_vec[obs_mask]
    n_obs <- sum(obs_mask)

    ## Key columns repeated for every field type
    iter_vec <- rep(iter_base, times = n_obs)
    chain_vec <- if (!is.null(chain_base)) rep(chain_base, times = n_obs) else NULL
    year_long <- suppressWarnings(as.numeric(rep(yr_u, each = n_iter)))
    cat_long <- rep(cat_u, each = n_iter)
    obs_key <- paste(yr_u, cat_u, sep = "\r")

    ## Build long form: one row per (iteration, year, category, type)
    ft_chunks <- vector("list", length(field_types))
    for (ft in field_types) {
      ft_mask <- valid & ftype_vec == ft
      if (!any(ft_mask)) next
      ft_cols <- which(ft_mask)
      ft_key <- paste(year_vec[ft_mask], category_vec[ft_mask], sep = "\r")
      idx <- match(obs_key, ft_key)
      vals <- rep(NA_real_, n_iter * n_obs)
      for (j in seq_len(n_obs)) {
        if (!is.na(idx[j])) {
          vals[((j - 1L) * n_iter + 1L):(j * n_iter)] <- as.numeric(val_df[[ft_cols[idx[j]]]])
        }
      }
      ft_chunks[[ft]] <- data.frame(
        iteration         = iter_vec,
        year              = year_long,
        category          = cat_long,
        observation_label = report_labels[i],
        observation_type  = this_report$observation_type,
        likelihood        = this_report$likelihood,
        type              = ft,
        value             = vals,
        stringsAsFactors  = FALSE
      )
      if (!is.null(chain_vec)) {
        ft_chunks[[ft]]$chain <- chain_vec
      }
    }

    complete_df <- .safe_rbind(list(complete_df, .safe_rbind(ft_chunks)))
  }
  complete_df
}
