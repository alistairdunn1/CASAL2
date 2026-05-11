#' Utility extract function
#'
#' @author Casal2 Development Team
#' @keywords internal
#'
make.data.frame <- function(lines) {
  columns <- string.to.vector.of.words(lines[1])
  rows    <- lapply(lines[-1], string.to.vector.of.words)
  bad     <- which(lengths(rows) != length(columns))
  if (length(bad) > 0L) {
    stop(paste(lines[bad[1L] + 1L], "is not the same length as", lines[1]))
  }
  data <- as.data.frame(do.call(rbind, rows), stringsAsFactors = FALSE)
  colnames(data) <- columns
  for (i in seq_len(ncol(data))) {
    if (is.all.numeric(data[[i]])) data[[i]] <- as.numeric(data[[i]])
  }
  data
}
