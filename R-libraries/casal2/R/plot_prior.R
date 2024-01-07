#' @title plot_prior
#' @description
#' Plot priors available in Casal2
#'
#' @author A. Dunn
#' @param type The prior type
#' @param mu mean of the distribution for the prior
#' @param sd standard deviation (where required) of the distribution for the prior
#' @param cv CV (where required) of the distribution for the prior
#' @param A The A parameter for the beta prior
#' @param B The B parameter for the beta prior
#' @param beta The beta parameter for the prior
#' @param bounds The bounds of the prior
#' @param xlim The x-axis limits for the plot
#' @param label TRUE/FALSE (default = FALSE) display the name of the prior
#' @param logx TRUE/FALSE (default = FALSE) to plot the xaxis in log-space
#' @param add TRUE/FALSE (default = FALSE) to add to an existing plot
#' @param dump TRUE/FALSE (default = FALSE) to dump the coordinates of the plotted line
#' @rdname plot_prior
#' @export plot_prior

"plot_prior" <- function(type = "lognormal", mu, sigma, cv, A = 0, B = 1, beta = 0, df = 3, bounds, xlim, label = F, xlab = "x", ylab = "Density", logx = F, add = F, plot.it = T, ...) {
  n <- 201
  if (missing(xlim) & !missing(bounds)) {
    xlim <- bounds
  } else if (missing(bounds) & !missing(xlim)) {
    bounds <- xlim
  } else if (missing(bounds) & missing(xlim)) stop("You must supply either the bounds or xlim or both")
  if (logx & (min(xlim) <= 0 | min(bounds) <= 0)) stop("With 'logx' option, the lower bound/xlim must be a postive number")
  Allowed <- c("normal", "lognormal", "normal_by_stdev", "uniform", "uniform_log", "normal_log", "beta", "students_t")
  type <- Allowed[as.numeric(pmatch(casefold(type), casefold(Allowed), nomatch = NA))]
  if (type == "") {
    stop(paste("Prior not found. Available priors are\n", paste(Allowed, collapse = ", ")))
  } else {
    cat(paste("Plotting ", type, " prior\n", sep = ""))
  }
  if (logx) {
    p <- exp(seq(log(bounds[1]), log(bounds[2]), length = n))
  } else {
    p <- seq(bounds[1], bounds[2], length = n)
  }

  if (type == "normal") {
    if (cv <= 0) stop("The CV on the normal prior must be greater than 0")
    res <- exp(-(0.5 * ((p - mu) / (cv * mu))^2))
  } else if (type == "lognormal") {
    if (cv <= 0) stop("The CV on the lognormal prior must be greater than 0")
    sigma <- sqrt(log(1 + (cv^2)))
    res <- exp(-(log(p) + 0.5 * ((log(p / mu) / sigma + sigma / 2)^2)))
  } else if (type == "normal_by_stdev") {
    if (sigma <= 0) stop("The sigma on the normal_by_stdev prior must be greater than 0")
    res <- exp(-(0.5 * (((p - mu) / sigma)^2)))
  } else if (type == "uniform") {
    if (logx) {
      res <- p
    } else {
      res <- rep(1, length(p))
    }
  } else if (type == "uniform_log") {
    if (logx) {
      res <- rep(1, length(p))
    } else {
      res <- exp(-log(p))
    }
  } else if (type == "normal_log") {
    if (sigma <= 0) stop("The sigma on the normal_log prior must be greater than 0")
    res <- exp(-(log(p) + 0.5 * (((log(p) - mu) / sigma)^2)))
  } else if (type == "beta") {
    new.mu <- (mu - A) / (B - A)
    new.t <- (((mu - A) * (B - mu)) / (sigma^2)) - 1
    if (new.t <= 0) stop("Standard deviation too large")
    if (min(bounds, na.rm = T) < A || max(bounds, na.rm = T) > B) stop("Bad bounds on Beta prior")
    Bm <- new.t * new.mu
    Bn <- new.t * (1 - new.mu)
    res <- (1 - Bm) * log(p - A) + (1 - Bn) * log(B - p)
    res <- exp(-res)
  } else if (type == "students_t") {
    x1 <- lgamma((df + 1) / 2) - lgamma(df / 2)
    x2 <- 1 / 2 * log(df * pi) + log(sigma)
    x3 <- log(1 + (1 / df) * ((p - mu) / sigma)^2)
    x4 <- (df + 1) / 2
    res <- exp(x1 - x2 - x3 * x4)
  } else {
    stop(paste0("Prior type ", type, " not found"))
  }

  res <- c(0, res / max(res), 0)
  p <- c(bounds[1], p, bounds[2])
  res <- res[p >= beta]
  p <- p[p >= beta]
  res <- res / casal.area(list("x" = p, "y" = res))
  if (plot.it) {
    ylim <- c(min(res), max(res) * 1.04)
    if (logx) {
      xlab <- paste("log(", xlab, ")", sep = "")
      if (!add) {
        plot(log(p), res, type = "n", xlab = xlab, ylab = "", xlim = log(xlim), axes = F, ylim = ylim)
        axis(side = 1)
        mtext(side = 2, line = 0.5, text = ylab)
        if (label) mtext(side = 3, type, adj = 0, line = 0.3)
        box()
      }
      lines(log(p), res, type = "l", ...)
    } else {
      if (!add) {
        plot(p, res, type = "n", xlab = xlab, ylab = "", xlim = xlim, axes = F, ylim = ylim)
        axis(side = 1)
        mtext(side = 2, line = 0.5, text = ylab)
        if (label) mtext(side = 3, type, adj = 0, line = 0.3)
        box()
      }
      lines(p, res, type = "l", ...)
    }
  }
  if (plot.it) {
    invisible(data.frame("x" = p, "y" = res))
  } else {
    return(data.frame("x" = p, "y" = res))
  }
}

# plot_prior("normal",mu = 0.7, cv = 0.5, bounds=c(0.001,2))
# plot_prior("lognormal", mu = 0.7, cv = 0.5, bounds = c(0.001, 2))
# plot_prior("normal_by_stdev", mu = 0.7, sigma = 0.5, bounds = c(0.001, 2))
# plot_prior("uniform",bounds=c(11,13))
# plot_prior("uniform_log",bounds=c(11,13))
# plot_prior("students_t",mu = 11, sigma = 2, df = 3, bounds=c(11,13))
