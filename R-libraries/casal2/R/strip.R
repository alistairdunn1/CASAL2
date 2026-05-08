#' Utility for extract function
#'
#' @author Casal2 Development Team
#' @keywords internal
#'
strip <- function(x) {
  tmp <- unlist(strsplit(x, "\t"))
  tmp <- unlist(strsplit(tmp, " "))
  return(as.vector(paste(tmp, collapse = " ")))
}
