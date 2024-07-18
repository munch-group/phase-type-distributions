
library(devtools)
library(ptdalgorithms)
library(rhdf5)
library("partitions")

block_coalescent <- function(n) {
      
    state_vector_length <- 1
    graph <- create_graph(state_vector_length)
    starting_vertex <- vertex_at(graph, 1)
    initial_state <- c(0)
    initial_state[1] <- n
    add_edge(starting_vertex, create_vertex(graph, initial_state), 1)
    index <- 2
    
    while (index <= vertices_length(graph)) {
      vertex <- vertex_at(graph, index)
      state <- vertex$state
      if (state[1] == 1) {
        break
      }
      rate <- state[1] * (state[1] - 1) / 2
      child_state <- state
      child_state[1] <- child_state[1] - 1
      add_edge(vertex, find_or_create_vertex(graph, child_state), rate)
      index <- index + 1
    }
    return(graph)
}


# equivalence_classes <- function(n) {
#     p <- parts(n)
#     return(apply(p, 2, function(x) {x[x!=0]}))
# }

equivalence_classes <- function(n, size=0) {
    if (size != 0) {    
        m <- apply(parts(n), 2, function(x) {x[x!=0]})
        p <- m[lapply(m, length) == size]
    } else {
        p <- apply(parts(n), 2, function(x) {x[x!=0]})
    }
    return(p)
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

multinom <- function(params) {
    if (length(params) == 1) {
        return(1)
    }
    return(choose(sum(params), params[length(params)]) * multinom(params[-length(params)]))
}



get_block_rewards <- function(n, l) {
    # a column in the matrix computed by get_block_rewards
    rewards <- matrix(0, n)
    for (p in equivalence_classes(n, size=l)) {
        counts <- ton_counts(p, n)
        rewards = rewards + counts * multinom(counts)
    }

    rewards <- rewards[-length(rewards)]
    if (sum(rewards) > 0) {
        scale <- l/sum(rewards)
        rewards <- rewards * scale
    }
    rewards <- c(0, rewards)
    return(matrix(rewards))
}

args = commandArgs(trailingOnly=TRUE)
hdf5_file_name <- args[1]
sample_size <- as.numeric(args[2])
reward_fun <- get_block_rewards
graph <- block_coalescent(sample_size)
tensor_dims <- c(sample_size, vertices_length(graph))    
reward_list <- lapply(1:vertices_length(graph), function(i) do.call(reward_fun, as.list(c(sample_size, vertex_at(graph, i)$state))))
rewards <- array(unlist(reward_list), dim=tensor_dims)
if (!file.exists(hdf5_file_name)) {
    h5createFile(hdf5_file_name)
    h5createGroup(hdf5_file_name, "coalescent")
}
name <- paste("coalescent/", sample_size, sep="")

for (i in 1:10) {
    tryCatch(
        {
        h5f <- H5Fopen(hdf5_file_name)
        h5write(rewards, h5f, name=paste("coalescent/", sample_size, sep=""))
        },
        error=function(e) {
            message('Error - retrying in 10')
            print(e)
            Sys.sleep(10)
            next
        },
        finally = {
            H5Fclose(h5f)
            break
        }
    )
}



                          
