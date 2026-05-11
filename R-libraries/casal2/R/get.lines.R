#' Utility extract function
#'
#' @author Casal2 Development Team
#' @keywords internal
#'
get.lines <- function(lines, from = -1, to = -1, contains = "", starts.with = "", clip.to = "", clip.from = "", clip.to.match = "", clip.from.match = "", fixed = TRUE, ...) {
  result <- lines
  if (from > 0) {
    result <- result[(1:length(result)) >= from]
  }
  if (to > 0) {
    result <- result[(1:length(result)) <= to]
  }
  if (clip.to != "") {
    if (any(result == clip.to)) {
      result <- result[(match(clip.to, result) + 1):length(result)]
    }
  }
  if (clip.from != "") {
    if (any(result == clip.from)) {
      result <- result[1:(match(clip.from, result) - 1)]
    }
  }
  if (clip.to.match != "") {
    if (regexp.in(result, clip.to.match)) {
      result <- result[(pos.match(result, clip.to.match) + 1):length(result)]
    }
  }
  if (clip.from.match != "") {
    if (regexp.in(result, clip.from.match)) {
      result <- result[1:(pos.match(result, clip.from.match) - 1)]
    }
  }
  if (contains != "") {
    result <- result[regexpr(contains, result, fixed = fixed, ...) > 0]
  }
  if (starts.with != "") {
    result <- result[regexpr(paste0("^", starts.with), result, fixed = fixed, ...) > 0]
  }
  return(result)
}
