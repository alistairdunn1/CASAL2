# Tests for extract.tabular()
# Reference files from TestModels/ (produced with -t / -I flags).
# setup.R defines: TESTMODELS_DIR, testmodel()

library(testthat)
library(Casal2)

# ---------------------------------------------------------------------------
# Simple/projections.log — produced with -t flag (tabular)
# ---------------------------------------------------------------------------
test_that("extract.tabular returns a casal2TAB object from Simple/projections.log", {
  tab <- extract.tabular(file = testmodel("Simple/projections.log"), quiet = TRUE)
  expect_s3_class(tab, "casal2TAB")
  expect_type(tab, "list")
})

test_that("extract.tabular result is a non-empty named list", {
  tab <- extract.tabular(file = testmodel("Simple/projections.log"), quiet = TRUE)
  expect_true(length(tab) > 0)
  expect_true(!is.null(names(tab)))
  expect_true(all(nchar(names(tab)) > 0))
})

test_that("extract.tabular report elements contain data frames or lists", {
  tab <- extract.tabular(file = testmodel("Simple/projections.log"), quiet = TRUE)
  first <- tab[[names(tab)[1]]]
  expect_true(is.data.frame(first) || is.list(first),
              label = "First tabular report element should be a data frame or list")
})

# ---------------------------------------------------------------------------
# Error / edge-case handling
# ---------------------------------------------------------------------------
test_that("extract.tabular raises an error for a non-tabular file (estimate_betadiff.log)", {
  expect_error(
    extract.tabular(file = testmodel("Simple/estimate_betadiff.log"), quiet = TRUE),
    regexp = "--tabular|extract\\.mpd|tabular"
  )
})

test_that("extract.tabular raises an error for a file that does not exist", {
  expect_error(extract.tabular(file = testmodel("nonexistent/tabular.log"), quiet = TRUE))
})
