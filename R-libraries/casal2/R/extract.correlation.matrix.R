#' Utility extract.correlation.matrix function
#'
#' This function reads in the 'mpd.out' file created in estimation mode
#'
#' @param file the name of the file containing the estimated covariance matrix ('mpd.out')
#' @param path (optional) the path to the file
#' @param fileEncoding (optional) allows the R library to read in files that have been encoded in alternative UTF formats. See the User Manual for the error message that would indicate when to use this switch.
#' @param quiet suppress print or cat statements to screen.
#' @return correlation matrix
#' @export
#'
"extract.correlation.matrix" <- function(file, path = "", fileEncoding = "", quiet = FALSE) {
  data <- extract.covariance.matrix(file, path, fileEncoding, quiet)

  if (dim(data)[1] > 1 & !is.na(data[1, 1])) {
    data <- cov2cor(data)
  }
  return(data)
}
