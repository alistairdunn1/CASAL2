# Extracted from test-extract-mcmc.R:92

# setup ------------------------------------------------------------------------
library(testthat)
test_env <- simulate_test_env(package = "Casal2", path = "..")
attach(test_env, warn.conflicts = FALSE)

# prequel ----------------------------------------------------------------------
library(testthat)
library(Casal2)

# test -------------------------------------------------------------------------
mcmc <- extract.mcmc(
    samples.file    = testmodel("mcmc_start_mpd/samples.1"),
    objectives.file = testmodel("mcmc_start_mpd/objectives.1"),
    quiet = TRUE
  )
