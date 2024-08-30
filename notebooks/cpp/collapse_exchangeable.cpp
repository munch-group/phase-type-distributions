/*
 * Clone or download the code, and include these files in the repository!
 * Make SURE that the version of the downloaded code is the same as the
 * installed R library!! Otherwise it may crash randomly.
 *
 * The path is currently ../ as we are in the same repository. This path
 * should be something like [full or relative path to cloned code]/api...
 */
#include "./../../../PtDAlgorithms/api/c/ptdalgorithms.h"

/*
* Including a .c file is very strange usually!
* But the way Rcpp::sourceCpp links is different from what
* you would usually expect. Therefore this is by far
* the easiest way of importing the code.
*/
#include "./../../../PtDAlgorithms/api/cpp/ptdalgorithmscpp.h"
#include "./../../../PtDAlgorithms/src/c/ptdalgorithms.c"

/* This is the binding layer such that R can invoke this function */
#include <Rcpp.h> 

/* Basic C libraries */
#include "stdint.h"
#include "stdlib.h"
// #include "assert.h"

using namespace std;
using namespace ptdalgorithms;
using namespace Rcpp; 


// void print_state(int *state, int l) {
//       sum_of_elems += n;
//     for (int i=0; i<l; i++) {
//          fprintf(stderr, "%d ", state[i]);
//     }
//     fprintf(stderr, "\n");
// }

// std::vector<int> dummy_state_mapping_callback(std::vector<int> state) {

//     int bl_state_len = 1;

//     int total;
//     for (auto& n : state)
//         total += n;

//     std::vector<int> bl_state;
//     bl_state.push_back(total);

//     return bl_state;
// }
int *dummy_callback_fun(int *state, int n) {

    int bl_state_length = 1; // HARD CODED
    
    int *bl_state = (int *) calloc(1, sizeof(int));

    for (size_t i=0; i < n; ++i) {
        bl_state[0] += state[i];
    }
    return bl_state;
}


    // // array to vector
    // std::vector<int> v;
    // for (int i = 0; i < N; i++) 
    //     v.push_back(arr[i]); 

    // // vector to array
    // int *arr = (int) calloc(n, sizeof(int));
    // std::copy(v.begin(), v.end(), arr);
        
// [[Rcpp::export]]
SEXP collapse_exchangeable(SEXP graph, Function callback_fun) {
    Rcpp::XPtr<Graph> graphcpp(graph);
    struct ptd_graph *ptd_graph = graphcpp->c_graph();
    struct ptd_avl_tree *avl_tree = graphcpp->c_avl_tree();
    int state_vector_length = ptd_graph->state_length;
    
    // get state vector size for bl_graph
    int bl_state_vector_length = 1;  // HARD CODED FOR NOW
    // /////////////
    // std::vector<int> probe_state;
    // for (int i = 0; i < ptd_graph->state_length; i++) 
    //     probe_state.push_back(ptd_graph->vertices[0]->state[i]); 
    
    // IntegerVector dummy = callback_fun(probe_state);
    // for (auto i: dummy)
    //     std::cout << i << ' ';
    // std::cout << std::endl;

    // // std::vector<int> dummy = dummy_state_mapping_callback(probe_state);
    // int bl_state_vector_length = dummy.size();
    // /////////////
        
    // create bl_graph
    struct ptd_graph *bl_ptd_graph = ptd_graph_create(bl_state_vector_length);
    struct ptd_avl_tree *bl_avl_tree = ptd_avl_tree_create(bl_state_vector_length);
        
    // lists of rewards
    std::vector<double *> reward_list;

    // buffers
    // int *state = (int *) calloc(state_vector_length, sizeof(int));
    int *bl_state = (int *) calloc(bl_state_vector_length, sizeof(int));

    for (size_t i=0; i < ptd_graph->vertices_length; ++i) {

        // get vertex
        struct ptd_vertex *vertex = ptd_graph->vertices[i];

        // get corresponding bl_vertex
        struct ptd_vertex *bl_vertex;
        if (i == 0) {
            bl_vertex = bl_ptd_graph->vertices[0];
        } else {
            // std::vector<int> vec = callback_fun(vertex->state);

            int *bl_state = dummy_callback_fun(vertex->state, state_vector_length); // free this
            
            // std::vector<int> state_vec;
            // for (int i = 0; i < ptd_graph->state_length; i++) 
            //     state_vec.push_back(vertex->state[i]); 
            // std::vector<int> vec = dummy_state_mapping_callback(state_vec);            
            // std::copy(vec.begin(), vec.end(), bl_state);
            
            bl_vertex = ptd_find_or_create_vertex(bl_ptd_graph, bl_avl_tree, bl_state);
        }

        for (size_t j = 0; j < vertex->edges_length; ++j) {

            // get child state
            int *child_state = vertex->edges[j]->to->state;

            int *bl_child_state = dummy_callback_fun(child_state, state_vector_length); // free this
            // std::vector<int> state_vec;
            // for (int i = 0; i < ptd_graph->state_length; i++) 
            //     state_vec.push_back(child_state[i]); 
            // std::vector<int> vec = dummy_state_mapping_callback(state_vec);
            // int *bl_child_state = (int *) calloc(bl_state_vector_length, sizeof(int));
            // std::copy(vec.begin(), vec.end(), bl_child_state);

            // get bl_child_vertex
            struct ptd_vertex *bl_child_vertex = ptd_find_or_create_vertex(bl_ptd_graph, bl_avl_tree, bl_child_state);

            // // make sure list is long enough
            // for (size_t k=0; k < ptd_graph->vertices_length; ++k) {
            //     if (reward_list.size() >= bl_child_vertex->index) {
            //         break;
            //     }
            //     double *arr = (double *) calloc(state_vector_length, sizeof(double));
            //     reward_list.push_back(arr);
            // }

            // // compute added reward
            // for (int k; k < state_vector_length; ++k) {
            //     reward_list[bl_child_vertex->index][k] += child_state[k] * vertex->edges[j]->weight;
            // }
            
            // // if bl_edge exists, add edge->weight to that edge
            // int bl_edge_index = -1;
            // for (size_t k = 0; k < bl_vertex->edges_length; ++k) {
            //     if (bl_vertex->edges[k]->to->index == bl_child_vertex->index) {
            //         bl_edge_index = k;
            //         break;
            //     }
            // }
            // if (bl_edge_index > -1) {
            //     // if so, add edge->weight to that edge
            //     bl_vertex->edges[bl_edge_index]->weight += vertex->edges[j]->weight;
            // } else {
            //     // else add the edge
            //     ptd_graph_add_edge(bl_vertex, bl_child_vertex, vertex->edges[j]->weight);  
            // }
        }
    }
    
    NumericMatrix dummy_rewards( 2 , 3 );
        
    List L = List::create(
             Rcpp::XPtr<Graph>(
                new Graph(bl_ptd_graph, bl_avl_tree)
             ),
            dummy_rewards // numeric vector rewards
        );
    // Setting element names
    L.attr("names") = CharacterVector::create("graph", "rewards");
            
    return L;
    
}
        