// avltree.c
#include "avltree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int avl_height(AVLNode* node) {
    return node ? node->height : 0;
}


static int max(int a, int b) {
    return (a > b) ? a : b;
}


int avl_get_balance(AVLNode* node) {
    return node ? avl_height(node->left) - avl_height(node->right) : 0;
}

AVLNode* avl_create_node(const char* name, const char* type, 
                         const char* scope, int line_number) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    if (!node) {
        fprintf(stderr, "Error: Failed to allocate AVL node\n");
        return NULL;
    }
    
    strncpy(node->name, name, MAX_NAME - 1);
    node->name[MAX_NAME - 1] = '\0';
    
    strncpy(node->type, type, MAX_TYPE - 1);
    node->type[MAX_TYPE - 1] = '\0';
    
    strncpy(node->scope, scope, MAX_SCOPE - 1);
    node->scope[MAX_SCOPE - 1] = '\0';
    
    node->line_number = line_number;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    
    return node;
}


AVLNode* avl_right_rotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    
    x->right = y;
    y->left = T2;
    
    y->height = max(avl_height(y->left), avl_height(y->right)) + 1;
    x->height = max(avl_height(x->left), avl_height(x->right)) + 1;
    
    return x;
}


AVLNode* avl_left_rotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    
    y->left = x;
    x->right = T2;
    
    x->height = max(avl_height(x->left), avl_height(x->right)) + 1;
    y->height = max(avl_height(y->left), avl_height(y->right)) + 1;
    
    return y;
}


AVLNode* avl_insert(AVLNode* root, const char* name, const char* type,
                    const char* scope, int line_number) {
    if (!root) {
        return avl_create_node(name, type, scope, line_number);
    }
    
    int cmp = strcmp(name, root->name);
    
    if (cmp < 0) {
        root->left = avl_insert(root->left, name, type, scope, line_number);
    } else if (cmp > 0) {
        root->right = avl_insert(root->right, name, type, scope, line_number);
    } else {
        strncpy(root->type, type, MAX_TYPE - 1);
        strncpy(root->scope, scope, MAX_SCOPE - 1);
        root->line_number = line_number;
        return root;
    }
    
    root->height = 1 + max(avl_height(root->left), avl_height(root->right));
    int balance = avl_get_balance(root);
    
    // Left-Left Case
    if (balance > 1 && strcmp(name, root->left->name) < 0) {
        return avl_right_rotate(root);
    }
    
    // Right-Right Case
    if (balance < -1 && strcmp(name, root->right->name) > 0) {
        return avl_left_rotate(root);
    }
    
    // Left-Right Case
    if (balance > 1 && strcmp(name, root->left->name) > 0) {
        root->left = avl_left_rotate(root->left);
        return avl_right_rotate(root);
    }
    
    // Right-Left Case
    if (balance < -1 && strcmp(name, root->right->name) < 0) {
        root->right = avl_right_rotate(root->right);
        return avl_left_rotate(root);
    }
    
    return root;
}


AVLNode* avl_find_min(AVLNode* root) {
    while (root && root->left) {
        root = root->left;
    }
    return root;
}


AVLNode* avl_find_max(AVLNode* root) {
    while (root && root->right) {
        root = root->right;
    }
    return root;
}

AVLNode* avl_delete(AVLNode* root, const char* name) {
    if (!root) {
        return root;
    }
    
    int cmp = strcmp(name, root->name);
    
    if (cmp < 0) {
        root->left = avl_delete(root->left, name);
    } else if (cmp > 0) {
        root->right = avl_delete(root->right, name);
    } else {
        if (!root->left || !root->right) {
            AVLNode* temp = root->left ? root->left : root->right;
            
            if (!temp) {
                temp = root;
                root = NULL;
            } else {
                *root = *temp;
            }
            free(temp);
        } else {
            AVLNode* temp = avl_find_min(root->right);
            
            strcpy(root->name, temp->name);
            strcpy(root->type, temp->type);
            strcpy(root->scope, temp->scope);
            root->line_number = temp->line_number;
            
            root->right = avl_delete(root->right, temp->name);
        }
    }
    
    if (!root) {
        return root;
    }
    
    root->height = 1 + max(avl_height(root->left), avl_height(root->right));
    int balance = avl_get_balance(root);
    
    if (balance > 1 && avl_get_balance(root->left) >= 0) {
        return avl_right_rotate(root);
    }
    
    if (balance > 1 && avl_get_balance(root->left) < 0) {
        root->left = avl_left_rotate(root->left);
        return avl_right_rotate(root);
    }
    
    if (balance < -1 && avl_get_balance(root->right) <= 0) {
        return avl_left_rotate(root);
    }
    
    if (balance < -1 && avl_get_balance(root->right) > 0) {
        root->right = avl_right_rotate(root->right);
        return avl_left_rotate(root);
    }
    
    return root;
}


AVLNode* avl_search(AVLNode* root, const char* name) {
    if (!root || strcmp(root->name, name) == 0) {
        return root;
    }
    
    if (strcmp(name, root->name) < 0) {
        return avl_search(root->left, name);
    }
    
    return avl_search(root->right, name);
}


int avl_exists(AVLNode* root, const char* name) {
    return avl_search(root, name) != NULL;
}


void avl_inorder(AVLNode* root) {
    if (root) {
        avl_inorder(root->left);
        printf("%-20s | %-15s | %-15s | Line %d\n",
               root->name, root->type, root->scope, root->line_number);
        avl_inorder(root->right);
    }
}


void avl_preorder(AVLNode* root) {
    if (root) {
        printf("%-20s | %-15s | %-15s | Line %d\n",
               root->name, root->type, root->scope, root->line_number);
        avl_preorder(root->left);
        avl_preorder(root->right);
    }
}


void avl_postorder(AVLNode* root) {
    if (root) {
        avl_postorder(root->left);
        avl_postorder(root->right);
        printf("%-20s | %-15s | %-15s | Line %d\n",
               root->name, root->type, root->scope, root->line_number);
    }
}


void avl_display_sorted(AVLNode* root) {
    if (!root) {
        printf("AVL tree is empty.\n");
        return;
    }
    
    printf("\n");
    printf("┌──────────────────────────────────────────────────────────────────────┐\n");
    printf("│              IDENTIFIERS (Sorted Alphabetically)                     │\n");
    printf("├──────────────────────────────────────────────────────────────────────┤\n");
    printf("  %-20s | %-15s | %-15s | %s\n", "Name", "Type", "Scope", "Line");
    printf("  ──────────────────────────────────────────────────────────────────────\n");
    
    avl_inorder(root);
    
    printf("└──────────────────────────────────────────────────────────────────────┘\n");
    printf("Total: %d identifiers\n\n", avl_count_nodes(root));
}


void avl_display_by_scope(AVLNode* root, const char* scope) {
    if (!root) return;
    
    avl_display_by_scope(root->left, scope);
    
    if (strcmp(root->scope, scope) == 0) {
        printf("  %-20s | %-15s | Line %d\n",
               root->name, root->type, root->line_number);
    }
    
    avl_display_by_scope(root->right, scope);
}


void avl_display_by_type(AVLNode* root, const char* type) {
    if (!root) return;
    
    avl_display_by_type(root->left, type);
    
    if (strcmp(root->type, type) == 0) {
        printf("  %-20s | %-15s | Line %d\n",
               root->name, root->scope, root->line_number);
    }
    
    avl_display_by_type(root->right, type);
}


void avl_find_range(AVLNode* root, int start_line, int end_line) {
    if (!root) return;
    
    avl_find_range(root->left, start_line, end_line);
    
    if (root->line_number >= start_line && root->line_number <= end_line) {
        printf("  %-20s | %-15s | %-15s | Line %d\n",
               root->name, root->type, root->scope, root->line_number);
    }
    
    avl_find_range(root->right, start_line, end_line);
}


int avl_count_nodes(AVLNode* root) {
    if (!root) return 0;
    return 1 + avl_count_nodes(root->left) + avl_count_nodes(root->right);
}


static void collect_inorder(AVLNode* root, SymbolInfo results[], int* count, int max_results) {
    if (!root || *count >= max_results) return;
    
    collect_inorder(root->left, results, count, max_results);
    
    if (*count < max_results) {
        strncpy(results[*count].name, root->name, MAX_NAME - 1);
        strncpy(results[*count].type, root->type, MAX_TYPE - 1);
        strncpy(results[*count].category, root->scope, MAX_CATEGORY - 1);
        results[*count].line = root->line_number;
        (*count)++;
    }
    
    collect_inorder(root->right, results, count, max_results);
}

void avl_collect_all(AVLNode* root, SymbolInfo results[], int* count, int max_results) {
    *count = 0;
    collect_inorder(root, results, count, max_results);
}


int avl_is_balanced(AVLNode* root) {
    if (!root) return 1;
    
    int balance = avl_get_balance(root);
    
    if (balance < -1 || balance > 1) return 0;
    
    return avl_is_balanced(root->left) && avl_is_balanced(root->right);
}


void avl_print_stats(AVLNode* root) {
    if (!root) {
        printf("AVL tree is empty.\n");
        return;
    }
    
    int total_nodes = avl_count_nodes(root);
    int tree_height = avl_height(root);
    int is_balanced = avl_is_balanced(root);
    size_t memory = total_nodes * sizeof(AVLNode);
    
    printf("\n┌──────────────────────────────────────────────────────────────┐\n");
    printf("│                 AVL TREE STATISTICS                    │\n");
    printf("├──────────────────────────────────────────────────────────────┤\n");
    printf("│ Total nodes:            %6d                       │\n", total_nodes);
    printf("│ Tree height:            %6d                       │\n", tree_height);
    printf("│ Balanced:               %6s                       │\n", is_balanced ? "Yes" : "No");
    printf("│ Memory usage:           %6zu bytes (%.2f KB)      │\n",
           memory, memory / 1024.0);
    printf("│ Avg bytes per node:     %6.2f                       │\n",
           (double)memory / total_nodes);
    printf("└──────────────────────────────────────────────────────────────┘\n\n");
}

/**
 * Print tree structure
 */
void avl_print_tree_structure(AVLNode* root, int space) {
    if (!root) return;
    
    space += 5;
    
    avl_print_tree_structure(root->right, space);
    
    printf("\n");
    for (int i = 5; i < space; i++) {
        printf(" ");
    }
    printf("%s[%d]\n", root->name, avl_get_balance(root));
    
    avl_print_tree_structure(root->left, space);
}


void avl_display_tree(AVLNode* root) {
    if (!root) {
        printf("AVL tree is empty.\n");
        return;
    }
    
    printf("\n");
    printf("┌──────────────────────────────────────────────────────────────────────┐\n");
    printf("│                    AVL TREE STRUCTURE                                │\n");
    printf("│              (Numbers in brackets show balance factor)               │\n");
    printf("└──────────────────────────────────────────────────────────────────────┘\n");
    
    avl_print_tree_structure(root, 0);
    printf("\n");
}


void avl_destroy(AVLNode* root) {
    if (root) {
        avl_destroy(root->left);
        avl_destroy(root->right);
        free(root);
    }
}