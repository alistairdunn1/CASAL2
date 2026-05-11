#' Utility extract function
#'
#' @author Casal2 Development Team
#' @keywords internal
#'
remove.last.words <- function(string, words = 1) {
  temp <- strsplit(string, split = " ", fixed = TRUE)[[1L]]
  to.drop <- length(temp) - (0:(words - 1))
  paste(unlist(temp[-to.drop]), collapse = " ")
}
