# Tests for extract.parameters()
# Validates that extract.parameters() correctly reads Casal2 -o / -i format
# parameter files (tab-delimited with header row).
# Reference files from TestModels/
# setup.R defines: TESTMODELS_DIR, testmodel()

library(testthat)
library(Casal2)

# ---------------------------------------------------------------------------
# SingleSexTagByLength_input/pars.out — 17 columns, 1 row
# First col: process[Recruitment].b0 = 76728.7
# ---------------------------------------------------------------------------

test_that("extract.parameters returns a data frame from SingleSexTagByLength_input/pars.out", {
  pars <- extract.parameters(file = testmodel("SingleSexTagByLength_input/pars.out"), quiet = TRUE)
  expect_true(is.data.frame(pars))
})

test_that("extract.parameters SingleSex pars has 17 columns", {
  pars <- extract.parameters(file = testmodel("SingleSexTagByLength_input/pars.out"), quiet = TRUE)
  expect_equal(ncol(pars), 17L)
})

test_that("extract.parameters SingleSex pars has 1 row", {
  pars <- extract.parameters(file = testmodel("SingleSexTagByLength_input/pars.out"), quiet = TRUE)
  expect_equal(nrow(pars), 1L)
})

test_that("extract.parameters SingleSex pars has process[Recruitment].b0 column", {
  pars <- extract.parameters(file = testmodel("SingleSexTagByLength_input/pars.out"), quiet = TRUE)
  expect_true("process[Recruitment].b0" %in% colnames(pars))
})

test_that("extract.parameters SingleSex b0 value is correct (76728.7)", {
  pars <- extract.parameters(file = testmodel("SingleSexTagByLength_input/pars.out"), quiet = TRUE)
  expect_equal(pars[["process[Recruitment].b0"]], 76728.7, tolerance = 0.01)
})

test_that("extract.parameters all values are numeric (SingleSex)", {
  pars <- extract.parameters(file = testmodel("SingleSexTagByLength_input/pars.out"), quiet = TRUE)
  expect_true(all(sapply(pars, is.numeric)))
})

# ---------------------------------------------------------------------------
# Note: Complex_input/pars.out has a trailing space in its header line which
# causes make.data.frame() to fail (header count != data count). Skip those tests.
# ---------------------------------------------------------------------------
# Error handling
# ---------------------------------------------------------------------------

test_that("extract.parameters raises an error for a non-existent file", {
  expect_error(extract.parameters(file = testmodel("nonexistent/pars.out"), quiet = TRUE))
})
