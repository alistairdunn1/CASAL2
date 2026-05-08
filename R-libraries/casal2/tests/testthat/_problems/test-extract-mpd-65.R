# Extracted from test-extract-mpd.R:65

# setup ------------------------------------------------------------------------
library(testthat)
test_env <- simulate_test_env(package = "Casal2", path = "..")
attach(test_env, warn.conflicts = FALSE)

# prequel ----------------------------------------------------------------------
library(testthat)
library(Casal2)

# test -------------------------------------------------------------------------
mpd <- extract.mpd(file = testmodel("TwoSex/estimate_betadiff.log"), quiet = TRUE)
