#' @title extract Tabular function for reading in Casal2 output that has been generated from a -r, -e, -f, -p run mode with the --tabular.
#'
#' @description
#' An extract function that reads Casal2 output that are produced from a '-r' or '-e' or '-f' or '-p' model run. This function
#' also create a 'casal2TAB' class which can be used in plotting and summary functions. See the casal2 manual for more information.
#'
#' @author Casal2 Development Team
#' @param file the name of the input file containing model output to extract
#' @param path Optionally, the path to the file
#' @param fileEncoding Optional, allows the R-library to read in files that have been encoded in alternative UTF formats, see the manual for the error message that would indicate when to use this switch.
#' @param quiet suppress print or cat statements to screen.
#' @return a 'casal2TAB' object which is essentially a list, that can be integrated using the str() function.
#' @export
#'
#'

# 2021-08-30
# {d} -> {dataframe}
# {d_r} -> {dataframe_with_row_labels}
# {m} -> {matrix}
# {L} -> {list}
# {L_E} -> {end_list} ?
# end {L} -> {end_list}
# {v} -> {vector}
# {s} -> {string}
# {c} and {C} not used

"extract.tabular" <- function(file, path = "", fileEncoding = "", quiet = FALSE) {
  filename <- if (nzchar(path)) file.path(path, file) else file

  ## Read file once into memory.  readLines is faster than scan() for large
  ## files and avoids the two-pass approach previously used.
  file_lines <- readLines(filename,
    encoding = if (nzchar(fileEncoding)) fileEncoding else "unknown",
    warn     = FALSE
  )
  if (!quiet) cat(sprintf("Read %d lines\n", length(file_lines)))

  ## Check this is a tabular report
  if (!grepl(pattern = "--tabular", x = file_lines[2]) & !grepl(pattern = "-t ", x = file_lines[2])) {
    stop("This model was NOT run with the command '--tabular'. Please use the extract.mpd() function to import model runs without --tabular or -t")
  }

  ## Pre-compute all report-header and *end positions in one pass -- avoids
  ## O(n^2) progressive clipping of the file vector inside the loop.
  star_ndx <- grep("^\\*", file_lines)
  end_ndx_all <- star_ndx[file_lines[star_ndx] == "*end"]
  report_start_ndx <- star_ndx[file_lines[star_ndx] != "*end"]

  if (length(report_start_ndx) == 0L) {
    warning("File is empty, no reports found")
    return(NULL)
  }

  if (!is.even(length(star_ndx))) {
    ## find the report which doesn't have a *end
    paired_ends <- end_ndx_all[seq_along(report_start_ndx)]
    bad <- which(is.na(paired_ends) | file_lines[report_start_ndx] == "*end")
    stop(paste0(
      "Each report section must begin with '*' and end with '*end'. ",
      "The report beginning with ", file_lines[report_start_ndx[bad[1]]],
      " does not have a trailing '*end'."
    ))
  }

  result <- list()

  for (i in seq_along(report_start_ndx)) {
    start_line <- report_start_ndx[i]
    end_line <- end_ndx_all[i]

    header_text <- file_lines[start_line]
    header <- split.header(header_text)
    label <- header[1]
    type <- header[2]

    if (!quiet) {
      print(paste0("loading tabular report '", label, "' of type '", type, "'"))
    }

    if (type == "info") {
      if (!quiet) print("Found informational messages in the output. Skipping that report.")
      next
    }
    if (type == "warnings") {
      if (!quiet) print("Found warning messages in the output. Skipping that report.")
      next
    }

    ## Non-blank lines that belong to this report (exclusive of header and *end)
    raw_range <- file_lines[(start_line + 1L):(end_line - 1L)]
    content_lines <- raw_range[nzchar(trimws(raw_range))]

    temp_result <- list()
    line_no <- 1L

    while (line_no <= length(content_lines)) {
      current_line <- content_lines[line_no]
      report_type <- get.line.type(current_line)
      report_label <- get.line.label(current_line)

      if (report_type == "L_E") {
        if (report_label == "type") report_label <- "sub_type"
        temp_result[[report_label]] <- make.list_element(current_line)
        line_no <- line_no + 1L
      } else if (report_type == "dataframe") {
        ## Next content line is the column-header row; the rest are data.
        ## Empty-report guard: nothing after the "values {dataframe}" marker.
        if (line_no + 1L > length(content_lines)) {
          line_no <- length(content_lines) + 1L
          next
        }

        col_header <- string.to.vector.of.words(content_lines[line_no + 1L])
        data_lines <- content_lines[(line_no + 2L):length(content_lines)]

        if (length(data_lines) > 0L) {
          ## Use in-memory textConnection -- avoids re-reading from disk.
          con <- textConnection(data_lines)
          Data <- read.table(con,
            stringsAsFactors = FALSE, sep = " ",
            header = FALSE, strip.white = FALSE, fill = FALSE
          )
          close(con)
          ## Drop trailing NA column caused by trailing whitespace
          if (ncol(Data) > 0L && all(is.na(Data[, ncol(Data)]))) {
            Data <- Data[, -ncol(Data), drop = FALSE]
          }
          colnames(Data) <- col_header
        } else {
          Data <- data.frame()
        }

        temp_result$values <- Data
        line_no <- length(content_lines) + 1L
      } else {
        warning(paste0("Cannot parse tabular report of type '", report_type, "'"))
        line_no <- line_no + 1L
      }
    }

    temp_result$type <- type
    result[[label]] <- temp_result
  }

  result <- set.class(result, "casal2TAB")
  return(result)
}
