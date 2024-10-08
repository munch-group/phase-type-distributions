#include <Rcpp.h>
#include "stdint.h"
#include "stdlib.h"

#include "./../../../PtDAlgorithms/api/c/ptdalgorithms.h"
#include "./../../../PtDAlgorithms/api/cpp/ptdalgorithmscpp.h"

#include "./../../../PtDAlgorithms/src/c/ptdalgorithms.c"

using namespace Rcpp;
using namespace ptdalgorithms;

static inline int get_matrix_index(int i, int j, int population, int matrix_size) {
    return (matrix_size * matrix_size * population) + i + j * matrix_size;
}


// // [[Rcpp::export]]
// int matrix_index_2_locus(int i, int j, int population, int locus, int n1, int n2) {
//   const int matrix_size = (n1 > n2 ? n1 : n2) + 1;
//   return (matrix_size * matrix_size * (population + locus)) + i + j * matrix_size;
// }

// [[Rcpp::export]]
int matrix_index(int i, int j, int population, int n1, int n2) {
  const int matrix_size = (n1 > n2 ? n1 : n2) + 1;
  return (matrix_size * matrix_size * population) + i + j * matrix_size;
}

// [[Rcpp::export]]
NumericVector start_prob_from_im(SEXP a_phase_type_graph, SEXP im_phase_type_graph, NumericVector im_stopping) {
    Rcpp::XPtr<Graph> a_graph(a_phase_type_graph);
    Rcpp::XPtr<Graph> im_graph(im_phase_type_graph);
    NumericVector result(a_graph->c_graph()->vertices_length);
    size_t state_length = im_graph->state_length();
    struct ptd_avl_tree *avl_tree = a_graph->c_avl_tree();
    
    for (int k = 1; k < (int)im_graph->c_graph()->vertices_length; ++k) {
      int *a_state = (int*) calloc(state_length, sizeof(int));
      int *im_state = im_graph->c_graph()->vertices[k]->state;
      
      for (int i = 0; i < (int)state_length; ++i) {
        if (i >= (int)state_length / 2) {
          a_state[i - state_length / 2] += im_state[i];
        } else {
          a_state[i] += im_state[i];
        }
      }
      
      struct ptd_vertex *vertex = (struct ptd_vertex*) ptd_avl_tree_find(avl_tree, a_state)->entry;
      result[vertex->index] += im_stopping[k];
      free(a_state);
    }
    
    return result;
}

// [[Rcpp::export]]
IntegerVector rewards_at(SEXP phase_type_graph, int i, int j, int n1, int n2) {
    Rcpp::XPtr<Graph> graph(phase_type_graph);

    const int matrix_size = (n1 > n2 ? n1 : n2) + 1;

    IntegerVector res(graph->c_graph()->vertices_length);

    for (int k = 0; k < (int)graph->c_graph()->vertices_length; ++k) {
        int pop1 = graph->c_graph()->vertices[k]->state[get_matrix_index(i, j, 0, matrix_size)];
        int pop2 = graph->c_graph()->vertices[k]->state[get_matrix_index(i, j, 1, matrix_size)];
        
        res[k] = pop1 + pop2;
    }

    return res;
}

// [[Rcpp::export]]
SEXP construct_ancestral_graph(int n1, int n2, float pop_size) {
    const int matrix_size = (n1 > n2 ? n1 : n2) + 1;
    const int state_length = matrix_size * matrix_size * 2;
    const size_t state_size = sizeof(int) * state_length;

    struct ptd_graph *ancestral_graph = ptd_graph_create((size_t) state_length);
    struct ptd_avl_tree *avl_tree = ptd_avl_tree_create((size_t) state_length);

    int *initial_state = (int *) calloc((size_t) state_length, sizeof(int));
    initial_state[get_matrix_index(1, 0, 0, matrix_size)] = n1;
    initial_state[get_matrix_index(0, 1, 0, matrix_size)] = n2;

    ptd_graph_add_edge(
            ancestral_graph->starting_vertex,
            ptd_find_or_create_vertex(ancestral_graph, avl_tree, initial_state),
            1
    );
    free(initial_state);
    int *child_state = (int *) malloc(state_size);

    for (size_t index = 1; index < ancestral_graph->vertices_length; ++index) {
        struct ptd_vertex *vertex = ancestral_graph->vertices[index];
        int *state = vertex->state;

        int lineages_left = 0;

        for (int i = 0; i < state_length; ++i) {
            lineages_left += state[i];
        }

        if (lineages_left == 0 || lineages_left == 1) {
            // Only one lineage left, absorb
            continue;
        }

        for (int i = 0; i < matrix_size; ++i) {
            for (int j = 0; j < matrix_size; ++j) {
                int index1 = get_matrix_index(i, j, 0, matrix_size);
                int entry1 = state[index1];
                if (entry1 == 0) {
                    continue;
                }
                for (int i2 = 0; i2 < matrix_size; ++i2) {
                    for (int j2 = 0; j2 < matrix_size; ++j2) {
                        if (i + j * matrix_size < i2 + j2 * matrix_size) {
                            continue;
                        }
                        int index2 = get_matrix_index(i2, j2, 0, matrix_size);
                        int entry2 = state[index2];
                        if (entry2 == 0) {
                            continue;
                        }
                        double weight;
                        if (i == i2 && j == j2) {
                            if (entry1 == 1) {
                                continue;
                            }
                            weight = entry1 * (entry1 - 1) / 2 / pop_size;
                        } else {
                            weight = entry1 * entry2 / pop_size;
                        }

                        memcpy(child_state, state, state_size);
                        child_state[get_matrix_index(i, j, 0, matrix_size)]--;
                        child_state[get_matrix_index(i2, j2, 0, matrix_size)]--;
                        child_state[get_matrix_index(i + i2, j + j2, 0, matrix_size)]++;
                        struct ptd_vertex *child = ptd_find_or_create_vertex(
                                ancestral_graph, avl_tree, child_state
                        );
                        ptd_graph_add_edge(vertex, child, weight);
                    }
                }
            }
        }
    }
    free(child_state);

    return Rcpp::XPtr<Graph>(
            new ptdalgorithms::Graph(ancestral_graph, avl_tree)
    );
}

// [[Rcpp::export]]
SEXP construct_im_graph(int n1, int n2, float p1_size, float p2_size, float m12, float m21) {
    const int matrix_size = (n1 > n2 ? n1 : n2) + 1;
    const int state_length = matrix_size * matrix_size * 2;
    const size_t state_size = sizeof(int) * state_length;

    struct ptd_graph *im_graph = ptd_graph_create((size_t) state_length);
    struct ptd_avl_tree *avl_tree = ptd_avl_tree_create((size_t) state_length);

    int *initial_state = (int *) calloc((size_t) state_length, sizeof(int));
    initial_state[get_matrix_index(1, 0, 0, matrix_size)] = n1;
    initial_state[get_matrix_index(0, 1, 1, matrix_size)] = n2;

    ptd_graph_add_edge(
            im_graph->starting_vertex,
            ptd_find_or_create_vertex(im_graph, avl_tree, initial_state),
            1
    );
    free(initial_state);
    int *child_state = (int *) malloc(state_size);

    for (size_t index = 1; index < im_graph->vertices_length; ++index) {
        struct ptd_vertex *vertex = im_graph->vertices[index];
        int *state = vertex->state;

        int lineages_left = 0;

        for (int i = 0; i < state_length; ++i) {
            lineages_left += state[i];
        }

        if (lineages_left == 0 || lineages_left == 1) {
            // Only one lineage left, absorb
            continue;
        }

        // Migration
        for (int i = 0; i < matrix_size; ++i) {
            for (int j = 0; j < matrix_size; ++j) {
                for (int population = 0; population <= 1; population++) {
                    int index1 = get_matrix_index(i, j, population, matrix_size);
                    int index2 = get_matrix_index(i, j, 1 - population, matrix_size);
                    if (state[index1] == 0) {
                        continue;
                    }
                    memcpy(child_state, state, state_size);
                    child_state[index1]--;
                    child_state[index2]++;
                    struct ptd_vertex *child = ptd_find_or_create_vertex(im_graph, avl_tree, child_state);
                    float migration_rate = population == 0 ? m21 : m12;
                    double weight = (child_state[index1] + 1) * migration_rate;
                    ptd_graph_add_edge(vertex, child, weight);
                    
                }
            }
        }

        for (int population = 0; population <= 1; population++) {
            float pop_size;
            if (population == 0) {
                pop_size = p1_size;
            }
            else {
                pop_size = p2_size;
            }
            for (int i = 0; i < matrix_size; ++i) {
                for (int j = 0; j < matrix_size; ++j) {
                    int index1 = get_matrix_index(i, j, population, matrix_size);
                    int entry1 = state[index1];
                    if (entry1 == 0) {
                        continue;
                    }
                    for (int i2 = 0; i2 < matrix_size; ++i2) {
                        for (int j2 = 0; j2 < matrix_size; ++j2) {
                            if (i + j * matrix_size < i2 + j2 * matrix_size) {
                                continue;
                            }
                            int index2 = get_matrix_index(i2, j2, population, matrix_size);
                            int entry2 = state[index2];
                            if (entry2 == 0) {
                                continue;
                            }
                            double weight;
                            if (i == i2 && j == j2) {
                                if (entry1 == 1) {
                                    continue;
                                }
                                weight = entry1 * (entry1 - 1) / 2 / pop_size;
                            } else {
                                weight = entry1 * entry2 / pop_size;
                            }

                            memcpy(child_state, state, state_size);
                            child_state[get_matrix_index(i, j, population, matrix_size)]--;
                            child_state[get_matrix_index(i2, j2, population, matrix_size)]--;
                            child_state[get_matrix_index(i + i2, j + j2, population, matrix_size)]++;
                            struct ptd_vertex *child = ptd_find_or_create_vertex(
                                    im_graph, avl_tree, child_state
                            );
                            ptd_graph_add_edge(vertex, child, weight);
                        }
                    }
                }
            }
        }
    }
    free(child_state);

    size_t nedges = 0;

    for (size_t k = 0; k < im_graph->vertices_length; ++k) {
        nedges += im_graph->vertices[k]->edges_length;
    }

    return Rcpp::XPtr<Graph>(
            new ptdalgorithms::Graph(im_graph, avl_tree)
    );
}
