# Extracted from test-extract-parameters.R:16

# setup ------------------------------------------------------------------------
library(testthat)
test_env <- simulate_test_env(package = "Casal2", path = "..")
attach(test_env, warn.conflicts = FALSE)

# prequel ----------------------------------------------------------------------
library(testthat)
library(Casal2)

# test -------------------------------------------------------------------------
pars <- extract.parameters(file = testmodel("SingleSexTagByLength_input/pars.out"), quiet = TRUE)
