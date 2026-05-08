# Extracted from test-extract-tabular.R:38

# setup ------------------------------------------------------------------------
library(testthat)
test_env <- simulate_test_env(package = "Casal2", path = "..")
attach(test_env, warn.conflicts = FALSE)

# prequel ----------------------------------------------------------------------
library(testthat)
library(Casal2)

# test -------------------------------------------------------------------------
expect_error(
    extract.tabular(file = testmodel("Simple/estimate_betadiff.log"), quiet = TRUE),
    regexp = "--tabular|extract\\.mpd|tabular"
  )
