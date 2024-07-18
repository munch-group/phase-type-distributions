library(devtools)
# remove.packages("ptdalgorithms")

devtools::install_github("TobiasRoikjer/PtDAlgorithms")

library(ptdalgorithms)


Rcpp::sourceCpp("./cpp/isolation_migration.cpp")


im_expectation <- function(n1, n2, m1, m2, split_t) {

    cat(n1, " ", n2, " ", m1, " ", m1, " ", split_t, " ")

    # build im graph
    im_g <- construct_im_graph(n1,n2,m1,m2)
    cat(vertices_length(im_g), " ")

    im_expected_visits <- accumulated_visiting_time(im_g, split_t)

    # create ancestral graph
    a_g <- construct_ancestral_graph(n1,n2)

    cat(vertices_length(a_g), " ")

    # find probabilities of starting at each state in ancestral graph
    start_prob <- start_prob_from_im(a_g, im_g, im_expected_visits)

    # compute expectations for each graph
    im_expectation <- matrix(nrow=n1+1,ncol=n2+1)
    a_expectation <- matrix(nrow=n1+1,ncol=n2+1)
    for (i in 0:n1) {
      for (j in 0:n2) {
        im_expectation[i+1,j+1] <- sum(im_expected_visits * rewards_at(im_g, i,j,n1,n2))
        a_expectation[i+1, j+1]<- sum(start_prob * expected_waiting_time(a_g, rewards_at(a_g, i,j,n1,n2)))
      }
    }
}

for (i in 3:10) {
    then <- proc.time()
    im_expectation(i, i, 1, 1, 2)
    now <- proc.time()
    cat((now - then)[3], "\n")
    flush.console()
}
