#' @title Casal2 R library
#' @description A set of R functions for extracting and summarising output from
#'   Casal2 fisheries stock assessment model runs.
#' @author Casal2 Development Team
#' @name Casal2-package
#' @aliases Casal2
#' @importFrom graphics axis box lines mtext points segments
#' @importFrom stats cov2cor lowess optimize plogis qlogis rlnorm rnorm runif var
#' @importFrom utils read.table
"_PACKAGE"

## Suppress R CMD check notes for ggplot2 aesthetic variable names used in
## summarise_estimated_parameters() when plot_it = TRUE.


## Build a file path from a directory path and file name.
## Used by write.csl2.file() and similar functions.
make.filename <- function(file = "", path = "", add.terminal = FALSE) {
  if (path != "") {
    plc <- substring(path, nchar(path))
    if (!(plc == "\\" | plc == "/")) path <- paste(path, "\\", sep = "")
  }
  filename <- paste(path, file, sep = "")
  if (add.terminal) {
    plc <- substring(filename, nchar(filename))
    if (!(plc == "\\" | plc == "/")) filename <- paste(filename, "\\", sep = "")
  }
  return(filename)
}

## Read a text file into a character vector of lines.
## Used by ReadSimulatedData() in place of the old casal::convert.to.lines().
convert.to.lines <- function(file, ...) {
  readLines(file, warn = FALSE, ...)
}
