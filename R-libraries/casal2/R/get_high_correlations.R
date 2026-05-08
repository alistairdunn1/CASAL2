#' @title get_high_correlations
#' @description Get highly correlated parameter pairs from a correlation matrix.
#' @author Craig Marsh
#' @param correlation_matrix A symmetric correlation matrix.
#' @param max_correlation The correlation threshold; pairs with absolute
#'   correlation above this value are returned.  Must be in (0, 1]
#'   (default \code{0.8}).
#' @param labels Optional character vector of parameter labels matching the
#'   rows/columns of \code{correlation_matrix}.
#' @param tol Tolerance for the symmetry check (default
#'   \code{.Machine$double.eps}).  Increase if rounding causes false failures.
#' @return A data frame with columns \code{row_ndx}, \code{col_ndx},
#'   \code{correlation}, and (when \code{labels} is supplied) \code{row_param}
#'   and \code{col_param}.  Returns \code{NULL} when no pairs exceed
#'   \code{max_correlation}.
#' @rdname get_high_correlations
#' @export get_high_correlations
get_high_correlations <- function(correlation_matrix, max_correlation = 0.8, labels = NULL, tol = .Machine$double.eps) {
  if (!is.matrix(correlation_matrix)) {
    stop("correlation_matrix is not a matrix")
  }
  if (max_correlation <= 0 || max_correlation > 1) {
    stop("max_correlation must be > 0 and <= 1")
  }
  if (!isSymmetric(correlation_matrix, tol = tol)) {
    stop("correlation_matrix is not symmetric. Increasing tol may help.")
  }
  ## zero out lower triangle and diagonal so each pair is counted once
  correlation_matrix[lower.tri(correlation_matrix, diag = TRUE)] <- 0.0
  arr_ind <- which(abs(correlation_matrix) > max_correlation, arr.ind = TRUE)
  flat_ind <- which(abs(correlation_matrix) > max_correlation, arr.ind = FALSE)
  if (length(flat_ind) == 0L) {
    message("No correlations were greater than max_correlation")
    return(NULL)
  }
  corr_df <- data.frame(
    row_ndx = arr_ind[, 1],
    col_ndx = arr_ind[, 2],
    correlation = correlation_matrix[flat_ind],
    stringsAsFactors = FALSE
  )
  if (!is.null(labels)) {
    if (ncol(correlation_matrix) != length(labels)) {
      stop("labels length differs from the dimensions of correlation_matrix")
    }
    corr_df$row_param <- labels[arr_ind[, 1]]
    corr_df$col_param <- labels[arr_ind[, 2]]
  }
  return(corr_df)
}
