library("partitions")

lineage_partitions <- function(n) {
    p <- parts(n)
    return(apply(p, 2, function(x) {x[x!=0]}))
}
ton_counts <- function(p, n) {
    result <- rep(0, n)
    counts <- table(p)
    for (i in seq(n)) {
        c <- counts[names(counts)==i]
        if (length(c)) {
            result[i] <- c
        }
    }
    return(result)
}
multinom_coef <- function(params) {
    if (length(params) == 1) {
        return(1)
    }
    return(choose(sum(params), params[length(params)]) * multinom_coef(params[-length(params)]))
}
get_block_rewards <- function(n) {
    rewards <- matrix(0, n, n)
    for (p in lineage_partitions(n)) {
        counts <- ton_counts(p, n)
        rewards[length(p), ] <- rewards[length(p), ] + counts * multinom_coef(counts)
    }
    rewards <- rewards[-1, -ncol(rewards)]
    scale <- (2:n)/apply(rewards, 1, sum)
    rewards <- rewards * scale
    rewards <- t(rewards)[, ncol(rewards):1]
    rewards <- cbind(rep(0, nrow(rewards)), rewards, rep(0, nrow(rewards)))
    return(rewards)
}

library(parallel)

v <- 3:100
n <- as.list(v)
names(n) <- v
sfs_rewards <- mclapply(n, get_block_rewards, mc.cores=detectCores())
saveRDS(sfs_rewards, file="sfs_rewards.RData")

