#' Utility extract function
#'
#' @author Casal2 Development Team
#' @keywords internal
#'
"string.to.vector.of.words" <- function(string) {
  temp <- strsplit(string, split = " ", fixed = TRUE)[[1L]]
  return(temp[temp != ""])
}
