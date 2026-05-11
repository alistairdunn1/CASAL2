#' @title combine.tabular used to return a list that combines multiple tabular reports into a single tabular report
#' @description returns a casal2TAB class that is several casal2TAB objects combined into one. Used when tabular output from different chains of the same model are to be combined into a single chain
#' @author Casal2 Development Team
#' @param tab_objects list object of tab_object casal2TAB objects
#' @param Row  Optionally, the Row number to burn in from. Note this is not the iteration but the row that corresponds to your iteration that you want to burn-in from. if keep > 1 then the iteration and row will be different
#' @param verbose print extra info.
#' @return a 'casal2TAB' object which has been manipulated.
#' @export
#'
"combine.tabular" <- function(tab_objects, Row = NULL, verbose = FALSE) {
  if (!is.list(tab_objects)) {
    stop("tab_objects must be a list of objects of class 'casal2TAB'")
  }
  if (length(tab_objects) < 2) {
    stop("tab_objects must be a list of more than one object of class 'casal2TAB'")
  }
  for (i in seq_along(tab_objects)) {
    if (class(tab_objects[[i]]) != "casal2TAB") {
      stop(paste0("The element ", i, "of tab_objects  must be class 'casal2TAB', but the parsed object is class '", class(tab_objects[[i]]), "'"))
    }
  }
  for (i in seq_along(tab_objects)) {
    if (any(names(tab_objects[[1]]) != names(tab_objects[[i]]))) {
      stop("The object of class 'casal2TAB' in tab_objects must be identical")
    }
  }
  if (!is.null(Row)) {
    for (i in seq_along(tab_objects)) {
      tab_objects[[i]] <- burn.in.tabular(tab_objects[[i]], Row = Row, verbose = verbose)
    }
  }
  tab_object <- tab_objects[[1]]

  n_objects <- length(tab_objects)
  for (i in seq_along(tab_object)) {
    this_list <- tab_object[[i]]
    if (!("values" %in% names(this_list))) {
      next
    }
    ## Collect all data frames then bind once -- avoids O(n^2) row copying
    ## that occurs when rbind-ing inside a loop.
    chunks <- vector("list", n_objects)
    chunks[[1L]] <- this_list$values
    for (j in 2L:n_objects) {
      chunks[[j]] <- tab_objects[[j]][[i]]$values
    }
    this_list$values <- do.call(rbind, chunks)
    tab_object[[i]] <- this_list
  }
  return(tab_object)
}
