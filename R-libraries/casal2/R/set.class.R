#' Set the class of an R object
#'
#' @param object R object to modify
#' @param new.class character string naming the new class to prepend
#' @keywords internal
#'
set.class <- function(object, new.class) {
  attributes(object)$class <- c(new.class, attributes(object)$class[attributes(object)$class != new.class])
  object
}
