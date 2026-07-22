#' @title get_priors
#' @description Extract prior density curves for all estimated parameters from
#'   a Casal2 MPD object. Internally calls \code{get_estimate_summary()} to
#'   obtain prior type, hyperparameters and bounds for each estimated parameter,
#'   then evaluates the normalised prior density over each parameter's range and
#'   returns the result as a long-format data frame suitable for plotting.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD} object or named \code{list} of
#'   \code{casal2MPD} objects produced by \code{extract.mpd()}.
#' @param n Integer; number of x-axis evaluation points per parameter
#'   (default \code{201}).
#' @param \dots Further arguments passed to \code{get_estimate_summary()}.
#' @return A long-format data frame with columns:
#'   \describe{
#'     \item{\code{parameter}}{Full Casal2 parameter label.}
#'     \item{\code{prior}}{Prior distribution type (e.g. \code{"lognormal"}).}
#'     \item{\code{hyperparameters}}{Hyperparameter string as stored in the
#'       \code{estimate_summary} report, e.g. \code{"mu = 1, cv = 0.2"}.}
#'     \item{\code{lower_bound}}{Lower bound of the parameter.}
#'     \item{\code{upper_bound}}{Upper bound of the parameter.}
#'     \item{\code{x}}{Parameter value (x-axis evaluation point).}
#'     \item{\code{density}}{Normalised prior density at \code{x} (peak = 1).}
#'     \item{\code{label}}{Casal2 report label.}
#'     \item{\code{par_set}}{Run label (\code{1L} for single-run output;
#'       ordered factor for multi-run \code{-i} output).}
#'   }
#'   Returns \code{NULL} when no \code{estimate_summary} report is found.
#' @rdname get_priors
#' @export get_priors
"get_priors" <- function(model, ...) {
  UseMethod("get_priors", model)
}

## Parse a hyperparameter string "mu = 1, cv = 0.2" to a named numeric vector.
.parse_hyper_str <- function(hyper_str) {
  if (is.na(hyper_str) || !nzchar(trimws(hyper_str))) {
    return(numeric(0L))
  }
  parts <- strsplit(hyper_str, ",\\s*")[[1L]]
  vals <- numeric(length(parts))
  nms <- character(length(parts))
  for (k in seq_along(parts)) {
    kv <- strsplit(parts[k], "\\s*=\\s*")[[1L]]
    nms[k] <- trimws(kv[1L])
    vals[k] <- suppressWarnings(as.numeric(kv[2L]))
  }
  setNames(vals, nms)
}

## Evaluate normalised prior density for one parameter over a grid of values.
## Returns NULL for unknown prior types so the caller can skip.
.prior_density_row <- function(type, hyper_vals, lower, upper, n) {
  log_priors <- c("uniform_log", "normal_log", "lognormal")
  lower_eval <- if (type %in% log_priors) max(lower, .Machine$double.eps) else lower
  x <- seq(lower_eval, upper, length.out = n)
  res <- switch(type,
    normal = {
      sigma <- hyper_vals[1L] * hyper_vals[2L]
      exp(-0.5 * ((x - hyper_vals[1L]) / sigma)^2)
    },
    lognormal = {
      sigma <- sqrt(log(hyper_vals[2L]^2 + 1))
      exp(-(log(x) + 0.5 * ((log(x / hyper_vals[1L]) / sigma + sigma / 2)^2)))
    },
    normal_by_stdev = {
      exp(-0.5 * ((x - hyper_vals[1L]) / hyper_vals[2L])^2)
    },
    uniform = rep(1.0, n),
    uniform_log = exp(-log(x)),
    normal_log = {
      exp(-(log(x) + 0.5 * ((log(x) - hyper_vals[1L]) / hyper_vals[2L])^2))
    },
    beta = {
      new_mu <- (hyper_vals[1L] - hyper_vals[3L]) / (hyper_vals[4L] - hyper_vals[3L])
      new_t <- (((hyper_vals[1L] - hyper_vals[3L]) * (hyper_vals[4L] - hyper_vals[1L])) /
        hyper_vals[2L]^2) - 1
      Bm <- new_t * new_mu
      Bn <- new_t * (1 - new_mu)
      r <- (1 - Bm) * log(x - hyper_vals[3L]) + (1 - Bn) * log(hyper_vals[4L] - x)
      exp(-r)
    },
    students_t = {
      mu <- hyper_vals[1L]
      sigma <- hyper_vals[2L]
      df <- hyper_vals[3L]
      x1 <- lgamma((df + 1) / 2) - lgamma(df / 2)
      x2 <- 0.5 * log(df * pi) + log(sigma)
      x3 <- log(1 + (1 / df) * ((x - mu) / sigma)^2)
      exp(x1 - x2 - x3 * ((df + 1) / 2))
    },
    NULL # unknown prior type
  )
  if (is.null(res)) {
    return(NULL)
  }
  peak <- max(res, na.rm = TRUE)
  if (is.finite(peak) && peak > 0) res <- res / peak
  data.frame(x = x, density = res, stringsAsFactors = FALSE)
}

#' @rdname get_priors
#' @method get_priors casal2MPD
#' @export
"get_priors.casal2MPD" <- function(model, n = 201L, ...) {
  summary_df <- get_estimate_summary(model, ...)
  if (is.null(summary_df) || !is.data.frame(summary_df) || nrow(summary_df) == 0L) {
    return(NULL)
  }

  rows <- vector("list", nrow(summary_df))
  for (i in seq_len(nrow(summary_df))) {
    row <- summary_df[i, ]
    hyper_vals <- .parse_hyper_str(row$hyperparameters)
    lower <- row$lower_bound
    upper <- row$upper_bound

    if (!is.finite(lower) || !is.finite(upper) || upper <= lower) next
    if (is.na(row$prior)) next

    density_df <- .prior_density_row(row$prior, hyper_vals, lower, upper, as.integer(n))
    if (is.null(density_df)) next

    rows[[i]] <- data.frame(
      parameter = row$parameter,
      prior = row$prior,
      hyperparameters = row$hyperparameters,
      lower_bound = lower,
      upper_bound = upper,
      density_df,
      label = row$label,
      par_set = row$par_set,
      stringsAsFactors = FALSE
    )
  }
  .bind_rows_list(rows)
}

#' @rdname get_priors
#' @method get_priors list
#' @export
"get_priors.list" <- function(model, n = 201L, ...) {
  .list_method(model, get_priors.casal2MPD, n = n, ...)
}
