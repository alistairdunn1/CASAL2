# Extracted from test-utility-functions.R:111

# setup ------------------------------------------------------------------------
library(testthat)
test_env <- simulate_test_env(package = "Casal2", path = "..")
attach(test_env, warn.conflicts = FALSE)

# prequel ----------------------------------------------------------------------
library(testthat)
library(Casal2)

# test -------------------------------------------------------------------------
tab <- extract.tabular(file = testmodel("Simple/projections.log"), quiet = TRUE)
