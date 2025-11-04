// graph.h
#ifndef GRAPH_H
#define GRAPH_H
#include "utils.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_FUNCTIONS 200
#define COUPLING_THRESHOLD 5
typedef struct AdjListNode {
    int dest_index;                 
    int call_count;                 
    struct AdjListNode* next;      
} AdjListNode;
typedef struct GraphNode {
    char function_name[MAX_NAME];  
    char return_type[MAX_TYPE];     
    int line_number;                
    int is_recursive;               
    // Call relationship metadata
    AdjListNode* adj_list;         
    int out_degree;                 
    int in_degree;                  
    // Traversal metadata
    int visited;                    
    int discovery_time;              
    int finish_time;                
    int in_stack;                  
    // Analysis metadata
    int depth_level;               
    int is_dead_code;               
    int is_highly_coupled;          
} GraphNode;
typedef struct FunctionGraph {
    GraphNode* nodes;               // Array of function nodes
    int node_count;                 // Current number of functions
    int capacity;                   // Maximum capacity
    int edge_count;                 // Total number of function calls
    int has_cycles;                 // 1 if graph contains recursive cycles
} FunctionGraph;
typedef struct CallPath {
    int* path;                      // Array of function indices
    int length;                     // Length of the path
} CallPath;
FunctionGraph* graph_create(int capacity);
void graph_destroy(FunctionGraph* graph);
int graph_add_function(FunctionGraph* graph, const char* name, 
                       const char* return_type, int line);

int graph_add_call(FunctionGraph* graph, const char* caller, const char* callee);
int graph_find_function(FunctionGraph* graph, const char* name);
GraphNode* graph_get_node(FunctionGraph* graph, int index);
void graph_dfs(FunctionGraph* graph, int start_index, void (*visit)(GraphNode*));
void graph_bfs(FunctionGraph* graph, int start_index, void (*visit)(GraphNode*));
void graph_print_hierarchy(FunctionGraph* graph, const char* root_func);
void graph_print_hierarchy_depth(FunctionGraph* graph, const char* root_func, int max_depth);
void graph_reset_visited(FunctionGraph* graph);
void graph_detect_dead_code(FunctionGraph* graph);
void graph_detect_high_coupling(FunctionGraph* graph, int threshold);
void graph_find_most_called(FunctionGraph* graph, int top_n);
void graph_detect_recursion(FunctionGraph* graph);
void graph_detect_cycles(FunctionGraph* graph);
void graph_calculate_depth_levels(FunctionGraph* graph, const char* root_func);
void graph_generate_report(FunctionGraph* graph);
void graph_print_stats(FunctionGraph* graph);
void graph_display_all_functions(FunctionGraph* graph);
void graph_display_by_popularity(FunctionGraph* graph);
void graph_export_dot(FunctionGraph* graph, const char* filename);
void graph_export_csv(FunctionGraph* graph, const char* filename);
void graph_print_adjacency_list(FunctionGraph* graph);
int graph_is_empty(FunctionGraph* graph);
int graph_get_edge_count(FunctionGraph* graph);
int graph_has_call(FunctionGraph* graph, const char* caller, const char* callee);
int graph_get_called_functions(FunctionGraph* graph, const char* function_name,
                                char results[][MAX_NAME], int max_results);

int graph_get_calling_functions(FunctionGraph* graph, const char* function_name,
                                 char results[][MAX_NAME], int max_results);

#endif