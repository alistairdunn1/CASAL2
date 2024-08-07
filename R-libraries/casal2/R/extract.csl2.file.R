#' Model configuration write function
#'
#' This function reads a Casal2 configuration file and returns a list object in R. Where each element is a command and subcommand from the configuration file
#'
#' @author Craig Marsh & Alistair Dunn
#' @param file the name of the input file containing model configuration
#' @param path Optionally, the path to the file
#' @param fileEncoding Optional, allows the R-library to read in files that have been encoded in alternative UTF formats, see the manual for the error message that would indicate when to use this switch.
#' @param quiet suppress print or cat statements to screen
#' @param include if true (default is false) process any !include filenames and add them to the returned object.
#' @returns a list of the Casal2 commands, subcommands, and arguments
#' @export
#'
"extract.csl2.file" <- function(file, path = "", fileEncoding = "", quiet = FALSE, include = FALSE) {
  ## if no path specified look in current directory
  if (missing(path)) {
    path <- ""
  }
  ## get the list that lines subcommands to their type.
  casal2_list <- get.casal2.unique_subcommands_list()
  filename <- make.filename(path = path, file = file)
  file <- convert.to.lines(filename, fileEncoding = fileEncoding, quiet = quiet)
  ## remove white space at the beginning of a subcommand or command e.g
  while (any(regexpr(" ", file) == 1)) {
    index <- regexpr(" ", file) == 1
    file <- ifelse(index, substring(file, 2), file)
  }
  ## try and convert tabs to spaces
  file <- as.vector(tapply(file, 1:length(file), strip))

  # process any !includes
  if (include) {
    print("Processing the !include statements in the csl2 file. Note that the path must be specified to allow the function to locate '!include' files")
    i <- 1
    while (i != length(file)) {
      if (tolower(substring(file[i], 1, 8)) == "!include") {
        temp <- string.to.vector.of.words(file[i])[2]
        temp <- gsub("\"", "", temp)
        temp <- make.filename(path = path, file = temp)
        if (!quiet) print(paste("Adding !include file: ", temp, sep = ""))
        new.file <- convert.to.lines(temp, fileEncoding = fileEncoding, quiet = quiet)
        ## remove white space at the beginning of a subcommand or command e.g
        while (any(regexpr(" ", new.file) == 1)) {
          index <- regexpr(" ", new.file) == 1
          new.file <- ifelse(index, substring(new.file, 2), new.file)
        }
        ## try and convert tabs to spaces
        new.file <- as.vector(tapply(new.file, 1:length(new.file), strip))
        file <- c(file[1:(i - 1)], new.file, file[(i + 1):length(file)])
      }
      i <- i + 1
    }
  }

  ## Remove any lines that begin with a #
  file <- strip_comments(file)
  # utility function for stripping tabs and spaces out of input values
  blocks <- get.lines(file, starts.with = "\\@", fixed = F)
  ## create a labels for blocks that do not take a label following the @block statement
  exception_blocks <- c("model", "categories")
  ## a list of tables that don't have headers
  non_header_tables <- c("obs", "data", "scanned", "recaptured", "error_values", "table", "proportions")
  ## there are three types of tables, 1) tables with headers (Instant mortality) 2) tables with row labels (observations and error values) and 3)tables that are just a matrix (ageing error)
  ans <- list()
  if (!quiet) {
    print(paste("The 'csl2' input parameter file has", length(file[substring(file, 1, 1) == "@"]), "commands, and", length(file), "lines"))
  }
  CommandCount <- 0
  ## A global variable to tell us if we are still inputting a table
  in_table <- FALSE
  label_name <- 1
  file <- file[file != ""]
  ## this will be the default label if no user defined label is specified
  for (i in 1:length(file)) {
    temp <- string.to.vector.of.words(file[i])
    ## expand numeric shorthand
    # if (any(check_inputs$numeric) && (substr(Command, 1, 3) != "est") | (substr(Command, 1, 3) != "par")) {
    # vals <- eval(parse(text = temp[check_inputs$numeric]))
    # eval(parse(text = "temp = paste(c(temp[!check_inputs$numeric],as.character(vals)), collapse = ' ')"))
    # temp <- string.to.vector.of.words(temp)
    # }
    ## expand String shorthand
    # if (any(check_inputs$string)) {
    ## Do nothing for now, this one needs more thought
    # }
    ## Check if it is a beginning of a block
    if (substring(temp[1], 1, 1) == "@") {
      if (is.na(temp[2])) {
        temp[2] <- paste(substr(temp[1], 2, 1000), "_", label_name, sep = "")
        label_name <- label_name + 1
      }
      ## create a block
      header <- 1
      CommandCount <- CommandCount + 1
      Command <- substring(temp[1], 2)
      if (!is.in(Command, exception_blocks)) {
        ## Create a label for the block
        Command <- paste(Command, "[", temp[2], "]", sep = "")
        if (!quiet) {
          print(temp[2])
        }
      }
      next
      ## if we come across a block we either give it a label and move on or just move on if it is an exception block
    }
    ## only two types of subcommands tables and vectors
    if (in_table || any(casal2_list$command == temp[1])) {
      type <- "table_label"
    } else {
      type <- "vector"
    }
    ## Check if it is a valid type/subcommand
    if (type == "vector") {
      new_string <- temp[1]
      ## if category or parameter subcommand skip expand short hand for * and :
      # new_string = temp

      if (grepl(temp[1], pattern = "categor", fixed = TRUE) | grepl(temp[1], pattern = "parameter", fixed = TRUE)) {
        new_string <- temp
      } else {
        ## check white space hasn't split it up already
        second_values <- NULL
        if (any(temp[2:length(temp)] == "*")) {
          second_values <- paste(temp[2:length(temp)], collapse = "")
        } else {
          second_values <- temp[2:length(temp)]
        }
        ## deal with : shorthand
        for (j in 1:length(second_values)) {
          try_convert <- tryCatch(expr = expand_shorthand_syntax(second_values[j]), error = function(e) {
            e
          }, warning = function(w) {
            w
          })
          if (inherits(try_convert, "error") | inherits(try_convert, "warning")) {
            new_string <- c(new_string, second_values[j])
          } else {
            new_string <- c(new_string, expand_shorthand_syntax(second_values[j]))
          }
        }
      }
      ans[[Command]][[new_string[1]]] <- list("value" = new_string[-1])
    } else if ((type == "table_label") || in_table) {
      ## deal with a table input. the biggest pain in the ***
      in_table <- TRUE
      if (header == 1) {
        Label <- temp[2]
        table_list <- list()
        mat <- matrix()
      } else {
        ## try the other cases
        if (header == 2 && !Label %in% non_header_tables) {
          ## We need to read in the header labels for the table
          Colnames <- temp
          ## initialise temp list object for storing info into the table
        } else if ((header > 2 && temp[1] != "end_table") || (Label %in% non_header_tables && temp[1] != "end_table")) {
          ## create a temp list which will be the same as
          if (!Label %in% non_header_tables) {
            ## if not an observational table
            for (j in 1:length(Colnames)) {
              table_list[[Colnames[j]]] <- c(table_list[[Colnames[j]]], temp[j])
            }
          } else {
            ## else make an exception for an observational table and ageing error
            if (Label == "table") {
              # then this is a ageing error matrix
              if (header == 2) {
                mat <- temp
              } else {
                mat <- rbind(mat, temp)
              }
            } else {
              table_list[[temp[1]]] <- temp[-1]
            }
          }
        } else if (length(casal2_list$type[casal2_list$command == temp[1]] == "end_table") > 0) {
          ## an initial check to prevent logical(0) in the condition
          if (temp[1] == "end_table") {
            ## we are leaving the table inputs
            in_table <- FALSE
            header <- 1
            if (!Label == "table") {
              for (k in 1:length(names(table_list))) {
                eval(parse(text = paste("ans[['", Command, "']]$Table$", Label, "[['", names(table_list)[k], "']] = table_list$'", names(table_list)[k], "'", sep = "")))
              }
            } else {
              dimnames(mat) <- NULL
              eval(parse(text = paste("ans[['", Command, "']]$Table$", Label, " = mat", sep = "")))
            }
            next
          }
        }
      }
      header <- header + 1
    } else {
      stop(paste("Haven't come up with R code to deal with this subcommand", temp[1]))
    }
  }
  ans
}
