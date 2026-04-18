#include "cfg.h"

CFG* cfg_create() {
    CFG* cfg = (CFG*)malloc(sizeof(CFG));
    if (cfg) {
        cfg->head = NULL;
        cfg->node_count = 0;
    }
    return cfg;
}

CFGNode* cfg_add_node(CFG* cfg, const char* label, int line) {
    if (!cfg) return NULL;
    
    CFGNode* node = (CFGNode*)malloc(sizeof(CFGNode));
    if (!node) return NULL;
    
    node->id = ++cfg->node_count;
    strncpy(node->label, label, sizeof(node->label) - 1);
    node->label[sizeof(node->label) - 1] = '\0';
    node->line = line;
    node->edges = NULL;
    node->next = NULL;
    
    // Append to list of nodes
    if (!cfg->head) {
        cfg->head = node;
    } else {
        CFGNode* curr = cfg->head;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = node;
    }
    return node;
}

void cfg_add_edge(CFGNode* source, CFGNode* target, const char* edge_label) {
    if (!source || !target) return;
    
    CFGEdge* edge = (CFGEdge*)malloc(sizeof(CFGEdge));
    if (!edge) return;
    
    edge->target = target;
    if (edge_label) {
        strncpy(edge->edge_label, edge_label, sizeof(edge->edge_label) - 1);
        edge->edge_label[sizeof(edge->edge_label) - 1] = '\0';
    } else {
        edge->edge_label[0] = '\0';
    }
    
    // Add to front of edge list
    edge->next = source->edges;
    source->edges = edge;
}

void cfg_print(CFG* cfg) {
    if (!cfg || !cfg->head) {
        printf("CFG is empty.\n");
        return;
    }
    
    CFGNode* curr = cfg->head;
    while (curr) {
        printf("Node [%d] (Line %d): %s\n", curr->id, curr->line, curr->label);
        CFGEdge* edge = curr->edges;
        while (edge) {
            printf("  -> [%d] %s (Label: '%s')\n", edge->target->id, edge->target->label, edge->edge_label);
            edge = edge->next;
        }
        curr = curr->next;
    }
}

void cfg_export_dot(CFG* cfg, const char* filename) {
    if (!cfg || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "digraph CFG {\n");
    fprintf(f, "    node [shape=box, style=filled, fillcolor=lightgray];\n");
    
    CFGNode* curr = cfg->head;
    while (curr) {
        fprintf(f, "    N%d [label=\"%s\\n(Line %d)\"];\n", curr->id, curr->label, curr->line);
        curr = curr->next;
    }
    
    curr = cfg->head;
    while (curr) {
        CFGEdge* edge = curr->edges;
        while (edge) {
            fprintf(f, "    N%d -> N%d [label=\"%s\"];\n", curr->id, edge->target->id, edge->edge_label);
            edge = edge->next;
        }
        curr = curr->next;
    }
    
    fprintf(f, "}\n");
    fclose(f);
}

void cfg_destroy(CFG* cfg) {
    if (!cfg) return;
    
    CFGNode* curr = cfg->head;
    while (curr) {
        CFGNode* next_node = curr->next;
        
        CFGEdge* edge = curr->edges;
        while (edge) {
            CFGEdge* next_edge = edge->next;
            free(edge);
            edge = next_edge;
        }
        
        free(curr);
        curr = next_node;
    }
    free(cfg);
}

CFG* cfg_build_from_ast(ASTNode* ast_root) {
    if (!ast_root) return NULL;
    
    CFG* cfg = cfg_create();
    if (!cfg) return NULL;
    
    CFGNode* prev = NULL;
    CFGNode* last_if_node = NULL; // To handle else/else-if branches
    
    // Top-down simplified sequence modelling AST statements logically.
    // The current AST parses blocks generically into sequential next/children loops.
    for (int i = 0; i < ast_root->childCount; i++) {
        ASTNode* func_stmt = ast_root->children[i];
        
        char label_buf[128];
        CFGNode* seq_node = NULL;
        
        if (func_stmt->type == NODE_FUNC_DECL) {
            snprintf(label_buf, sizeof(label_buf), "FUNC_ENTRY: %s", func_stmt->value);
            seq_node = cfg_add_node(cfg, label_buf, func_stmt->lineNumber);
            if (prev) cfg_add_edge(prev, seq_node, "next");
            
            // Artificial body placeholder
            CFGNode* body_node = cfg_add_node(cfg, "Function Body", func_stmt->lineNumber);
            cfg_add_edge(seq_node, body_node, "start");
            
            // Artificial Exit
            snprintf(label_buf, sizeof(label_buf), "FUNC_EXIT: %s", func_stmt->value);
            CFGNode* exit_node = cfg_add_node(cfg, label_buf, func_stmt->lineNumber);
            cfg_add_edge(body_node, exit_node, "return");
            prev = exit_node;
            last_if_node = NULL;
            continue;
        } 
        else if (func_stmt->type == NODE_IF) {
            if (strcmp(func_stmt->value, "else") == 0 || strcmp(func_stmt->value, "else-if") == 0) {
                snprintf(label_buf, sizeof(label_buf), "ELSE: %s", func_stmt->value);
                seq_node = cfg_add_node(cfg, label_buf, func_stmt->lineNumber);
                
                if (last_if_node) {
                    cfg_add_edge(last_if_node, seq_node, "false");
                }
                
                CFGNode* else_body = cfg_add_node(cfg, "Else Body", func_stmt->lineNumber);
                cfg_add_edge(seq_node, else_body, "true");
                
                CFGNode* end_else = cfg_add_node(cfg, "END ELSE", func_stmt->lineNumber);
                cfg_add_edge(else_body, end_else, "next");
                
                prev = end_else;
            } else {
                snprintf(label_buf, sizeof(label_buf), "IF: %s", func_stmt->value);
                seq_node = cfg_add_node(cfg, label_buf, func_stmt->lineNumber);
                if (prev) cfg_add_edge(prev, seq_node, "next");
                
                CFGNode* true_branch = cfg_add_node(cfg, "True Branch", func_stmt->lineNumber);
                cfg_add_edge(seq_node, true_branch, "true");
                
                CFGNode* end_if = cfg_add_node(cfg, "END IF", func_stmt->lineNumber);
                cfg_add_edge(true_branch, end_if, "next");
                
                last_if_node = seq_node; // Save for potential else
                prev = end_if;
            }
            continue;
        }
        else if (func_stmt->type == NODE_FOR || func_stmt->type == NODE_WHILE) {
            snprintf(label_buf, sizeof(label_buf), "LOOP: %s", func_stmt->value);
            seq_node = cfg_add_node(cfg, label_buf, func_stmt->lineNumber);
            if (prev) cfg_add_edge(prev, seq_node, "next");
            
            CFGNode* loop_body = cfg_add_node(cfg, "Loop Body", func_stmt->lineNumber);
            CFGNode* loop_exit = cfg_add_node(cfg, "Loop Exit", func_stmt->lineNumber);
            cfg_add_edge(seq_node, loop_body, "true");
            cfg_add_edge(loop_body, seq_node, "repeat");
            cfg_add_edge(seq_node, loop_exit, "false");
            
            prev = loop_exit;
            last_if_node = NULL;
            continue;
        }
        else if (func_stmt->type == NODE_RETURN) {
            snprintf(label_buf, sizeof(label_buf), "RETURN: %s", func_stmt->value);
            seq_node = cfg_add_node(cfg, label_buf, func_stmt->lineNumber);
            if (prev) cfg_add_edge(prev, seq_node, "next");
            
            // Link to a generic terminate node
            CFGNode* final_exit = cfg_add_node(cfg, "TERMINATE", func_stmt->lineNumber);
            cfg_add_edge(seq_node, final_exit, "return");
            
            prev = final_exit;
            last_if_node = NULL;
            continue;
        }
        else {
            snprintf(label_buf, sizeof(label_buf), "STMT: %s", func_stmt->value);
            seq_node = cfg_add_node(cfg, label_buf, func_stmt->lineNumber);
            if (prev) cfg_add_edge(prev, seq_node, "next");
            prev = seq_node;
            last_if_node = NULL;
            continue;
        }
    }
    
    return cfg;
}
