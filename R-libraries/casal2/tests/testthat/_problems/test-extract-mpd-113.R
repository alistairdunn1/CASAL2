# Extracted from test-extract-mpd.R:113

# setup ------------------------------------------------------------------------
library(testthat)
test_env <- simulate_test_env(package = "Casal2", path = "..")
attach(test_env, warn.conflicts = FALSE)

# prequel ----------------------------------------------------------------------
library(testthat)
library(Casal2)

# test -------------------------------------------------------------------------
expect_error(
    extract.mpd(file = testmodel("Simple/projections.log"), quiet = TRUE),
    regexp = "tabular|extract\\.tabular"
  )
