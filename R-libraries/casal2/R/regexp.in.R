#' Utility extract function
#'
#' @author Casal2 Development Team
#' @keywords internal
#'
"regexp.in" <- function(vector, regexp) {
  if (length(vector) == 0L) return(FALSE)
  any(regexpr(regexp, vector, fixed = TRUE) > 0)
}
