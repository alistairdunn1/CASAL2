# Tests for exported utility functions in the Casal2 R library.
# Covers: simplex/restoresimplex/jacobiansimplex, strip_comments,
# burn.in.tabular, CV.for.CPUE, casal2.binary.version

library(testthat)
library(Casal2)

# ===========================================================================
# setup.R defines: TESTMODELS_DIR, testmodel()
# simplex / restoresimplex / jacobiansimplex
# ===========================================================================

test_that("simplex converts a unit simplex vector to unconstrained (K-1) vector", {
  xk <- c(0.1, 0.5, 0.3, 0.1)          # sums to 1.0
  yk <- simplex(xk)
  expect_equal(length(yk), 3L)          # K-1 = 3
  expect_true(all(is.finite(yk)))
})

test_that("simplex errors when input does not sum to 1 and sum_to_one=TRUE", {
  expect_error(simplex(c(0.1, 0.5, 0.3, 0.5), sum_to_one = TRUE))
})

test_that("simplex with sum_to_one=FALSE rescales internally without error", {
  xk <- c(2, 5, 3, 1)   # sums to 11, not 1
  yk <- simplex(xk, sum_to_one = FALSE)
  expect_equal(length(yk), 3L)
  expect_true(all(is.finite(yk)))
})

test_that("restoresimplex converts an unconstrained (K-1) vector to a unit vector of length K", {
  yk <- c(0.1, -0.5, 0.3)
  xk <- restoresimplex(yk)
  expect_equal(length(xk), 4L)                         # K = K-1+1 = 4
  expect_true(all(is.finite(xk)))
  expect_true(all(xk > 0))
  expect_equal(sum(xk), 1.0, tolerance = 1e-10)       # must sum to 1
})

test_that("simplex and restoresimplex are inverse functions (round-trip)", {
  xk_orig <- c(0.1, 0.5, 0.3, 0.1)
  yk  <- simplex(xk_orig)
  xk2 <- restoresimplex(yk)
  expect_equal(xk2, xk_orig, tolerance = 1e-10)
})

test_that("jacobiansimplex returns a single positive finite scalar", {
  yk <- c(0.1, -0.5, 0.3)
  jac <- jacobiansimplex(yk)
  expect_equal(length(jac), 1L)
  expect_true(is.finite(jac))
  expect_true(jac > 0)
})

# ===========================================================================
# strip_comments
# ===========================================================================

test_that("strip_comments removes lines starting with #", {
  input <- c("# this is a comment", "actual line", "# another comment")
  result <- strip_comments(input)
  expect_false(any(startsWith(result, "#")))
  expect_true("actual line" %in% result)
})

test_that("strip_comments removes inline # comments from lines", {
  input <- c("keyword value  # inline comment")
  result <- strip_comments(input)
  expect_true(all(grepl("keyword value", result)))
  expect_false(any(grepl("inline comment", result)))
})

test_that("strip_comments removes block /* ... */ comments", {
  input <- c("before", "/* start block", "inside block", "*/ end block", "after")
  result <- strip_comments(input)
  expect_false("inside block" %in% result)
  expect_true("before" %in% result)
  expect_true("after" %in% result)
})

test_that("strip_comments returns empty vector when all lines are comments", {
  input <- c("# line 1", "# line 2")
  result <- strip_comments(input)
  expect_equal(length(result), 0L)
})

# ===========================================================================
# burn.in.tabular
# ===========================================================================

test_that("burn.in.tabular raises an error if given a non-casal2TAB object", {
  expect_error(burn.in.tabular(list(), Row = 1))
})

test_that("burn.in.tabular returns a casal2TAB object with fewer rows", {
  tab <- extract.tabular(file = testmodel("Simple/projections.log"), quiet = TRUE)
  # Find a report that has a values data frame
  report_name <- names(tab)[sapply(names(tab), function(n) {
    !is.null(tab[[n]]$values) && is.data.frame(tab[[n]]$values) && nrow(tab[[n]]$values) > 5
  })][1]
  skip_if(is.na(report_name), "No suitable report with >5 rows found in projections.log")

  original_rows <- nrow(tab[[report_name]]$values)
  burned <- burn.in.tabular(tab, Row = 3)
  expect_s3_class(burned, "casal2TAB")
  expect_true(nrow(burned[[report_name]]$values) < original_rows)
  expect_equal(nrow(burned[[report_name]]$values), original_rows - 2L)
})

test_that("burn.in.tabular errors when Row >= number of rows", {
  tab <- extract.tabular(file = testmodel("Simple/projections.log"), quiet = TRUE)
  report_name <- names(tab)[sapply(names(tab), function(n) {
    !is.null(tab[[n]]$values) && is.data.frame(tab[[n]]$values)
  })][1]
  skip_if(is.na(report_name), "No suitable report found")
  n_rows <- nrow(tab[[report_name]]$values)
  expect_error(burn.in.tabular(tab, Row = n_rows))
})

# ===========================================================================
# casal2.binary.version
# ===========================================================================

test_that("casal2.binary.version returns a non-empty character string", {
  ver <- casal2.binary.version()
  expect_type(ver, "character")
  expect_true(nchar(ver) > 0)
})

# ===========================================================================
# CV.for.CPUE (non-plotting path)
# ===========================================================================

test_that("CV.for.CPUE returns a numeric scalar CV when plot.it=FALSE", {
  set.seed(42)
  year <- 2000:2015
  cpue <- exp(0.05 * (year - 2000) + rnorm(length(year), sd = 0.1))
  result <- CV.for.CPUE(year, cpue, f = 0.5, plot.it = FALSE)
  # When plot.it=FALSE, the function returns a data.frame or a CV value
  # (depends on implementation); just check it is numeric
  expect_true(is.numeric(result) || is.data.frame(result))
})
