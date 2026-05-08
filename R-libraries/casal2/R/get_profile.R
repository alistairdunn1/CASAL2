#' @title get_profile
#' @description Extract profile run data from a Casal2 MPD model output into a
#'   tidy data frame combining the profiled parameter values with the objective
#'   function components at each profile point.
#' @author Casal2 Development Team
#' @param model A \code{casal2MPD} object produced by \code{extract.mpd()} from
#'   a Casal2 profile run (\code{-p} flag).
#' @param aggregate_obs Logical. Aggregate observation log-likelihood components
#'   over years (default \code{TRUE}).
#' @param reformat_labels Logical. Reformat default Casal2 report labels
#'   (default \code{TRUE}).
#' @param \dots Further arguments (currently unused).
#' @return A data frame with columns \code{component}, \code{parameter_values},
#'   \code{negative_loglikelihood}, and \code{parameter}.
#' @rdname get_profile
#' @export get_profile
"get_profile" <- function(model, ...) {
  UseMethod("get_profile", model)
}

#' @rdname get_profile
#' @method get_profile casal2MPD
#' @export
"get_profile.casal2MPD" <- function(model, aggregate_obs = TRUE, reformat_labels = TRUE, ...) {
  report_labels  <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  parameter_df   <- NULL
  ## Locate the profile report
  for (i in seq_along(model)) {
    if (report_labels[i] == "header") {
      ## Verify this was indeed a profile run
      if (!grepl("-p", model[[i]]$call, fixed = TRUE))
        stop(paste0("Not a profile Casal2 output. Cannot find '-p' in '",
                    model[[i]]$call, "'."))
      next
    }
    this_report <- model[[i]]
    ## profile report sits one level in when generated from -i
    rpt <- if ("type" %in% names(this_report)) this_report else this_report[[1L]]
    if (!is.null(rpt$type) && rpt$type == "profile") {
      profile_lab  <- if (length(rpt$profile) > 0L) rpt$profile else "Profile"
      parameter_df <- data.frame(
        label            = profile_lab,
        parameter        = rpt$parameter,
        parameter_values = rpt$values,
        stringsAsFactors = FALSE
      )
    }
  }
  if (is.null(parameter_df))
    stop("No profile report found in model output.")
  ## Retrieve objective function at each profile step
  obj_df <- get_objective_function(model, aggregate_obs = aggregate_obs,
                                    reformat_labels = reformat_labels)
  colnames(obj_df) <- c("component", as.character(parameter_df$parameter_values))
  ## Wide-to-long without reshape2
  param_cols   <- setdiff(colnames(obj_df), "component")
  n_comp       <- nrow(obj_df)
  molten_profile <- data.frame(
    component              = rep(obj_df$component, times = length(param_cols)),
    parameter_values       = rep(param_cols, each = n_comp),
    negative_loglikelihood = unlist(obj_df[param_cols], use.names = FALSE),
    stringsAsFactors       = FALSE
  )
  molten_profile$parameter_values  <- as.numeric(molten_profile$parameter_values)
  molten_profile$parameter         <- unique(parameter_df$parameter)
  molten_profile
}
