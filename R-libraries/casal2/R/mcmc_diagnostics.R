# This file is part of RStan
# Copyright (C) 2012, 2013, 2014, 2015, 2016, 2017, 2018 Trustees of Columbia University
# Copyright (C) 2018, 2019 Aki Vehtari, Paul Bürkner
#
# RStan is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# RStan is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

## ---- internal helpers (not exported) ----------------------------------------

fft_next_good_size <- function(N) {
  if (N <= 2) {
    return(2)
  }
  while (TRUE) {
    m <- N
    while ((m %% 2) == 0) m <- m / 2
    while ((m %% 3) == 0) m <- m / 3
    while ((m %% 5) == 0) m <- m / 5
    if (m <= 1) {
      return(N)
    }
    N <- N + 1
  }
}

is_constant <- function(x, tol = .Machine$double.eps) {
  abs(max(x) - min(x)) < tol
}

should_return_NA <- function(sims) {
  anyNA(sims) || any(!is.finite(sims)) || is_constant(sims)
}

z_scale <- function(x) {
  n <- length(x)
  r <- rank(x, ties.method = "average")
  z <- qnorm((r - 1 / 2) / n)
  z[is.na(x)] <- NA
  if (!is.null(dim(x))) {
    z <- array(z, dim = dim(x), dimnames = dimnames(x))
  }
  z
}

split_chains <- function(sims) {
  if (is.vector(sims)) {
    dim(sims) <- c(length(sims), 1)
  }
  niter <- dim(sims)[1]
  if (niter == 1L) {
    return(sims)
  }
  half <- niter / 2
  cbind(sims[1:floor(half), ], sims[ceiling(half + 1):niter, ])
}

autocovariance <- function(y) {
  N <- length(y)
  M <- fft_next_good_size(N)
  Mt2 <- 2 * M
  yc <- y - mean(y)
  yc <- c(yc, rep.int(0, Mt2 - N))
  transform <- fft(yc)
  ac <- fft(Conj(transform) * transform, inverse = TRUE)
  Re(ac)[1:N] / (N^2 * 2)
}

autocorrelation <- function(y) {
  ac <- autocovariance(y)
  ac / ac[1]
}

rhat_rfun <- function(sims) {
  if (anyNA(sims)) {
    return(NA)
  }
  if (any(!is.finite(sims))) {
    return(NaN)
  }
  if (is_constant(sims)) {
    return(NA)
  }
  if (is.vector(sims)) {
    dim(sims) <- c(length(sims), 1)
  }
  chains <- ncol(sims)
  n_samples <- nrow(sims)
  chain_mean <- numeric(chains)
  chain_var <- numeric(chains)
  for (i in seq_len(chains)) {
    chain_mean[i] <- mean(sims[, i])
    chain_var[i] <- var(sims[, i])
  }
  var_between <- n_samples * var(chain_mean)
  var_within <- mean(chain_var)
  sqrt((var_between / var_within + n_samples - 1) / n_samples)
}

ess_rfun <- function(sims) {
  if (is.vector(sims)) {
    dim(sims) <- c(length(sims), 1)
  }
  chains <- ncol(sims)
  n_samples <- nrow(sims)
  if (n_samples < 3L || should_return_NA(sims)) {
    return(NA)
  }
  acov <- lapply(seq_len(chains), function(i) autocovariance(sims[, i]))
  acov <- do.call(cbind, acov)
  chain_mean <- apply(sims, 2, mean)
  mean_var <- mean(acov[1, ]) * n_samples / (n_samples - 1)
  var_plus <- mean_var * (n_samples - 1) / n_samples
  if (chains > 1) {
    var_plus <- var_plus + var(chain_mean)
  }
  rho_hat_t <- rep.int(0, n_samples)
  t <- 0
  rho_hat_even <- 1
  rho_hat_t[t + 1] <- rho_hat_even
  rho_hat_odd <- 1 - (mean_var - mean(acov[t + 2, ])) / var_plus
  rho_hat_t[t + 2] <- rho_hat_odd
  while (t < nrow(acov) - 5 && !is.nan(rho_hat_even + rho_hat_odd) &&
    (rho_hat_even + rho_hat_odd > 0)) {
    t <- t + 2
    rho_hat_even <- 1 - (mean_var - mean(acov[t + 1, ])) / var_plus
    rho_hat_odd <- 1 - (mean_var - mean(acov[t + 2, ])) / var_plus
    if ((rho_hat_even + rho_hat_odd) >= 0) {
      rho_hat_t[t + 1] <- rho_hat_even
      rho_hat_t[t + 2] <- rho_hat_odd
    }
  }
  max_t <- t
  if (rho_hat_even > 0) {
    rho_hat_t[max_t + 1] <- rho_hat_even
  }
  t <- 0
  while (t <= max_t - 4) {
    t <- t + 2
    if (rho_hat_t[t + 1] + rho_hat_t[t + 2] > rho_hat_t[t - 1] + rho_hat_t[t]) {
      rho_hat_t[t + 1] <- (rho_hat_t[t - 1] + rho_hat_t[t]) / 2
      rho_hat_t[t + 2] <- rho_hat_t[t + 1]
    }
  }
  ess <- chains * n_samples
  tau_hat <- -1 + 2 * sum(rho_hat_t[1:max_t]) + rho_hat_t[max_t + 1]
  tau_hat <- max(tau_hat, 1 / log10(ess))
  ess / tau_hat
}

## ---- exported functions ------------------------------------------------------

#' @title Rhat
#' @description Compute the improved Rhat convergence diagnostic (rank
#'   normalised, folded split-Rhat) for a single MCMC parameter.
#' @author Aki Vehtari, Paul Bürkner (RStan); ported to casal2 R library.
#' @param sims A 2D array \emph{without} warmup samples (iterations x chains),
#'   or a vector when a single chain is supplied.
#' @return A single numeric Rhat value.  Values close to 1 indicate convergence.
#' @references Vehtari et al. (2019) arXiv:1903.08008.
#' @export Rhat
Rhat <- function(sims) {
  bulk_rhat <- rhat_rfun(z_scale(split_chains(sims)))
  sims_folded <- abs(sims - median(sims))
  tail_rhat <- rhat_rfun(z_scale(split_chains(sims_folded)))
  max(bulk_rhat, tail_rhat)
}

#' @title ess_bulk
#' @description Compute the bulk effective sample size (bulk-ESS) for a single
#'   MCMC parameter using rank normalisation and split chains.
#' @author Aki Vehtari, Paul Bürkner (RStan); ported to casal2 R library.
#' @param sims A 2D array \emph{without} warmup samples (iterations x chains),
#'   or a vector when a single chain is supplied.
#' @return A single numeric bulk-ESS value.
#' @references Vehtari et al. (2019) arXiv:1903.08008.
#' @export ess_bulk
ess_bulk <- function(sims) {
  ess_rfun(z_scale(split_chains(sims)))
}

#' @title ess_tail
#' @description Compute the tail effective sample size (tail-ESS) for a single
#'   MCMC parameter as the minimum ESS at the 5\% and 95\% quantiles.
#' @author Aki Vehtari, Paul Bürkner (RStan); ported to casal2 R library.
#' @param sims A 2D array \emph{without} warmup samples (iterations x chains),
#'   or a vector when a single chain is supplied.
#' @return A single numeric tail-ESS value.
#' @references Vehtari et al. (2019) arXiv:1903.08008.
#' @export ess_tail
ess_tail <- function(sims) {
  I05 <- sims <= quantile(sims, 0.05, na.rm = TRUE)
  q05_ess <- ess_rfun(split_chains(I05))
  I95 <- sims <= quantile(sims, 0.95, na.rm = TRUE)
  q95_ess <- ess_rfun(split_chains(I95))
  min(q05_ess, q95_ess)
}
