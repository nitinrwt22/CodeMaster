/* graph.c - Function Call Graph Implementation */
#include "graph.h"

/* ============================================================================
   INTERNAL HELPER FUNCTIONS
   ============================================================================ */

/**
 * Creates a new adjacency list node
 */
static AdjListNode* create_adj_node(int dest_index, int line) {
    AdjListNode* node = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (!node) {
        fprintf(stderr, "Error: Failed to allocate adjacency node\n");
        return NULL;
    }
    node->dest_index = dest_index;
    node->call_count = 1;
    node->call_line = line;
    node->next = NULL;
    return node;
}

/**
 * Adds an edge to adjacency list (or increments call count if exists)
 */
static int add_to_adj_list(GraphNode* node, int dest_index, int line) {
    /* Check if edge already exists */
    AdjListNode* curr = node->adj_list;
    while (curr) {
        if (curr->dest_index == dest_index) {
            curr->call_count++;
            return 1;
        }
        curr = curr->next;
    }
    
    /* Create new edge */
    AdjListNode* new_node = create_adj_node(dest_index, line);
    if (!new_node) return 0;
    
    new_node->next = node->adj_list;
    node->adj_list = new_node;
    node->out_degree++;
    
    return 1;
}

/**
 * Comparison function for sorting by in_degree (most called first)
 */
static int compare_by_popularity(const void* a, const void* b) {
    GraphNode* na = (GraphNode*)a;
    GraphNode* nb = (GraphNode*)b;
    return nb->in_degree - na->in_degree;
}

/* ============================================================================
   CORE GRAPH OPERATIONS
   ============================================================================ */

FunctionGraph* graph_create(int capacity) {
    if (capacity <= 0) capacity = MAX_FUNCTIONS;
    
    FunctionGraph* graph = (FunctionGraph*)malloc(sizeof(FunctionGraph));
    if (!graph) {
        fprintf(stderr, "Error: Failed to allocate graph\n");
        return NULL;
    }
    
    graph->nodes = (GraphNode*)calloc(capacity, sizeof(GraphNode));
    if (!graph->nodes) {
        fprintf(stderr, "Error: Failed to allocate graph nodes\n");
        free(graph);
        return NULL;
    }
    
    graph->node_count = 0;
    graph->capacity = capacity;
    graph->edge_count = 0;
    graph->has_cycles = 0;
    
    return graph;
}

void graph_destroy(FunctionGraph* graph) {
    if (!graph) return;
    
    /* Free all adjacency lists */
    for (int i = 0; i < graph->node_count; i++) {
        AdjListNode* curr = graph->nodes[i].adj_list;
        while (curr) {
            AdjListNode* temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
    
    free(graph->nodes);
    free(graph);
}

int graph_find_function(FunctionGraph* graph, const char* name) {
    if (!graph || !name) return -1;
    
    for (int i = 0; i < graph->node_count; i++) {
        if (strcmp(graph->nodes[i].function_name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int graph_add_function(FunctionGraph* graph, const char* name, 
                       const char* return_type, int line) {
    if (!graph || !name) return -1;
    
    /* Check if function already exists */
    int existing = graph_find_function(graph, name);
    if (existing != -1) return existing;
    
    /* Check capacity */
    if (graph->node_count >= graph->capacity) {
        fprintf(stderr, "Error: Graph capacity exceeded\n");
        return -1;
    }
    
    /* Add new function */
    GraphNode* node = &graph->nodes[graph->node_count];
    strncpy(node->function_name, name, MAX_NAME - 1);
    node->function_name[MAX_NAME - 1] = '\0';
    
    if (return_type) {
        strncpy(node->return_type, return_type, MAX_TYPE - 1);
        node->return_type[MAX_TYPE - 1] = '\0';
    } else {
        strcpy(node->return_type, "unknown");
    }
    
    node->line_number = line;
    node->adj_list = NULL;
    node->out_degree = 0;
    node->in_degree = 0;
    node->visited = 0;
    node->is_recursive = 0;
    node->is_dead_code = 0;
    node->is_highly_coupled = 0;
    node->depth_level = -1;
    node->discovery_time = 0;
    node->finish_time = 0;
    node->in_stack = 0;
    
    return graph->node_count++;
}

int graph_add_call(FunctionGraph* graph, const char* caller, const char* callee, int line) {
    if (!graph || !caller || !callee) return 0;
    
    int caller_idx = graph_find_function(graph, caller);
    int callee_idx = graph_find_function(graph, callee);
    
    if (caller_idx == -1 || callee_idx == -1) {
        return 0;
    }
    
    /* Check for self-recursion */
    if (caller_idx == callee_idx) {
        graph->nodes[caller_idx].is_recursive = 1;
    }
    
    /* Add edge */
    if (add_to_adj_list(&graph->nodes[caller_idx], callee_idx, line)) {
        graph->nodes[callee_idx].in_degree++;
        graph->edge_count++;
        return 1;
    }
    
    return 0;
}

GraphNode* graph_get_node(FunctionGraph* graph, int index) {
    if (!graph || index < 0 || index >= graph->node_count) {
        return NULL;
    }
    return &graph->nodes[index];
}

/* ============================================================================
   GRAPH TRAVERSAL OPERATIONS
   ============================================================================ */

void graph_reset_visited(FunctionGraph* graph) {
    if (!graph) return;
    for (int i = 0; i < graph->node_count; i++) {
        graph->nodes[i].visited = 0;
        graph->nodes[i].in_stack = 0;
    }
}

static void dfs_util(FunctionGraph* graph, int index, void (*visit)(GraphNode*)) {
    if (index < 0 || index >= graph->node_count) return;
    
    GraphNode* node = &graph->nodes[index];
    if (node->visited) return;
    
    node->visited = 1;
    
    if (visit) {
        visit(node);
    }
    
    /* Visit all adjacent nodes */
    AdjListNode* adj = node->adj_list;
    while (adj) {
        dfs_util(graph, adj->dest_index, visit);
        adj = adj->next;
    }
}

void graph_dfs(FunctionGraph* graph, int start_index, void (*visit)(GraphNode*)) {
    if (!graph) return;
    graph_reset_visited(graph);
    dfs_util(graph, start_index, visit);
}

void graph_bfs(FunctionGraph* graph, int start_index, void (*visit)(GraphNode*)) {
    if (!graph || start_index < 0 || start_index >= graph->node_count) return;
    
    graph_reset_visited(graph);
    
    int queue[MAX_FUNCTIONS];
    int front = 0, rear = 0;
    
    queue[rear++] = start_index;
    graph->nodes[start_index].visited = 1;
    
    while (front < rear) {
        int curr_idx = queue[front++];
        GraphNode* node = &graph->nodes[curr_idx];
        
        if (visit) {
            visit(node);
        }
        
        AdjListNode* adj = node->adj_list;
        while (adj) {
            int dest = adj->dest_index;
            if (!graph->nodes[dest].visited) {
                queue[rear++] = dest;
                graph->nodes[dest].visited = 1;
            }
            adj = adj->next;
        }
    }
}

static void print_hierarchy_util(FunctionGraph* graph, int index, int depth, 
                                  int max_depth, int* visited_in_path) {
    if (index < 0 || index >= graph->node_count) return;
    if (max_depth >= 0 && depth > max_depth) return;
    
    GraphNode* node = &graph->nodes[index];
    
    /* Print indentation */
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    /* Print function info */
    if (depth > 0) printf("└── ");
    printf("%s() [Line %d]", node->function_name, node->line_number);
    
    if (node->is_recursive) {
        printf(" [RECURSIVE]");
    }
    
    /* Check for cycle in current path */
    if (visited_in_path[index]) {
        printf(" [CYCLE DETECTED]\n");
        return;
    }
    
    printf("\n");
    
    visited_in_path[index] = 1;
    
    /* Visit children */
    AdjListNode* adj = node->adj_list;
    while (adj) {
        print_hierarchy_util(graph, adj->dest_index, depth + 1, max_depth, visited_in_path);
        adj = adj->next;
    }
    
    visited_in_path[index] = 0;
}

void graph_print_hierarchy(FunctionGraph* graph, const char* root_func) {
    graph_print_hierarchy_depth(graph, root_func, -1);
}

void graph_print_hierarchy_depth(FunctionGraph* graph, const char* root_func, int max_depth) {
    if (!graph || !root_func) return;
    
    int root_idx = graph_find_function(graph, root_func);
    if (root_idx == -1) {
        printf("Function '%s' not found in graph\n", root_func);
        return;
    }
    
    printf("\n");
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│              FUNCTION CALL HIERARCHY                           │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    
    int visited_in_path[MAX_FUNCTIONS] = {0};
    print_hierarchy_util(graph, root_idx, 0, max_depth, visited_in_path);
    printf("\n");
}

/* ============================================================================
   CODE ANALYSIS OPERATIONS
   ============================================================================ */

void graph_detect_dead_code(FunctionGraph* graph) {
    if (!graph) return;
    
    printf("\n");
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│                  DEAD CODE DETECTION                           │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    
    int found = 0;
    for (int i = 0; i < graph->node_count; i++) {
        GraphNode* node = &graph->nodes[i];
        
        /* Skip main and functions called by others */
        if (strcmp(node->function_name, "main") == 0) continue;
        
        if (node->in_degree == 0) {
            printf("⚠  Function '%s' is never called (Line %d)\n",
                   node->function_name, node->line_number);
            node->is_dead_code = 1;
            found = 1;
        }
    }
    
    if (!found) {
        printf("✓ No dead code detected - all functions are used\n");
    } else {
        printf("\nSuggestion: Remove unused functions or verify if they should be called\n");
    }
    printf("\n");
}

void graph_detect_high_coupling(FunctionGraph* graph, int threshold) {
    if (!graph) return;
    
    printf("\n");
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│                HIGH COUPLING DETECTION                         │\n");
    printf("│              (Threshold: %d function calls)                     │\n", threshold);
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    
    int found = 0;
    for (int i = 0; i < graph->node_count; i++) {
        GraphNode* node = &graph->nodes[i];
        
        if (node->out_degree > threshold) {
            printf("⚠  Function '%s' calls %d functions (high coupling)\n",
                   node->function_name, node->out_degree);
            printf("   Line %d | Calls: ", node->line_number);
            
            /* List called functions */
            AdjListNode* adj = node->adj_list;
            int count = 0;
            while (adj && count < 5) {
                if (count > 0) printf(", ");
                printf("%s()", graph->nodes[adj->dest_index].function_name);
                adj = adj->next;
                count++;
            }
            if (node->out_degree > 5) {
                printf(", ... and %d more", node->out_degree - 5);
            }
            printf("\n\n");
            
            node->is_highly_coupled = 1;
            found = 1;
        }
    }
    
    if (!found) {
        printf("✓ No high coupling detected - functions are well-modularized\n");
    } else {
        printf("Suggestion: Consider breaking down highly coupled functions\n");
    }
    printf("\n");
}

void graph_find_most_called(FunctionGraph* graph, int top_n) {
    if (!graph || graph->node_count == 0) return;
    
    printf("\n");
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│              TOP %d MOST CALLED FUNCTIONS                       │\n", top_n);
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    
    /* Create a copy and sort by in_degree */
    GraphNode* sorted = (GraphNode*)malloc(graph->node_count * sizeof(GraphNode));
    if (!sorted) return;
    
    memcpy(sorted, graph->nodes, graph->node_count * sizeof(GraphNode));
    qsort(sorted, graph->node_count, sizeof(GraphNode), compare_by_popularity);
    
    printf("%-4s %-25s %-12s %-10s\n", "Rank", "Function", "Called", "Line");
    printf("─────────────────────────────────────────────────────────────────\n");
    
    int displayed = 0;
    for (int i = 0; i < graph->node_count && displayed < top_n; i++) {
        if (sorted[i].in_degree > 0) {
            printf("%-4d %-25s %-12d %-10d\n",
                   displayed + 1,
                   sorted[i].function_name,
                   sorted[i].in_degree,
                   sorted[i].line_number);
            displayed++;
        }
    }
    
    if (displayed == 0) {
        printf("No functions are called by other functions\n");
    }
    
    printf("\n");
    free(sorted);
}

static int detect_cycle_util(FunctionGraph* graph, int index, int* rec_stack) {
    if (!rec_stack[index]) {
        rec_stack[index] = 1;
        
        AdjListNode* adj = graph->nodes[index].adj_list;
        while (adj) {
            int dest = adj->dest_index;
            
            if (!rec_stack[dest]) {
                if (detect_cycle_util(graph, dest, rec_stack)) {
                    return 1;
                }
            } else if (rec_stack[dest] == 1) {
                /* Back edge found - cycle detected */
                return 1;
            }
            
            adj = adj->next;
        }
    }
    
    rec_stack[index] = 2; /* Mark as fully processed */
    return 0;
}

void graph_detect_cycles(FunctionGraph* graph) {
    if (!graph) return;
    
    printf("\n");
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│                  CYCLE DETECTION                               │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    
    int rec_stack[MAX_FUNCTIONS] = {0};
    int cycles_found = 0;
    
    for (int i = 0; i < graph->node_count; i++) {
        if (rec_stack[i] == 0) {
            if (detect_cycle_util(graph, i, rec_stack)) {
                cycles_found = 1;
            }
        }
    }
    
    if (cycles_found) {
        printf("⚠  Cycles detected in function call graph\n");
        printf("   This may indicate:\n");
        printf("   - Recursive functions\n");
        printf("   - Mutual recursion between functions\n");
        printf("   - Complex control flow patterns\n");
        graph->has_cycles = 1;
    } else {
        printf("✓ No cycles detected - call graph is acyclic (DAG)\n");
    }
    printf("\n");
}

void graph_detect_recursion(FunctionGraph* graph) {
    if (!graph) return;
    
    printf("\n");
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│                RECURSION DETECTION                             │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    
    int found = 0;
    
    /* Direct recursion */
    printf("Direct Recursion:\n");
    for (int i = 0; i < graph->node_count; i++) {
        if (graph->nodes[i].is_recursive) {
            printf("  ✓ %s() calls itself (Line %d)\n",
                   graph->nodes[i].function_name,
                   graph->nodes[i].line_number);
            found = 1;
        }
    }
    if (!found) printf("  None detected\n");
    
    printf("\n");
}

void graph_calculate_depth_levels(FunctionGraph* graph, const char* root_func) {
    if (!graph || !root_func) return;
    
    int root_idx = graph_find_function(graph, root_func);
    if (root_idx == -1) return;
    
    /* Initialize all depths to -1 */
    for (int i = 0; i < graph->node_count; i++) {
        graph->nodes[i].depth_level = -1;
    }
    
    /* BFS to calculate depths */
    int queue[MAX_FUNCTIONS];
    int front = 0, rear = 0;
    
    queue[rear++] = root_idx;
    graph->nodes[root_idx].depth_level = 0;
    
    while (front < rear) {
        int curr = queue[front++];
        int curr_depth = graph->nodes[curr].depth_level;
        
        AdjListNode* adj = graph->nodes[curr].adj_list;
        while (adj) {
            int dest = adj->dest_index;
            if (graph->nodes[dest].depth_level == -1) {
                graph->nodes[dest].depth_level = curr_depth + 1;
                queue[rear++] = dest;
            }
            adj = adj->next;
        }
    }
}

/* ============================================================================
   REPORTING & VISUALIZATION
   ============================================================================ */

void graph_print_stats(FunctionGraph* graph) {
    if (!graph) return;
    
    printf("\n");
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│              FUNCTION CALL GRAPH STATISTICS                    │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    
    printf("Total Functions:        %d\n", graph->node_count);
    printf("Total Function Calls:   %d\n", graph->edge_count);
    printf("Has Cycles:             %s\n", graph->has_cycles ? "Yes" : "No");
    
    /* Calculate additional stats */
    int leaf_functions = 0;
    int root_functions = 0;
    
    for (int i = 0; i < graph->node_count; i++) {
        if (graph->nodes[i].out_degree == 0) leaf_functions++;
        if (graph->nodes[i].in_degree == 0) root_functions++;
    }
    
    printf("Leaf Functions:         %d (call no other functions)\n", leaf_functions);
    printf("Root Functions:         %d (never called by others)\n", root_functions);
    
    size_t memory = sizeof(FunctionGraph) + 
                    graph->capacity * sizeof(GraphNode);
    
    for (int i = 0; i < graph->node_count; i++) {
        AdjListNode* adj = graph->nodes[i].adj_list;
        while (adj) {
            memory += sizeof(AdjListNode);
            adj = adj->next;
        }
    }
    
    printf("Memory Usage:           %zu bytes (%.2f KB)\n", memory, memory / 1024.0);
    printf("\n");
}

void graph_display_all_functions(FunctionGraph* graph) {
    if (!graph || graph->node_count == 0) {
        printf("Graph is empty\n");
        return;
    }
    
    printf("\n");
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│                   ALL FUNCTIONS IN GRAPH                       │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    
    printf("%-25s %-12s %-10s %-10s %-10s\n", 
           "Function", "Return Type", "Line", "Calls", "Called By");
    printf("────────────────────────────────────────────────────────────────────\n");
    
    for (int i = 0; i < graph->node_count; i++) {
        GraphNode* node = &graph->nodes[i];
        printf("%-25s %-12s %-10d %-10d %-10d\n",
               node->function_name,
               node->return_type,
               node->line_number,
               node->out_degree,
               node->in_degree);
    }
    printf("\n");
}

void graph_display_by_popularity(FunctionGraph* graph) {
    graph_find_most_called(graph, 10);
}

void graph_generate_report(FunctionGraph* graph) {
    if (!graph) return;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║            FUNCTION CALL GRAPH ANALYSIS REPORT                ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    graph_print_stats(graph);
    graph_detect_dead_code(graph);
    graph_detect_high_coupling(graph, COUPLING_THRESHOLD);
    graph_detect_recursion(graph);
    graph_find_most_called(graph, 5);
}

void graph_export_dot(FunctionGraph* graph, const char* filename) {
    if (!graph || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Error: Cannot open %s for writing\n", filename);
        return;
    }
    
    fprintf(f, "digraph FunctionCallGraph {\n");
    fprintf(f, "  rankdir=TB;\n");
    fprintf(f, "  node [shape=box, style=rounded];\n\n");
    
    /* Write nodes */
    for (int i = 0; i < graph->node_count; i++) {
        GraphNode* node = &graph->nodes[i];
        fprintf(f, "  \"%s\" [label=\"%s()\\nLine %d\"",
                node->function_name, node->function_name, node->line_number);
        
        if (node->is_dead_code) {
            fprintf(f, ", color=red, style=\"rounded,filled\", fillcolor=lightpink");
        } else if (node->is_highly_coupled) {
            fprintf(f, ", color=orange, style=\"rounded,filled\", fillcolor=lightyellow");
        } else if (strcmp(node->function_name, "main") == 0) {
            fprintf(f, ", color=blue, style=\"rounded,filled\", fillcolor=lightblue");
        }
        
        fprintf(f, "];\n");
    }
    
    fprintf(f, "\n");
    
    /* Write edges */
    for (int i = 0; i < graph->node_count; i++) {
        GraphNode* node = &graph->nodes[i];
        AdjListNode* adj = node->adj_list;
        
        while (adj) {
            fprintf(f, "  \"%s\" -> \"%s\"",
                    node->function_name,
                    graph->nodes[adj->dest_index].function_name);
            
            if (adj->call_count > 1) {
                fprintf(f, " [label=\"%dx\"]", adj->call_count);
            }
            
            if (i == adj->dest_index) {
                fprintf(f, " [color=red, style=bold]"); /* Recursive */
            }
            
            fprintf(f, ";\n");
            adj = adj->next;
        }
    }
    
    fprintf(f, "}\n");
    fclose(f);
    
    printf("✓ Graph exported to %s\n", filename);
    printf("  Use Graphviz to visualize: dot -Tpng %s -o graph.png\n\n", filename);
}

void graph_export_csv(FunctionGraph* graph, const char* filename) {
    if (!graph || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Error: Cannot open %s for writing\n", filename);
        return;
    }
    
    fprintf(f, "Caller,Callee,CallCount,CallerLine,CallLine\n");
    
    for (int i = 0; i < graph->node_count; i++) {
        GraphNode* caller = &graph->nodes[i];
        AdjListNode* adj = caller->adj_list;
        
        while (adj) {
            GraphNode* callee = &graph->nodes[adj->dest_index];
            fprintf(f, "%s,%s,%d,%d,%d\n",
                    caller->function_name,
                    callee->function_name,
                    adj->call_count,
                    caller->line_number,
                    adj->call_line);
            adj = adj->next;
        }
    }
    
    fclose(f);
    printf("✓ Call data exported to %s\n\n", filename);
}

void graph_print_adjacency_list(FunctionGraph* graph) {
    if (!graph) return;
    
    printf("\n");
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│                  ADJACENCY LIST REPRESENTATION                 │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    
    for (int i = 0; i < graph->node_count; i++) {
        GraphNode* node = &graph->nodes[i];
        printf("%s() -> ", node->function_name);
        
        AdjListNode* adj = node->adj_list;
        if (!adj) {
            printf("(no calls)\n");
        } else {
            while (adj) {
                printf("%s()", graph->nodes[adj->dest_index].function_name);
                if (adj->call_count > 1) {
                    printf("[%dx]", adj->call_count);
                }
                if (adj->next) printf(", ");
                adj = adj->next;
            }
            printf("\n");
        }
    }
    printf("\n");
}

/* ============================================================================
   UTILITY FUNCTIONS
   ============================================================================ */

int graph_is_empty(FunctionGraph* graph) {
    return !graph || graph->node_count == 0;
}

int graph_get_edge_count(FunctionGraph* graph) {
    return graph ? graph->edge_count : 0;
}

int graph_has_call(FunctionGraph* graph, const char* caller, const char* callee) {
    if (!graph || !caller || !callee) return 0;
    
    int caller_idx = graph_find_function(graph, caller);
    if (caller_idx == -1) return 0;
    
    int callee_idx = graph_find_function(graph, callee);
    if (callee_idx == -1) return 0;
    
    AdjListNode* adj = graph->nodes[caller_idx].adj_list;
    while (adj) {
        if (adj->dest_index == callee_idx) return 1;
        adj = adj->next;
    }
    
    return 0;
}

int graph_get_called_functions(FunctionGraph* graph, const char* function_name,
                                char results[][MAX_NAME], int max_results) {
    if (!graph || !function_name || !results) return 0;
    
    int idx = graph_find_function(graph, function_name);
    if (idx == -1) return 0;
    
    int count = 0;
    AdjListNode* adj = graph->nodes[idx].adj_list;
    
    while (adj && count < max_results) {
        strncpy(results[count], graph->nodes[adj->dest_index].function_name, MAX_NAME - 1);
        results[count][MAX_NAME - 1] = '\0';
        count++;
        adj = adj->next;
    }
    
    return count;
}

int graph_get_calling_functions(FunctionGraph* graph, const char* function_name,
                                 char results[][MAX_NAME], int max_results) {
    if (!graph || !function_name || !results) return 0;
    
    int target_idx = graph_find_function(graph, function_name);
    if (target_idx == -1) return 0;
    
    int count = 0;
    
    /* Search through all functions to find who calls the target */
    for (int i = 0; i < graph->node_count && count < max_results; i++) {
        AdjListNode* adj = graph->nodes[i].adj_list;
        
        while (adj) {
            if (adj->dest_index == target_idx) {
                strncpy(results[count], graph->nodes[i].function_name, MAX_NAME - 1);
                results[count][MAX_NAME - 1] = '\0';
                count++;
                break; /* Found a caller, move to next function */
            }
            adj = adj->next;
        }
    }
    
    return count;
}