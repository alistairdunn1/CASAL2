#' generate.starting.pars Generates a parameter file that is formatted for -i input into Casal2.
#'
#' This function takes an  Casal2 MPD  object with an estimate_summary report, and returns a free parameters file. Where each parameter is drawn from the prior defined in the estimate_summary block.
#'
#' @author Alistair Dunn (derived from code by Craig Marsh)
#' @param MPD <casal2MPD> object that is generated from the extract() function.
#' @param path Optionally, the path to location to write the output file
#' @param n_sim the number of randomly generated sets of free parameters
#' @param start_free_filename = the filename of the file created
#' @param all_uniform = logical if TRUE draw from a uniform between bounds regardless of the prior distribution
#' @param shrink shrink bounds by a percent of the range of bounds to keep starting pars away from bounds (default = 0) and range (0-0.45)
#' @param quiet Optional, suppresses printing statements
#' @param write if false, do not write to the output file

#' @return a file (with default name 'start_free.dat') in the 'path' directory and returns these values if the output is assigned to an object
#' @export
#'
generate.starting.pars <- function(MPD = MPD, n_sim = 10, start_free_filename = "start_free.dat", path = "", all_uniform = FALSE, shrink = 0, quiet = T, write = TRUE) {
  rlognorm <- function(number, mu, cv) {
    logvar <- sqrt(log(cv^2 + 1))
    logmean <- log(mu) - (logvar^2) / 2
    return(exp(rnorm(number, logmean, logvar)))
  }

  estimate_blocks <- list()
  found_estimate <- FALSE
  for (i in 1:length(MPD)) {
    ndx <- ifelse(!is.null(MPD[[i]]$type) && MPD[[i]]$type == "estimate_summary", TRUE, FALSE)
    if (ndx) estimate_blocks <- c(estimate_blocks, MPD[[i]])
    if (ndx) found_estimate <- TRUE
  }
  if (!found_estimate) {
    stop("No estimate_summary reports were found in the MPD file supplied")
  }

  param_values <- c()
  N <- length(estimate_blocks) - 1 # ignore the last one as that's the extract report summary

  if (!quiet) {
    message(paste("A total of", N, "parameters were found"))
  }

  for (i in 1:N) {
    this_estimate <- estimate_blocks[[i]]
    values <- NULL
    if (this_estimate$sub_type == "uniform" || all_uniform) {
      values <- runif(n = n_sim, min = as.numeric(this_estimate$lower_bound), max = as.numeric(this_estimate$upper_bound))
    } else if (this_estimate$sub_type == "uniform_log") {
      values <- exp(runif(n = n_sim, min = log(as.numeric(this_estimate$lower_bound)), max = log(as.numeric(this_estimate$upper_bound))))
    } else if (this_estimate$sub_type == "normal") {
      std_dev <- as.numeric(this_estimate$hyperparameter_values[1]) * as.numeric(this_estimate$hyperparameter_values[2])
      values <- rnorm(n = n_sim, mean = as.numeric(this_estimate$this_estimate$hyperparameter_values[1]), sd = std_dev)
    } else if (this_estimate$sub_type == "lognormal") {
      values <- rlognorm(n = n_sim, mu = as.numeric(this_estimate$hyperparameter_values[1]), cv = as.numeric(this_estimate$hyperparameter_values[2]))
    } else if (this_estimate$sub_type == "normal_by_stdev") {
      values <- rnorm(n = n_sim, mean = as.numeric(this_estimate$hyperparameter_values[1]), sd = as.numeric(this_estimate$hyperparameter_values[2]))
    } else if (this_estimate$sub_type == "normal_log") {
      values <- exp(rnorm(n = n_sim, mean = as.numeric(this_estimate$hyperparameter_values[1]), sd = as.numeric(this_estimate$hyperparameter_values[2])))
    }
    ## set to bounds if generated past them
    if (shrink < 0 || shrink > 0.45) {
      stop("shrink must be a value between 0 and 0.45")
    }
    range <- as.numeric(this_estimate$upper_bound) - as.numeric(this_estimate$lower_bound)
    diff <- range * shrink

    values[values < as.numeric(this_estimate$lower_bound)] <- as.numeric(this_estimate$lower_bound) + diff
    values[values > as.numeric(this_estimate$upper_bound)] <- as.numeric(this_estimate$upper_bound) - diff
    param_values <- cbind(param_values, values)
  }
  param_labels <- names(estimate_blocks)[-length(names(estimate_blocks))]
  colnames(param_values) <- param_labels
  if (write) {
    filename <- make.filename(path = path, file = start_free_filename)
    message(paste0("Writing start_free_filename to '", start_free_filename, "' in path='", path, "'"))
    cat(param_labels, file = filename, sep = " ", fill = F, labels = NULL, append = F)
    cat("\n", file = filename, sep = " ", fill = F, labels = NULL, append = T)
    for (i in 1:nrow(param_values)) {
      cat(param_values[i, ], file = filename, sep = " ", fill = F, labels = NULL, append = T)
      cat("\n", file = filename, sep = " ", fill = F, labels = NULL, append = T)
    }
  }
  invisible(param_values)
}
# MPD <- extract.mpd(make.filename("estimate.log", PATH))
# a <- generate.starting.pars(MPD = MPD, n_sim = 10, start_free_filename = "start_free.dat", path =PATH, all_uniform = FALSE, quiet = F)
