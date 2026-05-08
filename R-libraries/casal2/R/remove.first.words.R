#' Utility extract function
#'
#' @author Casal2 Development Team
#' @keywords internal
#'
"remove.first.words" <- function(string, words = 1) {
  paste(strsplit(string, split = " ", fixed = TRUE)[[1L]][-(1:words)], collapse = " ")
}
