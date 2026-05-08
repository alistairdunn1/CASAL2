#' Utility extract function
#'
#' @author Casal2 Development Team
#' @description
#' create a matrix, does not expect header values.
#' @keywords internal
#'
"make.matrix" <- function(lines) {
  if (length(lines) < 1L) return(NA)
  rows  <- lapply(lines, string.to.vector.of.numbers)
  ncols <- length(rows[[1L]])
  bad   <- which(lengths(rows) != ncols)
  if (length(bad) > 0L) {
    stop(paste(lines[bad[1L]], "is not the same length as", lines[1]))
  }
  do.call(rbind, rows)
}
