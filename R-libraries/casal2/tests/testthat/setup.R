# Shared setup for all Casal2 testthat tests.
# Locates the TestModels directory regardless of whether tests are run via
# devtools::test() (cwd = package root) or interactively from tests/testthat/.

.find_testmodels <- function() {
  candidates <- c(
    normalizePath(file.path("..", "..", "TestModels"), mustWork = FALSE),        # from package root
    normalizePath(file.path("..", "..", "..", "..", "TestModels"), mustWork = FALSE)  # from tests/testthat/
  )
  for (p in candidates) {
    if (dir.exists(p)) return(p)
  }
  stop("Cannot locate TestModels directory. Run tests from the package root or tests/testthat/.")
}

TESTMODELS_DIR <- .find_testmodels()

# Helper: build a path inside TestModels
testmodel <- function(...) file.path(TESTMODELS_DIR, ...)
