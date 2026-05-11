#' @title summarise_estimated_parameters
#' @description Summarise estimated parameters from a Casal2 MPD
#'   \code{estimate_summary} report.  Returns a named list containing a summary
#'   data frame and a prior-evaluation data frame that can be used for plotting.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD} object produced by \code{extract.mpd()}.
#' @param ignore Optional regular expression string. Parameters whose labels
#'   match are excluded from the output.
#' @param n Integer. Number of points at which to evaluate each prior density
#'   curve (default \code{250}).
#' @param \dots Further arguments passed to methods.
#' @return A named list with elements:
#'   \describe{
#'     \item{\code{summary_df}}{Data frame of estimated parameters with columns
#'       \code{parameter}, \code{initial_value}, \code{prior}, \code{bounds},
#'       \code{phase}, \code{MPD}, \code{std_dev}, \code{mcmc_fixed}, and
#'       \code{Hyper parameters}.}
#'     \item{\code{plot_df}}{Data frame with columns \code{parameter_value},
#'       \code{likelihood}, and \code{parameter} for each parameter's prior
#'       evaluated over its bounds, suitable for passing to a plotting function.}
#'   }
#'   Returns a character string message when no \code{estimate_summary} report
#'   is found.
#' @rdname summarise_estimated_parameters
#' @export summarise_estimated_parameters
"summarise_estimated_parameters" <- function(model, ...) {
  UseMethod("summarise_estimated_parameters", model)
}

## Evaluate normalised prior density over a grid of parameter values.
.eval_prior_density <- function(param_value, type, hyper_param_values, lower_b, upper_b) {
  res <- switch(type,
    normal = {
      sigma <- hyper_param_values[1L] * hyper_param_values[2L]
      exp(-0.5 * ((param_value - hyper_param_values[1L]) / sigma)^2)
    },
    lognormal = {
      sigma <- sqrt(log(hyper_param_values[2L]^2 + 1))
      exp(-(log(param_value) + 0.5 * ((log(param_value / hyper_param_values[1L]) / sigma + sigma / 2)^2)))
    },
    normal_by_stdev = {
      exp(-0.5 * ((param_value - hyper_param_values[1L]) / hyper_param_values[2L])^2)
    },
    uniform = {
      rep(1.0, length(param_value))
    },
    uniform_log = {
      exp(-log(param_value))
    },
    normal_log = {
      exp(-(log(param_value) + 0.5 * ((log(param_value) - hyper_param_values[1L]) / hyper_param_values[2L])^2))
    },
    beta = {
      new_mu <- (hyper_param_values[1L] - hyper_param_values[3L]) /
                (hyper_param_values[4L] - hyper_param_values[3L])
      new_t  <- (((hyper_param_values[1L] - hyper_param_values[3L]) *
                   (hyper_param_values[4L] - hyper_param_values[1L])) /
                  (hyper_param_values[2L]^2)) - 1
      Bm <- new_t * new_mu
      Bn <- new_t * (1 - new_mu)
      r  <- (1 - Bm) * log(param_value - hyper_param_values[3L]) +
            (1 - Bn) * log(hyper_param_values[4L] - param_value)
      exp(-r)
    },
    students_t = {
      mu    <- hyper_param_values[1L]
      sigma <- hyper_param_values[2L]
      df    <- hyper_param_values[3L]
      x1    <- lgamma((df + 1) / 2) - lgamma(df / 2)
      x2    <- 0.5 * log(df * pi) + log(sigma)
      x3    <- log(1 + (1 / df) * ((param_value - mu) / sigma)^2)
      x4    <- (df + 1) / 2
      exp(x1 - x2 - x3 * x4)
    },
    NULL   # unknown type -> NULL so caller can skip
  )
  if (!is.null(res)) res / max(res, na.rm = TRUE) else NULL
}

#' @rdname summarise_estimated_parameters
#' @method summarise_estimated_parameters casal2MPD
#' @export
summarise_estimated_parameters.casal2MPD <- function(model, ignore = NULL,
                                                      n = 250L, ...) {
  report_labels <- reformat_default_labels(names(model))
  complete_df   <- NULL
  plot_rows     <- list()
  found_report  <- FALSE
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") next
    this_report <- model[[i]]
    ## Handle both single-run and multi-run format
    if (!"type" %in% names(this_report)) {
      if (this_report[[1L]]$type != "estimate_summary") next
      if (length(this_report) != 1L) {
        message("summarise_estimated_parameters: implemented for a single MPD object only")
        return(NULL)
      }
      this_report <- this_report[[1L]]
    }
    if (this_report$type != "estimate_summary") next
    if (found_report) {
      message("Multiple 'estimate_summary' reports found - only the first is used.")
      next
    }
    found_report <- TRUE
    ## Iterate over individual parameter entries
    for (p in seq_along(this_report)) {
      if (names(this_report)[p] == "type") next
      this_param <- this_report[[p]]
      ## Sanitise parameter label for downstream use
      param_label <- gsub("[\\{\\}\\[\\]]", "_", this_param$parameter, perl = TRUE)
      if (!is.null(ignore) && grepl(ignore, param_label)) next
      hyper_params       <- this_param$hyperparameters
      hyper_param_values <- as.numeric(this_param$hyperparameter_values)
      hyper_str          <- paste(paste0(hyper_params, " = ", hyper_param_values), collapse = ", ")
      temp_df <- data.frame(
        parameter       = this_param$parameter,
        initial_value   = as.numeric(this_param$initial_value),
        prior           = this_param$sub_type,
        bounds          = paste0(this_param$lower_bound, " - ", this_param$upper_bound),
        phase           = this_param$phase,
        MPD             = as.numeric(this_param$value),
        std_dev         = as.numeric(this_param$std_dev),
        mcmc_fixed      = if (length(this_param$mcmc_fixed) > 0L) this_param$mcmc_fixed else FALSE,
        stringsAsFactors = FALSE
      )
      temp_df[["Hyper parameters"]] <- hyper_str
      complete_df <- rbind(complete_df, temp_df)
      ## Evaluate prior density for plotting
      param_seq <- seq(from = as.numeric(this_param$lower_bound),
                       to   = as.numeric(this_param$upper_bound),
                       length.out = n)
      dens <- .eval_prior_density(param_seq, type = temp_df$prior,
                                   hyper_param_values = hyper_param_values,
                                   lower_b = as.numeric(this_param$lower_bound),
                                   upper_b = as.numeric(this_param$upper_bound))
      if (!is.null(dens)) {
        plot_rows[[length(plot_rows) + 1L]] <- data.frame(
          parameter_value  = param_seq,
          likelihood       = dens,
          parameter        = this_param$parameter,
          stringsAsFactors = FALSE
        )
      }
    }
  }
  if (is.null(complete_df))
    return("Could not find 'estimate_summary' report in Casal2 output")
  plot_df <- .bind_rows_list(plot_rows)
  list(plot_df = plot_df, summary_df = complete_df)
}
