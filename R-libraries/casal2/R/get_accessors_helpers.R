## Internal helpers shared by all get_* and summarise_* accessor functions.
## Not exported.

## Efficiently bind a list of data frames, ignoring NULL entries.
.bind_rows_list <- function(lst) {
  lst <- lst[!vapply(lst, is.null, logical(1L))]
  if (length(lst) == 0L) return(NULL)
  do.call(rbind, lst)
}

## Bind a list of data frames retaining only columns common to all data frames.
## Used where different reports may produce different column sets.
.safe_rbind <- function(df_list) {
  df_list <- df_list[!vapply(df_list, is.null, logical(1L))]
  if (length(df_list) == 0L) return(NULL)
  if (length(df_list) == 1L) return(df_list[[1L]])
  common_cols <- Reduce(intersect, lapply(df_list, colnames))
  do.call(rbind, lapply(df_list, function(d) d[, common_cols, drop = FALSE]))
}

## Convert a matrix (or data frame coerced to matrix) to long-form data frame.
## Mimics reshape2::melt(as.matrix(x)) - returns Var1, Var2, value columns.
.melt_matrix <- function(mat) {
  mat <- as.matrix(mat)
  rn <- if (!is.null(rownames(mat))) rownames(mat) else as.character(seq_len(nrow(mat)))
  cn <- if (!is.null(colnames(mat))) colnames(mat) else as.character(seq_len(ncol(mat)))
  nr <- nrow(mat)
  nc <- ncol(mat)
  data.frame(
    Var1  = rep(rn, times = nc),
    Var2  = rep(cn, each  = nr),
    value = as.vector(mat),
    stringsAsFactors = FALSE
  )
}

## Standard list-of-MPD dispatcher.
## Calls FUN(model[[i]], reformat_labels = reformat_labels, ...) for each element
## of a named list of casal2MPD objects, adds a model_label column, then binds.
.list_method <- function(model, FUN, reformat_labels = TRUE, ...) {
  labels <- if (reformat_labels) reformat_default_labels(names(model)) else names(model)
  rows   <- vector("list", length(model))
  for (i in seq_along(model)) {
    if (!inherits(model[[i]], "casal2MPD"))
      stop("This function only works on a named list with elements of class = 'casal2MPD'")
    df <- FUN(model[[i]], reformat_labels = reformat_labels, ...)
    if (!is.null(df)) {
      df$model_label <- labels[[i]]
      rows[[i]]      <- df
    }
  }
  .bind_rows_list(rows)
}
