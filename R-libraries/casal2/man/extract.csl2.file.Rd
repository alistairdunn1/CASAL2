% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/extract.csl2.file.R
\name{extract.csl2.file}
\alias{extract.csl2.file}
\title{Model configuration write function}
\usage{
extract.csl2.file(
  file,
  path = "",
  fileEncoding = "",
  quiet = FALSE,
  include = FALSE
)
}
\arguments{
\item{file}{the name of the input file containing model configuration}

\item{path}{Optionally, the path to the file}

\item{fileEncoding}{Optional, allows the R-library to read in files that have been encoded in alternative UTF formats, see the manual for the error message that would indicate when to use this switch.}

\item{quiet}{suppress print or cat statements to screen}

\item{include}{if true (default is false) process any !include filenames and add them to the returned object.}
}
\value{
a list of the Casal2 commands, subcommands, and arguments
}
\description{
This function reads a Casal2 configuration file and returns a list object in R. Where each element is a command and subcommand from the configuration file
}
\author{
Craig Marsh & Alistair Dunn
}
