#' @title get_profile
#' @description Extract profile run data from a Casal2 MPD model output into a
#'   tidy data frame combining the profiled parameter values with the objective
#'   function components at each profile point.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD} object produced by \code{extract.mpd()} from
#'   a Casal2 profile run (\code{-p} flag), or a named \code{list} of
#'   \code{casal2MPD} objects.
#' @param aggregate_obs Logical. Aggregate observation log-likelihood components
#'   over years (default \code{TRUE}).
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return A data frame with columns \code{component}, \code{parameter_values}
#'   (estimated value from each MPD run), \code{negative_loglikelihood}, and
#'   \code{parameter}. For list input, the output additionally includes
#'   \code{model_label}.
#' @rdname get_profile
#' @export get_profile
"get_profile" <- function(model, ...) {
  UseMethod("get_profile", model)
}

#' @rdname get_profile
#' @method get_profile casal2MPD
#' @export
"get_profile.casal2MPD" <- function(model, aggregate_obs = TRUE, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)

  ## Verify this was a profile run from the call in the header.
  if (!"header" %in% report_labels) {
    stop("No header report found in model output.")
  }
  header_idx <- which(report_labels == "header")[1L]
  if (!grepl("-p", model[[header_idx]]$call, fixed = TRUE)) {
    stop(paste0(
      "Not a profile Casal2 output. Cannot find '-p' in '",
      model[[header_idx]]$call, "'."
    ))
  }

  ## Locate profile report and extract the profiled parameter.
  profile_report <- NULL
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    rpt <- if ("type" %in% names(this_report)) this_report else this_report[[1L]]
    if (!is.null(rpt$type) && tolower(rpt$type) == "profile") {
      profile_report <- rpt
      break
    }
  }
  if (is.null(profile_report)) {
    stop("No profile report found in model output.")
  }

  if (is.null(profile_report$parameter)) {
    stop("Profile report is missing 'parameter'.")
  }
  profile_parameter <- as.character(profile_report$parameter)[1L]

  ## Locate estimate_value report and build one-row-per-run parameter estimates.
  estimate_report <- NULL
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    rpt <- if ("type" %in% names(this_report)) this_report else this_report[[1L]]
    if (!is.null(rpt$type) && tolower(rpt$type) == "estimate_value") {
      estimate_report <- this_report
      break
    }
  }
  if (is.null(estimate_report)) {
    stop("No estimate_value report found in model output.")
  }

  if ("type" %in% names(estimate_report)) {
    run_labels <- "1"
    estimate_rows <- estimate_report$values[1L, , drop = FALSE]
    rownames(estimate_rows) <- run_labels
  } else {
    run_labels <- names(estimate_report)
    estimate_list <- vector("list", length(estimate_report))
    for (i in seq_along(estimate_report)) {
      vals <- estimate_report[[i]]$values
      estimate_list[[i]] <- vals[1L, , drop = FALSE]
      rownames(estimate_list[[i]]) <- run_labels[[i]]
    }
    estimate_rows <- do.call(rbind, estimate_list)
  }
  estimate_rows[] <- lapply(estimate_rows, as.numeric)
  ## Retrieve objective function at each profile step
  obj_df <- get_objective_function(model,
    aggregate_obs = aggregate_obs,
    reformat_labels = reformat_labels
  )
  if (is.null(obj_df)) {
    stop("No objective_function report found in model output.")
  }

  if (!all(c("component", "negative_loglik") %in% names(obj_df))) {
    stop("Unexpected objective_function format: expected columns 'component' and 'negative_loglik'.")
  }
  if (!"par_set" %in% names(obj_df)) obj_df$par_set <- 1L
  if (is.factor(obj_df$par_set)) {
    par_order <- levels(obj_df$par_set)
  } else {
    par_order <- unique(as.character(obj_df$par_set))
  }

  ## Prefer the explicit profiled parameter column from estimate values; otherwise infer.
  infer_profile_parameter <- function(est_df) {
    num_df <- est_df[, vapply(est_df, is.numeric, logical(1L)), drop = FALSE]
    if (ncol(num_df) == 0L) stop("No numeric parameter columns found in estimate_value report.")

    n_runs <- nrow(num_df)
    varying <- vapply(num_df, function(x) length(unique(x)) > 1L, logical(1L))
    candidates <- names(num_df)[varying]
    if (length(candidates) == 0L) {
      stop("Could not infer profiled parameter: no varying estimated parameter across profile runs.")
    }

    best_name <- NULL
    best_score <- Inf
    for (nm in candidates) {
      x <- as.numeric(num_df[[nm]])
      if (length(unique(x)) != n_runs) next
      dx <- diff(x)
      monotonic <- all(dx >= 0) || all(dx <= 0)
      if (!monotonic) next
      mdx <- mean(abs(dx))
      score <- if (mdx == 0) Inf else stats::sd(dx) / mdx
      if (is.finite(score) && score < best_score) {
        best_score <- score
        best_name <- nm
      }
    }

    if (!is.null(best_name)) {
      return(best_name)
    }

    vars <- vapply(num_df[candidates], stats::var, numeric(1L))
    names(vars)[which.max(vars)]
  }

  estimate_parameter <- if (profile_parameter %in% colnames(estimate_rows)) {
    profile_parameter
  } else {
    infer_profile_parameter(estimate_rows)
  }

  if (all(par_order %in% rownames(estimate_rows))) {
    estimated_value_lookup <- as.numeric(estimate_rows[par_order, estimate_parameter])
  } else {
    numeric_order <- suppressWarnings(as.integer(par_order))
    if (!anyNA(numeric_order) &&
      all(numeric_order >= 1L) &&
      all(numeric_order <= nrow(estimate_rows))) {
      estimated_value_lookup <- as.numeric(estimate_rows[numeric_order, estimate_parameter])
    } else if (length(par_order) == nrow(estimate_rows)) {
      estimated_value_lookup <- as.numeric(estimate_rows[[estimate_parameter]])
    } else {
      stop(paste0(
        "Objective-function parameter sets (", length(par_order),
        ") do not match estimate_value runs (", nrow(estimate_rows), ")."
      ))
    }
  }

  lookup_df <- data.frame(
    par_set = par_order,
    parameter_values = estimated_value_lookup,
    stringsAsFactors = FALSE
  )
  obj_df$par_set <- as.character(obj_df$par_set)
  molten_profile <- merge(obj_df, lookup_df, by = "par_set", all.x = TRUE, sort = FALSE)
  molten_profile <- molten_profile[, c("component", "parameter_values", "negative_loglik")]
  colnames(molten_profile)[colnames(molten_profile) == "negative_loglik"] <- "negative_loglikelihood"
  molten_profile$parameter <- profile_parameter
  molten_profile
}

#' @rdname get_profile
#' @method get_profile list
#' @export
"get_profile.list" <- function(model, aggregate_obs = TRUE, reformat_labels = TRUE, ...) {
  .list_method(
    model = model,
    FUN = get_profile.casal2MPD,
    reformat_labels = reformat_labels,
    aggregate_obs = aggregate_obs,
    ...
  )
}
