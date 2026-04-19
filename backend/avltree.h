// avltree.h
#ifndef AVLTREE_H
#define AVLTREE_H

#include "utils.h"

// AVL Tree node structure with metadata
typedef struct AVLNode {
    char name[MAX_NAME];        // Identifier name
    char type[MAX_TYPE];        // Type (int, float, void, etc.)
    char scope[MAX_SCOPE];      // Scope (global, function name, etc.)
    int line_number;            // Line number in source code
    int height;                 // Height of node for balancing
    struct AVLNode* left;       // Left child
    struct AVLNode* right;      // Right child
} AVLNode;

// Core AVL tree operations
AVLNode* avl_create_node(const char* name, const char* type, 
                         const char* scope, int line_number);
AVLNode* avl_insert(AVLNode* root, const char* name, const char* type,
                    const char* scope, int line_number);
AVLNode* avl_delete(AVLNode* root, const char* name);
AVLNode* avl_search(AVLNode* root, const char* name);
void avl_destroy(AVLNode* root);

// Traversal operations
void avl_inorder(AVLNode* root);
void avl_preorder(AVLNode* root);
void avl_postorder(AVLNode* root);

// Display operations with formatting
void avl_display_tree(AVLNode* root);
void avl_display_sorted(AVLNode* root);
void avl_display_by_scope(AVLNode* root, const char* scope);
void avl_display_by_type(AVLNode* root, const char* type);

// Utility operations
int avl_height(AVLNode* node);
int avl_get_balance(AVLNode* node);
int avl_count_nodes(AVLNode* root);
AVLNode* avl_find_min(AVLNode* root);
AVLNode* avl_find_max(AVLNode* root);

// Search operations
int avl_exists(AVLNode* root, const char* name);
void avl_find_range(AVLNode* root, int start_line, int end_line);
void avl_collect_all(AVLNode* root, SymbolInfo results[], int* count, int max_results);

// Statistics and visualization
void avl_print_stats(AVLNode* root);
void avl_print_tree_structure(AVLNode* root, int space);
int avl_is_balanced(AVLNode* root);

// Rotation operations (exposed for testing/debugging)
AVLNode* avl_right_rotate(AVLNode* y);
AVLNode* avl_left_rotate(AVLNode* x);

#endif // AVLTREE_H