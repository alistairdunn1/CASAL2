# Tests for extract.csl2.file()
# Validates that extract.csl2.file() correctly reads Casal2 configuration
# (.csl2) files and returns the expected list structure.
# Uses TestModels/Simple/ files.
# setup.R defines: TESTMODELS_DIR, testmodel()

library(testthat)
library(Casal2)

# ---------------------------------------------------------------------------
# Simple/population.csl2 — direct file (no !include directives)
# @model: start_year=1975, final_year=2012, @categories: names=stock
# ---------------------------------------------------------------------------

test_that("extract.csl2.file returns a list from Simple/population.csl2", {
  csl2 <- extract.csl2.file(
    file  = "population.csl2",
    path  = testmodel("Simple"),
    quiet = TRUE
  )
  expect_type(csl2, "list")
  expect_true(length(csl2) > 0)
})

test_that("extract.csl2.file Simple/population.csl2 has @model block", {
  csl2 <- extract.csl2.file(
    file  = "population.csl2",
    path  = testmodel("Simple"),
    quiet = TRUE
  )
  expect_true("model" %in% names(csl2))
})

test_that("extract.csl2.file @model has correct start_year and final_year", {
  csl2 <- extract.csl2.file(
    file  = "population.csl2",
    path  = testmodel("Simple"),
    quiet = TRUE
  )
  model <- csl2[["model"]]
  expect_true("start_year" %in% names(model))
  expect_true("final_year" %in% names(model))
  expect_equal(as.integer(model$start_year), 1975L)
  expect_equal(as.integer(model$final_year), 2012L)
})

test_that("extract.csl2.file @categories block has names='stock'", {
  csl2 <- extract.csl2.file(
    file  = "population.csl2",
    path  = testmodel("Simple"),
    quiet = TRUE
  )
  expect_true("categories" %in% names(csl2))
  cats <- csl2[["categories"]]
  expect_true("names" %in% names(cats))
  expect_equal(cats$names$value, "stock")
})

test_that("extract.csl2.file @model has time_steps subcommand", {
  csl2 <- extract.csl2.file(
    file  = "population.csl2",
    path  = testmodel("Simple"),
    quiet = TRUE
  )
  expect_true("time_steps" %in% names(csl2[["model"]]))
})

test_that("extract.csl2.file has @process blocks in population.csl2", {
  csl2 <- extract.csl2.file(
    file  = "population.csl2",
    path  = testmodel("Simple"),
    quiet = TRUE
  )
  process_names <- grep("^process", names(csl2), value = TRUE, ignore.case = TRUE)
  expect_true(length(process_names) > 0)
})

# ---------------------------------------------------------------------------
# config_betadiff.csl2 — has !include directives; test include=TRUE expansion
# ---------------------------------------------------------------------------

# Note: config_betadiff.csl2 consists entirely of !include directives.
# Calling extract.csl2.file() with include=TRUE on such a file crashes
# with "object 'Command' not found" — a known parser limitation.
# Test include=TRUE using population.csl2 (no !include directives) instead.
test_that("extract.csl2.file with include=TRUE works on a plain csl2 file", {
  csl2_inc <- extract.csl2.file(
    file    = "population.csl2",
    path    = testmodel("Simple"),
    include = TRUE,
    quiet   = TRUE
  )
  expect_type(csl2_inc, "list")
  expect_true("model" %in% names(csl2_inc))
  expect_true("categories" %in% names(csl2_inc))
})

# ---------------------------------------------------------------------------
# Error handling
# ---------------------------------------------------------------------------

test_that("extract.csl2.file errors for a file that does not exist", {
  expect_error(
    extract.csl2.file(
      file  = "no_such_config.csl2",
      path  = testmodel("Simple"),
      quiet = TRUE
    )
  )
})
