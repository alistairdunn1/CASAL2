#' Utility extract function
#'
#' @author Casal2 Development Team
#' @description
#' create a matrix that has a header
#' @keywords internal
#'
"make.matrix_with_header" <- function(lines) {
  if (length(lines) < 2L) return(NA)
  ncols <- length(string.to.vector.of.words(lines[1L]))
  rows  <- lapply(lines[-1L], string.to.vector.of.numbers)
  bad   <- which(lengths(rows) != ncols)
  if (length(bad) > 0L) {
    stop(paste(lines[bad[1L] + 1L], "is not the same length as", lines[1]))
  }
  do.call(rbind, rows)
}
