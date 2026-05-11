#' Utility extract function
#'
#' @author Casal2 Development Team
#' @keywords internal
#'
makeDataFrameRowLabels <- function(lines) {
  columns  <- string.to.vector.of.words(lines[1])[-1] ## remove first element (row-label column header)
  rows     <- lapply(lines[-1], string.to.vector.of.words)
  row_labs <- vapply(rows, `[[`, character(1L), 1L)
  rows     <- lapply(rows, `[`, -1L)
  bad      <- which(lengths(rows) != length(columns))
  if (length(bad) > 0L) {
    stop(paste(lines[bad[1L] + 1L], "is not the same length as", lines[1]))
  }
  data <- as.data.frame(do.call(rbind, rows), stringsAsFactors = FALSE)
  colnames(data) <- columns
  rownames(data) <- row_labs
  for (i in seq_len(ncol(data))) {
    if (is.all.numeric(data[[i]])) data[[i]] <- as.numeric(data[[i]])
  }
  data
}
