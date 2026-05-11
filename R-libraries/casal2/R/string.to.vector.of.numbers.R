#' Utility extract function
#'
#' @author Casal2 Development Team
#' @keywords internal
#'
"string.to.vector.of.numbers" <- function(string) {
  as.numeric(string.to.vector.of.words(string))
}
