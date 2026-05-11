# Tests for extract.mcmc()
# Reference files: TestModels/mcmc_start_mpd/samples.1 and objectives.1
# (HAK4 model, 46 parameters, produced with casal2 -M mpd.log)
#
# The workspace extract.mcmc() (loaded via devtools::test()) uses cbind() not
# dplyr::bind_cols(), so no dplyr dependency is needed here.
# Return structure:
#   default              -> single data.frame, class casal2MCMC
#   return_covariance=TRUE -> list(Data=<df>, Covariance=<matrix>)

library(testthat)
library(Casal2)

# ---------------------------------------------------------------------------
# Basic smoke test
# mcmc_start_mpd: HAK4 model, 46 parameters, run with casal2 -M mpd.log
# ---------------------------------------------------------------------------

test_that("extract.mcmc returns a casal2MCMC data frame from mcmc_start_mpd", {
  mcmc <- extract.mcmc(
    samples.file    = testmodel("mcmc_start_mpd/samples.1"),
    objectives.file = testmodel("mcmc_start_mpd/objectives.1"),
    quiet = TRUE
  )
  expect_s3_class(mcmc, "casal2MCMC")
  expect_true(is.data.frame(mcmc))
})

# ---------------------------------------------------------------------------
# Objective columns
# ---------------------------------------------------------------------------

test_that("extract.mcmc merged data frame has objective columns", {
  mcmc <- extract.mcmc(
    samples.file    = testmodel("mcmc_start_mpd/samples.1"),
    objectives.file = testmodel("mcmc_start_mpd/objectives.1"),
    quiet = TRUE
  )
  expect_true("objective_score" %in% colnames(mcmc),
              label = "objective_score column should be present in merged data frame")
  expect_true("likelihood" %in% colnames(mcmc),
              label = "likelihood column should be present")
  expect_true(is.numeric(mcmc[["objective_score"]]),
              label = "objective_score should be numeric")
})

# ---------------------------------------------------------------------------
# Parameter columns
# ---------------------------------------------------------------------------

test_that("extract.mcmc merged data frame has parameter columns", {
  mcmc <- extract.mcmc(
    samples.file    = testmodel("mcmc_start_mpd/samples.1"),
    objectives.file = testmodel("mcmc_start_mpd/objectives.1"),
    quiet = TRUE
  )
  expect_true("process[Recruitment].b0" %in% colnames(mcmc),
              label = "b0 parameter column should be present")
  expect_true("catchability[chatTANq].q" %in% colnames(mcmc),
              label = "chatTANq catchability column should be present")
  expect_true(any(grepl("chatTANSel", colnames(mcmc))),
              label = "chatTANSel selectivity parameters should be present")
  expect_true(any(grepl("recruitment_multipliers", colnames(mcmc))),
              label = "recruitment_multipliers columns should be present")
})

# ---------------------------------------------------------------------------
# Dimensions
# ---------------------------------------------------------------------------

test_that("extract.mcmc data frame has plausible dimensions", {
  mcmc <- extract.mcmc(
    samples.file    = testmodel("mcmc_start_mpd/samples.1"),
    objectives.file = testmodel("mcmc_start_mpd/objectives.1"),
    quiet = TRUE
  )
  # 46 parameter columns + at least 9 objective columns
  expect_true(ncol(mcmc) >= 55L,
              label = paste("Expected >=55 total columns, got", ncol(mcmc)))
  expect_true(nrow(mcmc) > 0, label = "MCMC data frame should have at least one row")
})

# ---------------------------------------------------------------------------
# b0 values
# ---------------------------------------------------------------------------

test_that("extract.mcmc b0 values are positive and in a plausible range", {
  mcmc <- extract.mcmc(
    samples.file    = testmodel("mcmc_start_mpd/samples.1"),
    objectives.file = testmodel("mcmc_start_mpd/objectives.1"),
    quiet = TRUE
  )
  b0 <- mcmc[["process[Recruitment].b0"]]
  expect_true(all(b0 > 0), label = "All b0 values should be positive")
  # HAK4 b0 starts around 50301; expect in plausible range
  expect_true(all(b0 > 1e4 & b0 < 2e5),
              label = paste("b0 values should be in (10000, 200000); got range",
                            paste(round(range(b0)), collapse = "-")))
})

# ---------------------------------------------------------------------------
# Covariance matrix (return_covariance=TRUE)
# ---------------------------------------------------------------------------

test_that("extract.mcmc returns list(Data, Covariance) when return_covariance=TRUE", {
  result <- extract.mcmc(
    samples.file      = testmodel("mcmc_start_mpd/samples.1"),
    objectives.file   = testmodel("mcmc_start_mpd/objectives.1"),
    return_covariance = TRUE,
    quiet = TRUE
  )
  expect_type(result, "list")
  expect_true("Data" %in% names(result),
              label = "Result should have a 'Data' element when return_covariance=TRUE")
  expect_true("Covariance" %in% names(result),
              label = "Result should have a 'Covariance' element")
  cov_mat <- result$Covariance
  expect_true(is.matrix(cov_mat))
  expect_equal(nrow(cov_mat), ncol(cov_mat),
               label = "Covariance matrix should be square")
  # 46 parameters -> 46x46 covariance matrix
  expect_equal(nrow(cov_mat), 46L)
})

# ---------------------------------------------------------------------------
# Error handling
# ---------------------------------------------------------------------------

test_that("extract.mcmc raises an error when samples file does not exist", {
  expect_error(
    extract.mcmc(
      samples.file    = testmodel("nonexistent/samples.1"),
      objectives.file = testmodel("mcmc_start_mpd/objectives.1"),
      quiet = TRUE
    )
  )
})

test_that("extract.mcmc raises an error when objectives file does not exist", {
  expect_error(
    extract.mcmc(
      samples.file    = testmodel("mcmc_start_mpd/samples.1"),
      objectives.file = testmodel("nonexistent/objectives.1"),
      quiet = TRUE
    )
  )
})

test_that("extract.mcmc raises an error if samples file has wrong header", {
  expect_error(
    extract.mcmc(
      samples.file    = testmodel("Simple/estimate_betadiff.log"),
      objectives.file = testmodel("mcmc_start_mpd/objectives.1"),
      quiet = TRUE
    )
  )
})
