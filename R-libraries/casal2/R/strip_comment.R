#' @title strip_comments
#'
#' @description
#' A utility function for stripping out comments that is
#' line start with #
#' or a wrapped in /* */
#'
#' @author Casal2 Development Team
#' @param file a file read in by scan
#' @return a file that has removed lines starting with # or betwee '/*' '*/'
#' @rdname strip_comments
#' @export
strip_comments <- function(file) {
  file <- file[substring(file, 1, 1) != "#"]
  index1 <- ifelse(substring(file, 1, 2) == "/*", 1:length(file), 0)
  index2 <- ifelse(substring(file, 1, 2) == "*/", 1:length(file), 0)
  index1 <- index1[index1 != 0]
  index2 <- index2[index2 != 0]
  if (length(index1) != length(index2)) {
    stop(paste("Error in the file. Cannot find a matching '/*' or '*/'"))
  }
  if (length(index1) > 0 || length(index2) > 0) {
    index <- unlist(apply(cbind(index1, index2), 1, function(x) {
      seq(x[1], x[2])
    }))
    file <- file[!1:length(file) %in% index]
  }
  file <- ifelse(regexpr("#", file) > 0, substring(file, 1, regexpr("#", file) - 1), file)

  file <- file[nzchar(file)]
  ## strip inline /* ... */ comment sections (vectorised; the original loop used
  ## a broken regex that matched any '/' and ran on every line)
  if (any(grepl("/*", file, fixed = TRUE))) {
    file <- gsub("/\\*.*?\\*/", "", file, perl = TRUE)
    file <- trimws(file)
    file <- file[nzchar(file)]
  }

  return(file)
}
