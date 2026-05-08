#' @title get_composition_mean_bin
#' @description Calculate mean age or length (and associated statistics) from
#'   composition observations in a Casal2 model output.  For each observation
#'   and year the function computes the Francis (TA1.8) mean-bin statistics:
#'   observed mean (\code{Oy}), predicted mean (\code{Ey}), residual
#'   (\code{Ry}), standard error (\code{SEy}), standardised residual
#'   (\code{Std.res}), and adjusted 95\% confidence intervals
#'   (\code{ObsloAdj}, \code{ObshiAdj}).
#' @author Craig Marsh
#' @param model A \code{casal2MPD} or named \code{list} of \code{casal2MPD}
#'   objects produced by \code{extract.mpd()}.
#' @param ignore_plus_group Logical. Remove the maximum age/length bin before
#'   computing means (default \code{FALSE}).
#' @param by_category Logical. Return values by category (\code{TRUE}) or
#'   aggregated over categories (\code{FALSE}, default).
#' @param \dots Further arguments (currently unused).
#' @return A data frame with columns \code{year}, \code{observation_label},
#'   optionally \code{category}, \code{Ey}, \code{Oy}, \code{E_squared_y},
#'   \code{Nassumed}, \code{Ry}, \code{SEy}, \code{Std.res},
#'   \code{ObsloAdj}, \code{ObshiAdj}.
#' @rdname get_composition_mean_bin
#' @export get_composition_mean_bin
"get_composition_mean_bin" <- function(model, ...) {
  UseMethod("get_composition_mean_bin", model)
}

#' @rdname get_composition_mean_bin
#' @method get_composition_mean_bin casal2MPD
#' @export
"get_composition_mean_bin.casal2MPD" <- function(model, ignore_plus_group = FALSE, by_category = FALSE, ...) {
  comp_obs <- get_composition_observations(model)
  if (is.null(comp_obs)) {
    return(NULL)
  }
  obs <- unique(comp_obs$observation_label)
  mean_bin_df <- NULL
  for (i in seq_along(obs)) {
    this_obs <- comp_obs[comp_obs$observation_label == obs[i], ]
    is_age <- grepl("_age", unique(this_obs$observation_type))
    if (ignore_plus_group) {
      if (is_age) {
        this_obs <- this_obs[this_obs$age != max(this_obs$age), ]
      } else {
        this_obs <- this_obs[this_obs$length != max(this_obs$length), ]
      }
    }
    ## force proportions to sum = 1
    if (!by_category) {
      this_obs <- do.call(rbind, lapply(split(this_obs, list(this_obs$year, this_obs$observation_label)), function(d) {
        d$expected <- d$expected / sum(d$expected)
        d$observed <- d$observed / sum(d$observed)
        d
      }))
    } else {
      this_obs <- do.call(rbind, lapply(split(this_obs, list(this_obs$year, this_obs$observation_label, this_obs$category)), function(d) {
        d$expected <- d$expected / sum(d$expected)
        d$observed <- d$observed / sum(d$observed)
        d
      }))
    }
    if (is_age) {
      if (!by_category) {
        mean_stats <- do.call(rbind, lapply(split(this_obs, list(this_obs$year, this_obs$observation_label)), function(d) {
          data.frame(
            year              = unique(d$year),
            observation_label = unique(d$observation_label),
            Ey                = sum(d$age * d$expected),
            Oy                = sum(d$age * d$observed),
            E_squared_y       = sum(d$age^2 * d$expected),
            Nassumed          = mean(d$adjusted_error),
            stringsAsFactors  = FALSE
          )
        }))
      } else {
        mean_stats <- do.call(rbind, lapply(split(this_obs, list(this_obs$year, this_obs$observation_label, this_obs$category)), function(d) {
          data.frame(
            year              = unique(d$year),
            observation_label = unique(d$observation_label),
            category          = unique(d$category),
            Ey                = sum(d$age * d$expected),
            Oy                = sum(d$age * d$observed),
            E_squared_y       = sum(d$age^2 * d$expected),
            Nassumed          = mean(d$adjusted_error),
            stringsAsFactors  = FALSE
          )
        }))
      }
    } else {
      if (!by_category) {
        mean_stats <- do.call(rbind, lapply(split(this_obs, list(this_obs$year, this_obs$observation_label)), function(d) {
          data.frame(
            year              = unique(d$year),
            observation_label = unique(d$observation_label),
            Ey                = sum(d$length * d$expected),
            Oy                = sum(d$length * d$observed),
            E_squared_y       = sum(d$length^2 * d$expected),
            Nassumed          = mean(d$adjusted_error),
            stringsAsFactors  = FALSE
          )
        }))
      } else {
        mean_stats <- do.call(rbind, lapply(split(this_obs, list(this_obs$year, this_obs$observation_label, this_obs$category)), function(d) {
          data.frame(
            year              = unique(d$year),
            observation_label = unique(d$observation_label),
            category          = unique(d$category),
            Ey                = sum(d$length * d$expected),
            Oy                = sum(d$length * d$observed),
            E_squared_y       = sum(d$length^2 * d$expected),
            Nassumed          = mean(d$adjusted_error),
            stringsAsFactors  = FALSE
          )
        }))
      }
    }
    mean_stats$Ry <- mean_stats$Oy - mean_stats$Ey
    mean_stats$SEy <- sqrt((mean_stats$E_squared_y - mean_stats$Ey^2) / mean_stats$Nassumed)
    mean_bin_df <- rbind(mean_bin_df, mean_stats)
  }
  if (is.null(mean_bin_df)) {
    return(NULL)
  }
  mean_bin_df$"Std.res" <- (mean_bin_df$Oy - mean_bin_df$Ey) / mean_bin_df$SEy
  Nmult <- 1 / var(mean_bin_df$"Std.res", na.rm = TRUE)
  mean_bin_df$ObsloAdj <- mean_bin_df$Oy - 2 * mean_bin_df$SEy / sqrt(Nmult)
  mean_bin_df$ObshiAdj <- mean_bin_df$Oy + 2 * mean_bin_df$SEy / sqrt(Nmult)
  return(mean_bin_df)
}

#' @rdname get_composition_mean_bin
#' @method get_composition_mean_bin list
#' @export
"get_composition_mean_bin.list" <- function(model, ignore_plus_group = FALSE, by_category = FALSE, ...) {
  run_labs <- names(model)
  full_DF <- NULL
  for (i in seq_along(model)) {
    if (!inherits(model[[i]], "casal2MPD")) {
      stop("This function only works on a named list with elements of class = 'casal2MPD'")
    }
    this_df <- get_composition_mean_bin(model[[i]], ignore_plus_group = ignore_plus_group, by_category = by_category)
    if (!is.null(this_df)) {
      this_df$model_label <- run_labs[i]
      full_DF <- rbind(full_DF, this_df)
    }
  }
  return(full_DF)
}

#' @rdname get_composition_mean_bin
#' @method get_composition_mean_bin casal2TAB
#' @export
"get_composition_mean_bin.casal2TAB" <- function(model, ignore_plus_group = FALSE, by_category = FALSE, ...) {
  stop("get_composition_mean_bin for casal2TAB has not been implemented")
}
