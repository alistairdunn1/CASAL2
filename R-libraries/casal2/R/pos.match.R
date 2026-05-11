#' Utility extract function
#'
#' @author Casal2 Development Team
#' @keywords internal
#'
"pos.match" <- function(vector, regexp) {
  which(regexpr(regexp, vector, fixed = TRUE) > 0)[1L]
}
