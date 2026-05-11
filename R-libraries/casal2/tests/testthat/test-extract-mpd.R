# Tests for extract.mpd()
# All reference files come from TestModels/ (new-format {dataframe_with_row_labels} output).
# setup.R defines: TESTMODELS_DIR, testmodel()
library(testthat)
library(Casal2)

# ---------------------------------------------------------------------------
# Simple model: single-category 'stock', years 1975-2012
# ---------------------------------------------------------------------------

test_that("extract.mpd returns a casal2MPD object from Simple/estimate_betadiff.log", {
  mpd <- extract.mpd(file = testmodel("Simple/estimate_betadiff.log"), quiet = TRUE)
  expect_s3_class(mpd, "casal2MPD")
  expect_type(mpd, "list")
})

test_that("extract.mpd header is populated from Simple/estimate_betadiff.log", {
  mpd <- extract.mpd(file = testmodel("Simple/estimate_betadiff.log"), quiet = TRUE)
  expect_true("header" %in% names(mpd))
  hdr <- mpd$header
  expect_match(hdr$call, "-e", fixed = TRUE)
  expect_true(nchar(hdr$date) > 0)
  expect_true(nchar(hdr$version) > 0)
})

test_that("extract.mpd has partition_step1 report year-keyed by integers", {
  mpd <- extract.mpd(file = testmodel("Simple/estimate_betadiff.log"), quiet = TRUE)
  expect_true("partition_step1" %in% names(mpd))
  part <- mpd[["partition_step1"]]
  year_keys <- setdiff(names(part), "type")
  expect_true(length(year_keys) > 0)
  expect_true(all(grepl("^[0-9]+$", year_keys)))
  expect_true("1975" %in% year_keys)
  expect_true("2010" %in% year_keys)
})

test_that("extract.mpd partition_step1 values are data frames with numeric content", {
  mpd <- extract.mpd(file = testmodel("Simple/estimate_betadiff.log"), quiet = TRUE)
  vals <- mpd[["partition_step1"]][["1975"]]$values
  expect_true(is.data.frame(vals))
  expect_true(all(sapply(vals, is.numeric)))
  expect_equal(nrow(vals), 1L)   # single category 'stock'
  expect_equal(ncol(vals), 30L)  # ages 1-30
})

test_that("extract.mpd partition_step1 row name is 'stock'", {
  mpd <- extract.mpd(file = testmodel("Simple/estimate_betadiff.log"), quiet = TRUE)
  vals <- mpd[["partition_step1"]][["1975"]]$values
  expect_equal(rownames(vals), "stock")
})

test_that("extract.mpd has estimate_value summary report with a data frame of values", {
  mpd <- extract.mpd(file = testmodel("Simple/estimate_betadiff.log"), quiet = TRUE)
  expect_true("estimate_value" %in% names(mpd))
  summ <- mpd[["estimate_value"]]
  expect_false(is.null(summ$values))
  expect_true(is.data.frame(summ$values))
})

# ---------------------------------------------------------------------------
# TwoSex model: multi-category (male + female)
# ---------------------------------------------------------------------------

test_that("extract.mpd returns a casal2MPD object from TwoSex/estimate_betadiff.log", {
  mpd <- extract.mpd(file = testmodel("TwoSex/estimate_betadiff.log"), quiet = TRUE)
  expect_s3_class(mpd, "casal2MPD")
})

test_that("extract.mpd TwoSex partition has two category rows (male and female)", {
  mpd <- extract.mpd(file = testmodel("TwoSex/estimate_betadiff.log"), quiet = TRUE)
  part_names <- grep("^partition|^state", names(mpd), value = TRUE, ignore.case = TRUE)
  expect_true(length(part_names) > 0, label = "TwoSex should have a partition report")
  part <- mpd[[part_names[1]]]
  year_keys <- setdiff(names(part), "type")
  expect_true(length(year_keys) > 0)
  vals <- part[[year_keys[1]]]$values
  expect_true(is.data.frame(vals))
  expect_equal(nrow(vals), 2L, label = "TwoSex partition should have 2 rows (male + female)")
  expect_true(all(c("male", "female") %in% rownames(vals)))
})

# ---------------------------------------------------------------------------
# Complex_input run with -r -i pars.out
# ---------------------------------------------------------------------------

test_that("extract.mpd reads Complex_input/run.log (run with -r -i pars.out)", {
  mpd <- extract.mpd(file = testmodel("Complex_input/run.log"), quiet = TRUE)
  expect_s3_class(mpd, "casal2MPD")
  # Report *estimate_value[summary] → key is the label "summary"
  expect_true("summary" %in% names(mpd))
})

test_that("extract.mpd Complex_input estimate_value has catchability[CSacous].q column", {
  mpd <- extract.mpd(file = testmodel("Complex_input/run.log"), quiet = TRUE)
  # Multi-input run (-i pars.out) wraps each report in a numbered list
  vals <- mpd[["summary"]][[1]]$values
  expect_true(is.data.frame(vals))
  expect_true("catchability[CSacous].q" %in% colnames(vals))
})

# ---------------------------------------------------------------------------
# Error / edge-case handling
# ---------------------------------------------------------------------------

test_that("extract.mpd raises an error for a file that does not exist", {
  expect_error(extract.mpd(file = testmodel("nonexistent/file.log"), quiet = TRUE))
})

test_that("extract.mpd raises an error when fed a tabular output file", {
  expect_error(
    extract.mpd(file = testmodel("Simple/projections.log"), quiet = TRUE),
    regexp = "tabular|extract\\.tabular"
  )
})
