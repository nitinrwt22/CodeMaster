#ifndef CFG_H
#define CFG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

typedef struct CFGNode CFGNode;
typedef struct CFGEdge CFGEdge;

struct CFGEdge {
    CFGNode* target;
    char edge_label[64];  // e.g., "true", "false", ""
    CFGEdge* next;
};

struct CFGNode {
    int id;
    char label[128];
    int line;
    CFGNode* next;        // Linked list of all nodes in CFG
    CFGEdge* edges;       // Outgoing edges list
};

typedef struct CFG {
    CFGNode* head;        // Head of nodes list
    int node_count;
} CFG;

// Core functions
CFG* cfg_create();
CFGNode* cfg_add_node(CFG* cfg, const char* label, int line);
void cfg_add_edge(CFGNode* source, CFGNode* target, const char* edge_label);
void cfg_print(CFG* cfg);
void cfg_export_dot(CFG* cfg, const char* filename);
void cfg_destroy(CFG* cfg);

// Integration with AST
CFG* cfg_build_from_ast(ASTNode* ast_root);

#endif // CFG_H
