#' @title summarise_config
#' @description Read a Casal2 configuration file (and all files it
#'   \code{!include}s) and return a structured summary of the model in a list of
#'   data frames.  Uses \code{\link{extract.csl2.file}} with
#'   \code{include = TRUE} so that the entire model configuration is parsed in
#'   a single pass without any bespoke file-reading code.
#' @author Casal2 Development Team (ported from r4Casal2 by Craig Marsh)
#' @param config_dir Path to the directory containing the configuration files.
#' @param config_file Name of the top-level configuration file (default
#'   \code{"config.csl2"}).
#' @param quiet Logical.  When \code{TRUE} (default) suppresses progress
#'   messages.
#' @param fileEncoding Optional character string passed to
#'   \code{\link{extract.csl2.file}} for non-default file encodings.
#' @return A named list with elements:
#'   \describe{
#'     \item{\code{category_df}}{Data frame of category / growth / length-weight
#'       relationships (compact form).}
#'     \item{\code{full_category_df}}{Same as \code{category_df} but with block
#'       type annotations in parentheses.}
#'     \item{\code{time_step_df}}{Data frame of time steps with process types
#'       and assumed growth proportions.}
#'     \item{\code{time_step_df_just_lab}}{Same as \code{time_step_df} but using
#'       bare process labels.}
#'     \item{\code{obs_year_df}}{Long-form data frame (\code{year},
#'       \code{observation}, \code{type}, \code{active}) indicating which years
#'       each observation is active.}
#'     \item{\code{estimate_df}}{Data frame of \code{@estimate} blocks with
#'       columns \code{label}, \code{same}, \code{prior}, \code{lower_bound},
#'       \code{upper_bound}.}
#'     \item{\code{catch_df}}{Long-form data frame of catches by fishery and
#'       year (columns \code{year}, \code{process}, \code{fishery},
#'       \code{catch}).  \code{NULL} if catches are supplied via
#'       \code{!include} inside the mortality block.}
#'     \item{\code{method_df}}{Data frame of fishery method definitions.}
#'     \item{\code{M_by_category}}{Data frame of natural mortality values by
#'       category.}
#'     \item{\code{model_years}}{Integer vector of all model years.}
#'     \item{\code{model_ages}}{Integer vector of model ages (age-based models
#'       only; \code{NULL} for length-based models).}
#'     \item{\code{model_length_bins}}{Numeric vector of model length bins
#'       (\code{NULL} for age-based models).}
#'     \item{\code{model_block}}{The parsed \code{@model} block list.}
#'   }
#' @examples
#' \dontrun{
#' summary <- summarise_config("path/to/model", config_file = "config.csl2")
#' summary$obs_year_df
#' }
#' @rdname summarise_config
#' @export summarise_config
summarise_config <- function(config_dir = "",
                             config_file = "config.csl2",
                             quiet = TRUE,
                             fileEncoding = "") {
  ## --- validate file exists ---------------------------------------------------
  full_path <- file.path(config_dir, config_file)
  if (!file.exists(full_path)) {
    stop(paste0("Could not find '", config_file, "' at '", config_dir, "'"))
  }

  ## --- parse entire model config in one call (handles all !includes) ----------
  all_blocks <- tryCatch(
    extract.csl2.file(
      file = config_file,
      path = config_dir,
      fileEncoding = fileEncoding,
      quiet = quiet,
      include = TRUE
    ),
    error = function(e) stop(paste0("Failed to parse '", config_file, "': ", e$message)),
    warning = function(w) stop(paste0("Warning while parsing '", config_file, "': ", w$message))
  )

  blocks_vec <- get_block(names(all_blocks))
  labels_vec <- get_label(names(all_blocks))

  ## --- containers -------------------------------------------------------------
  model_block <- list()
  observation_blocks <- list()
  derived_quantity_blocks <- list()
  process_blocks <- list()
  estimate_blocks <- list()
  time_steps_list <- list()
  age_length_list <- list()
  length_weight_list <- list()
  growth_list <- list()

  category_labels <- NULL
  observation_labels <- NULL
  category_age_lengths <- NULL
  category_growth_increments <- NULL
  category_format <- NULL
  model_years <- NULL
  model_length_bins <- NULL
  ages <- NULL
  time_steps <- NULL
  length_based_model <- FALSE

  ## --- dispatch each block ----------------------------------------------------
  for (j in seq_along(all_blocks)) {
    blk <- tolower(blocks_vec[j])
    lbl <- labels_vec[j]
    b <- all_blocks[[j]]

    if (blk == "model") {
      length_based_model <- identical(tolower(b$type$value), "length")
      model_block[["model"]] <- b
      model_years <- as.numeric(b$start_year$value):as.numeric(b$final_year$value)
      if (!is.null(b$min_age)) {
        ages <- as.numeric(b$min_age$value):as.numeric(b$max_age$value)
      }
      time_steps <- b$time_steps$value
      if (!is.null(b$length_bins)) {
        for (k in seq_along(b$length_bins$value)) {
          model_length_bins <- c(
            model_length_bins,
            expand_shorthand_syntax(b$length_bins$value[k])
          )
        }
      }
    } else if (blk == "time_step") {
      time_steps_list[[lbl]] <- b$processes$value
    } else if (blk == "categories") {
      for (k in seq_along(b$names$value)) {
        category_labels <- c(
          category_labels,
          expand_category_block(b$names$value[k])
        )
      }
      if (!is.null(b$age_lengths)) {
        for (k in seq_along(b$age_lengths$value)) {
          category_age_lengths <- c(
            category_age_lengths,
            expand_shorthand_syntax(b$age_lengths$value[k])
          )
        }
      }
      if (!is.null(b$growth_increment)) {
        for (k in seq_along(b$growth_increment$value)) {
          category_growth_increments <- c(
            category_growth_increments,
            expand_shorthand_syntax(b$growth_increment$value[k])
          )
        }
      }
      if (!is.null(b$format)) {
        category_format <- b$format$value
      }
    } else if (blk == "age_length") {
      age_length_list[[lbl]] <- b
    } else if (blk == "growth_increment") {
      growth_list[[lbl]] <- b
    } else if (blk == "process") {
      process_blocks[[lbl]] <- b
    } else if (blk == "length_weight") {
      length_weight_list[[lbl]] <- b
    } else if (blk == "estimate") {
      estimate_blocks[[lbl]] <- b
    } else if (blk == "observation") {
      observation_blocks[[lbl]] <- b
      observation_labels <- c(observation_labels, lbl)
    } else if (blk == "derived_quantity") {
      derived_quantity_blocks[[lbl]] <- b
    }
  }

  ## --- categories summary -----------------------------------------------------
  category_df <- NULL
  full_category_df <- NULL
  age_length_time_step_growth <- NULL

  for (i in seq_along(category_labels)) {
    if (!length_based_model) {
      this_al <- age_length_list[[category_age_lengths[i]]]
      this_lw <- length_weight_list[[this_al$length_weight$value]]
      distribution <- if (!is.null(this_al$distribution)) this_al$distribution else "normal"

      category_df <- rbind(category_df, data.frame(
        Category = category_labels[i],
        AgeLength = category_age_lengths[i],
        LengthWeight = this_al$length_weight$value,
        Distribution = distribution,
        stringsAsFactors = FALSE
      ))
      full_category_df <- rbind(full_category_df, data.frame(
        Category = category_labels[i],
        AgeLength = paste0(category_age_lengths[i], " (", this_al$type$value, ")"),
        LengthWeight = paste0(this_al$length_weight$value, " (", this_lw$type$value, ")"),
        Distribution = distribution,
        stringsAsFactors = FALSE
      ))

      tsp <- if (is.null(this_al$time_step_proportions$value)) 0.0 else this_al$time_step_proportions$value
      al_lbl <- category_age_lengths[i]
      if (is.null(age_length_time_step_growth) ||
        !al_lbl %in% age_length_time_step_growth$AgeLength) {
        age_length_time_step_growth <- rbind(
          age_length_time_step_growth,
          data.frame(
            AgeLength = al_lbl, time_step_proportions = tsp,
            stringsAsFactors = FALSE
          )
        )
      }
    } else {
      this_gi <- growth_list[[category_growth_increments[i]]]
      this_lw <- length_weight_list[[this_gi$length_weight$value]]
      distribution <- if (!is.null(this_gi$distribution)) this_gi$distribution else "normal"

      category_df <- rbind(category_df, data.frame(
        Category = category_labels[i],
        GrowthIncrement = category_growth_increments[i],
        LengthWeight = this_gi$length_weight$value,
        Distribution = distribution,
        stringsAsFactors = FALSE
      ))
      full_category_df <- rbind(full_category_df, data.frame(
        Category = category_labels[i],
        GrowthIncrement = paste0(category_growth_increments[i], " (", this_gi$type$value, ")"),
        LengthWeight = paste0(this_gi$length_weight$value, " (", this_lw$type$value, ")"),
        Distribution = distribution,
        stringsAsFactors = FALSE
      ))
      gi_lbl <- category_growth_increments[i]
      if (is.null(age_length_time_step_growth) ||
        !gi_lbl %in% age_length_time_step_growth$GrowthIncrement) {
        age_length_time_step_growth <- rbind(
          age_length_time_step_growth,
          data.frame(GrowthIncrement = gi_lbl, stringsAsFactors = FALSE)
        )
      }
    }
  }

  ## --- observations summary ---------------------------------------------------
  obs_year_df <- NULL
  if (!is.null(observation_labels)) {
    for (i in seq_along(observation_labels)) {
      this_obs <- observation_blocks[[observation_labels[i]]]
      years <- NULL
      for (y in seq_along(this_obs$years$value)) {
        years <- c(years, expand_shorthand_syntax(this_obs$years$value[y]))
      }
      active_ndx <- model_years %in% as.numeric(years)
      obs_year_df <- rbind(obs_year_df, data.frame(
        year = model_years,
        observation = observation_labels[i],
        type = this_obs$type$value,
        active = ifelse(active_ndx, 1L, NA_integer_),
        stringsAsFactors = FALSE
      ))
    }
  }

  ## --- time steps summary -----------------------------------------------------
  time_step_df <- NULL
  time_step_df_just_lab <- NULL

  for (i in seq_along(time_steps)) {
    processes <- time_steps_list[[time_steps[i]]]
    process_type <- character(length(processes))
    df_entry <- character(length(processes))
    for (j in seq_along(processes)) {
      process_type[j] <- process_blocks[[processes[j]]]$type$value
      df_entry[j] <- paste0(processes[j], " (", process_type[j], ")")
    }
    ## insert derived quantities calculated within this time step
    for (j in seq_along(derived_quantity_blocks)) {
      if (time_steps[i] == derived_quantity_blocks[[j]]$time_step$value) {
        prop_mortality <- if (!is.null(derived_quantity_blocks[[j]]$time_step_proportion$value)) {
          as.numeric(derived_quantity_blocks[[j]]$time_step_proportion$value)
        } else {
          0.5
        }
        process_type <- c(process_type, "derived-quantity")
        df_entry <- c(df_entry, paste0(
          names(derived_quantity_blocks)[j],
          " (derived-quantity ", round(prop_mortality, 2L), ")"
        ))
      }
    }
    time_step_df <- rbind(
      time_step_df,
      data.frame(
        time_step = time_steps[i],
        processes = paste(df_entry, collapse = ", "),
        stringsAsFactors = FALSE
      )
    )
    time_step_df_just_lab <- rbind(
      time_step_df_just_lab,
      data.frame(
        time_step = time_steps[i],
        processes = paste(processes, collapse = ", "),
        stringsAsFactors = FALSE
      )
    )
  }

  if (!length_based_model && !is.null(age_length_time_step_growth)) {
    al_labs <- unique(age_length_time_step_growth$AgeLength)
    for (i in seq_along(al_labs)) {
      tsp <- age_length_time_step_growth$time_step_proportions[
        age_length_time_step_growth$AgeLength == al_labs[i]
      ]
      time_step_df <- cbind(time_step_df, tsp)
      time_step_df_just_lab <- cbind(time_step_df_just_lab, tsp)
    }
    al_col_names <- paste0(al_labs, " (assumed growth)")
    colnames(time_step_df) <- c("Time-step", "Processes (type)", al_col_names)
    colnames(time_step_df_just_lab) <- c("Time-step", "Processes", al_col_names)
  }

  ## --- mortality: M and catches -----------------------------------------------
  M_by_category <- NULL
  M_time_steps <- NULL
  catch_df <- NULL
  method_df <- NULL

  for (i in seq_along(process_blocks)) {
    this_process <- process_blocks[[i]]
    proc_type <- tolower(this_process$type$value)

    if (proc_type == "mortality_instantaneous") {
      m <- expand_shorthand_syntax(this_process$m$value)
      categories <- NULL
      for (j in seq_along(this_process$categories$value)) {
        categories <- c(
          categories,
          expand_category_shorthand(this_process$categories$value[j],
            category_labels,
            category_format = category_format
          )
        )
      }
      selectivity <- NULL
      for (j in seq_along(this_process$relative_m_by_age$value)) {
        selectivity <- c(
          selectivity,
          expand_shorthand_syntax(this_process$relative_m_by_age$value[j])
        )
      }

      M_by_category <- rbind(M_by_category, data.frame(
        process = names(process_blocks)[i],
        category = categories,
        M = m,
        relative_M = selectivity,
        stringsAsFactors = FALSE
      ))

      time_prop <- if (is.null(this_process$time_step_proportions$value)) {
        rep(1, nrow(time_step_df))
      } else {
        this_process$time_step_proportions$value
      }

      M_time_steps <- rbind(M_time_steps, data.frame(
        process = names(process_blocks)[i],
        time_step_proportions = time_prop,
        stringsAsFactors = FALSE
      ))

      ## catches -- only available when the table is inline (not !include'd)
      if (!is.null(this_process$Table$catches)) {
        this_catch <- Reduce(cbind, this_process$Table$catches)
        class(this_catch) <- "numeric"
        colnames(this_catch) <- names(this_process$Table$catches)
        this_catch <- as.data.frame(this_catch, stringsAsFactors = FALSE)
        this_catch$process <- names(process_blocks)[i]

        ## base-R wide-to-long (replaces reshape2::melt)
        id_cols <- c("year", "process")
        val_cols <- setdiff(colnames(this_catch), id_cols)
        molten <- do.call(rbind, lapply(val_cols, function(v) {
          data.frame(
            year = this_catch$year,
            process = this_catch$process,
            fishery = v,
            catch = as.numeric(this_catch[[v]]),
            stringsAsFactors = FALSE
          )
        }))
        catch_df <- rbind(catch_df, molten)

        this_method <- Reduce(cbind, this_process$Table$method)
        colnames(this_method) <- names(this_process$Table$method)
        this_method <- as.data.frame(this_method, stringsAsFactors = FALSE)
        for (k in seq_len(nrow(this_method))) {
          this_method$category[k] <- paste(
            expand_category_shorthand(this_method$category[k],
              category_labels,
              category_format = category_format
            ),
            collapse = ","
          )
        }
        this_method$process <- names(process_blocks)[i]
        method_df <- rbind(method_df, this_method)
      }
    } else if (proc_type == "mortality_instantaneous_retained") {
      warning("summarise_config: 'mortality_instantaneous_retained' process is not yet summarised")
    }
  }

  ## --- estimates summary ------------------------------------------------------
  estimate_df <- NULL
  if (length(estimate_blocks) > 0L) {
    for (i in seq_along(estimate_blocks)) {
      this_est <- estimate_blocks[[i]]
      lower_bound <- this_est$lower_bound$value
      upper_bound <- this_est$upper_bound$value
      same <- this_est$same$value
      if (is.null(same)) {
        same <- "-"
      } else if (length(same) > 1L) {
        same <- paste(same, collapse = ", ")
      }
      if (length(lower_bound) > 1L) {
        lower_bound <- paste(lower_bound, collapse = " ")
      }
      if (length(upper_bound) > 1L) {
        upper_bound <- paste(upper_bound, collapse = " ")
      }

      estimate_df <- rbind(estimate_df, data.frame(
        label = names(estimate_blocks)[i],
        same = same,
        prior = this_est$type$value,
        lower_bound = lower_bound,
        upper_bound = upper_bound,
        stringsAsFactors = FALSE
      ))
    }
  }

  ## --- return -----------------------------------------------------------------
  list(
    category_df           = category_df,
    full_category_df      = full_category_df,
    time_step_df          = time_step_df,
    time_step_df_just_lab = time_step_df_just_lab,
    obs_year_df           = obs_year_df,
    estimate_df           = estimate_df,
    catch_df              = catch_df,
    method_df             = method_df,
    M_by_category         = M_by_category,
    model_years           = model_years,
    model_ages            = ages,
    model_length_bins     = model_length_bins,
    model_block           = model_block[["model"]]
  )
}
