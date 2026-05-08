#' @title aggregate_single_objective_report
#' @description Aggregate objective function components from a single Casal2
#'   \code{objective_function} report.  Observation components are summed over
#'   all years for a given observation label.
#' @author Casal2 Development Team
#' @param values Named numeric vector from \code{report$values} of an
#'   \code{objective_function} report.
#' @return A data frame with columns \code{component} and \code{negative_loglik}.
#' @rdname aggregate_single_objective_report
#' @export aggregate_single_objective_report
aggregate_single_objective_report <- function(values) {
  obj_labels <- names(values)
  parts <- strsplit(obj_labels, "->", fixed = TRUE)
  type_vec <- vapply(parts, `[[`, character(1L), 1L)
  label_vec <- vapply(
    parts, function(x) if (length(x) >= 2L) x[[2L]] else NA_character_,
    character(1L)
  )
  new_label <- ifelse(!is.na(label_vec), paste0("-", label_vec), "")
  unique_vals <- paste0(type_vec, new_label)
  ## rowsum() is vectorised C code: groups and sums in one pass, preserving order.
  agg <- rowsum(as.numeric(values), group = unique_vals, reorder = FALSE)
  data.frame(component = rownames(agg), negative_loglik = agg[, 1L], stringsAsFactors = FALSE)
}

#' @title aggregate_objective_report
#' @description Aggregate objective function components from a Casal2
#'   \code{objective_function} report object (single or multi-run).
#' @author Casal2 Development Team
#' @param objective_report An \code{objective_function} report element extracted
#'   from a \code{casal2MPD} object.
#' @return For single-run input: a data frame with columns \code{component} and
#'   \code{negative_loglik}.  For multi-run input: the first column is
#'   \code{component} followed by one numeric column per run.
#' @rdname aggregate_objective_report
#' @export aggregate_objective_report
aggregate_objective_report <- function(objective_report) {
  multi_parameter_input <- FALSE
  if (is.null(objective_report$type)) {
    if (is.null(objective_report[[1L]]$type)) {
      stop("Unknown format for objective_report")
    }
    if (objective_report[[1L]]$type != "objective_function") {
      stop("objective_report must be of type 'objective_function'")
    }
    multi_parameter_input <- TRUE
  } else if (objective_report$type != "objective_function") {
    stop("objective_report must be of type 'objective_function'")
  }
  if (!multi_parameter_input) {
    return(aggregate_single_objective_report(objective_report$values))
  }
  ## Pre-allocate; build column list then bind once (avoids O(n^2) cbind copies).
  par_sets <- vector("list", length(objective_report))
  for (i in seq_along(objective_report)) {
    par_sets[[i]] <- aggregate_single_objective_report(objective_report[[i]]$values)
  }
  full_df <- par_sets[[1L]]
  if (length(par_sets) > 1L) {
    extra_cols <- do.call(cbind, lapply(par_sets[-1L], `[[`, "negative_loglik"))
    full_df <- cbind(full_df, extra_cols)
  }
  full_df
}

#' @title get_objective_function
#' @description Extract and (optionally) aggregate objective function components
#'   from a Casal2 model output.  Handles single-run and multi-run MPD output,
#'   tabular (\code{casal2TAB}) output, and named lists of multiple MPD objects.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD}, \code{casal2TAB}, or named \code{list} of
#'   \code{casal2MPD} objects.
#' @param aggregate_obs Logical.  When \code{TRUE} (default) observation
#'   components are aggregated over years; when \code{FALSE} the raw per-year
#'   component values are returned.
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return A data frame with columns \code{component}, \code{negative_loglik},
#'   and \code{par_set} (integer index of the parameter set/minimisation; 1 for
#'   a single run, ordered factor for multi-run \code{-i} output).  When
#'   \code{aggregate_obs = TRUE} (default) observation components are summed
#'   across years before returning.  Returns \code{NULL} when no matching report
#'   is found.
#' @rdname get_objective_function
#' @export get_objective_function
"get_objective_function" <- function(model, ...) {
  UseMethod("get_objective_function", model)
}

#' @rdname get_objective_function
#' @method get_objective_function casal2MPD
#' @export
"get_objective_function.casal2MPD" <- function(model, aggregate_obs = TRUE, reformat_labels = TRUE, ...) {
  report_labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    if ("type" %in% names(this_report)) {
      ## single run
      if (tolower(this_report$type) != "objective_function") next
      if (aggregate_obs) {
        df <- aggregate_single_objective_report(this_report$values)
      } else {
        df <- data.frame(
          component        = names(this_report$values),
          negative_loglik  = as.numeric(this_report$values),
          stringsAsFactors = FALSE
        )
      }
      df$par_set <- 1L
      return(df)
    } else {
      ## multi-run (-i)
      if (tolower(this_report[[1L]]$type) != "objective_function") next
      n_runs <- length(this_report)
      iter_labs <- names(this_report)
      par_sets <- vector("list", n_runs)
      for (dash_i in seq_len(n_runs)) {
        if (aggregate_obs) {
          df_i <- aggregate_single_objective_report(this_report[[dash_i]]$values)
        } else {
          df_i <- data.frame(
            component        = names(this_report[[dash_i]]$values),
            negative_loglik  = as.numeric(this_report[[dash_i]]$values),
            stringsAsFactors = FALSE
          )
        }
        df_i$par_set <- iter_labs[dash_i]
        par_sets[[dash_i]] <- df_i
      }
      full_df <- .bind_rows_list(par_sets)
      full_df$par_set <- factor(full_df$par_set, ordered = TRUE)
      return(full_df)
    }
  }
  invisible(NULL)
}

#' @rdname get_objective_function
#' @method get_objective_function list
#' @export
"get_objective_function.list" <- function(model, aggregate_obs = TRUE, reformat_labels = TRUE, ...) {
  labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (!inherits(model[[i]], "casal2MPD")) {
      stop("This function only works on a named list with elements of class = 'casal2MPD'")
    }
    df <- get_objective_function.casal2MPD(model[[i]],
      aggregate_obs = aggregate_obs,
      reformat_labels = reformat_labels
    )
    if (!is.null(df)) {
      df$model_label <- labels[[i]]
      rows[[i]] <- df
    }
  }
  .bind_rows_list(rows)
}

#' @rdname get_objective_function
#' @method get_objective_function casal2TAB
#' @export
"get_objective_function.casal2TAB" <- function(model, aggregate_obs = TRUE, reformat_labels = TRUE, ...) {
  stop("get_objective_function for casal2TAB has not been implemented")
}
