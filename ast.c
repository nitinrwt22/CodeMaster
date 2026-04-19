// ast.c
#include "ast.h"


ASTNode* createNode(NodeType type, const char* value) {
    return createNodeWithLine(type, value, 0);
}


ASTNode* createNodeWithLine(NodeType type, const char* value, int lineNumber) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for ASTNode\n");
        exit(1);
    }
    
    node->type = type;
    strncpy(node->value, value ? value : "", sizeof(node->value) - 1);
    node->value[sizeof(node->value) - 1] = '\0';
    
    node->dataType[0] = '\0';
    node->returnType[0] = '\0';
    node->lineNumber = lineNumber;
    node->complexity = 0;
    node->nesting_depth = 0;
    
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    node->next = NULL;
    node->childCount = 0;
    
    node->paramCount = 0;
    for (int i = 0; i < MAX_PARAMS; i++) {
        node->parameters[i] = NULL;
    }
    
    node->childCapacity = 4;
    node->children = (struct ASTNode**)malloc(node->childCapacity * sizeof(struct ASTNode*));
    if (node->children) {
        for (int i = 0; i < node->childCapacity; i++) {
            node->children[i] = NULL;
        }
    }
    
    return node;
}


ASTNode* connectNodes(ASTNode* parent, ASTNode* left, ASTNode* right) {
    if (parent) {
        parent->left = left;
        parent->right = right;
        if (left) left->parent = parent;
        if (right) right->parent = parent;
    }
    return parent;
}


void addChild(ASTNode* parent, ASTNode* child) {
    if (!parent || !child) return;
    
    if (parent->childCount >= parent->childCapacity) {
        int newCap = parent->childCapacity == 0 ? 4 : parent->childCapacity * 2;
        ASTNode** newArray = (ASTNode**)realloc(parent->children, newCap * sizeof(ASTNode*));
        if (!newArray) return;
        parent->children = newArray;
        for (int i = parent->childCapacity; i < newCap; i++) {
            parent->children[i] = NULL;
        }
        parent->childCapacity = newCap;
    }
    
    parent->children[parent->childCount++] = child;
    child->parent = parent;
}


void appendStatement(ASTNode** list, ASTNode* stmt) {
    if (!list || !stmt) return;
    
    if (*list == NULL) {
        *list = stmt;
    } else {
        ASTNode* temp = *list;
        while (temp->next) temp = temp->next;
        temp->next = stmt;
    }

    if (*list != stmt && (*list)->parent && !stmt->parent) {
        stmt->parent = (*list)->parent;
    }
}


void setParentPointers(ASTNode* root, ASTNode* parent) {
    if (!root) return;
    
    root->parent = parent;
    
    if (root->left) setParentPointers(root->left, root);
    if (root->right) setParentPointers(root->right, root);
    
    for (int i = 0; i < root->childCount; i++) {
        if (root->children[i]) {
            setParentPointers(root->children[i], root);
        }
    }
    
    if (root->next) setParentPointers(root->next, parent);
}


void printAST(ASTNode* root, int level) {
    if (root == NULL) return;

    for (int i = 0; i < level; i++) printf("  ");
    printf("(%s)\n", root->value);

    if (root->left) printAST(root->left, level + 1);
    if (root->right) printAST(root->right, level + 1);
    
    for (int i = 0; i < root->childCount; i++) {
        if (root->children[i]) printAST(root->children[i], level + 1);
    }
    
    if (root->next) printAST(root->next, level);
}


void printASTDetailed(ASTNode* root, int level) {
    if (root == NULL) return;

    for (int i = 0; i < level; i++) printf("  ");
    
    printf("[%s] value='%s'", root->type == NODE_FUNC_DECL ? "FUNC" : 
                              root->type == NODE_VAR_DECL ? "VAR" :
                              root->type == NODE_IF ? "IF" :
                              root->type == NODE_FOR ? "FOR" : "EXPR", 
           root->value);
    
    if (root->lineNumber > 0) printf(" line=%d", root->lineNumber);
    if (root->dataType[0]) printf(" type='%s'", root->dataType);
    printf("\n");

    if (root->left) printASTDetailed(root->left, level + 1);
    if (root->right) printASTDetailed(root->right, level + 1);
    
    for (int i = 0; i < root->childCount; i++) {
        if (root->children[i]) printASTDetailed(root->children[i], level + 1);
    }
    
    if (root->next) printASTDetailed(root->next, level);
}


int calcComplexityHelper(ASTNode* node) {
    if (!node) return 0;
    int c = 0;
    if (node->type == NODE_IF || 
        node->type == NODE_WHILE || 
        node->type == NODE_FOR ||
        node->type == NODE_DO_WHILE) {
        c = 1;
    }
    if (node->left) c += calcComplexityHelper(node->left);
    if (node->right) c += calcComplexityHelper(node->right);
    for (int i = 0; i < node->childCount; i++) {
        if (node->children[i]) c += calcComplexityHelper(node->children[i]);
    }
    if (node->next) c += calcComplexityHelper(node->next);
    return c;
}

int calculateComplexity(ASTNode* funcNode) {
    if (!funcNode || funcNode->type != NODE_FUNC_DECL) return 0;
    
    int complexity = 1; // Base complexity
    
    // Count decision points recursively through body and children
    if (funcNode->left) complexity += calcComplexityHelper(funcNode->left);
    if (funcNode->right) complexity += calcComplexityHelper(funcNode->right);
    for (int i = 0; i < funcNode->childCount; i++) {
        if (funcNode->children[i]) complexity += calcComplexityHelper(funcNode->children[i]);
    }
    
    return complexity;
}


int calculateNestingDepth(ASTNode* node, int currentDepth) {
    if (!node) return currentDepth;
    
    int originalDepth = currentDepth;
    int newDepth = currentDepth;
    
    if (node->type == NODE_IF || 
        node->type == NODE_WHILE || 
        node->type == NODE_FOR ||
        node->type == NODE_DO_WHILE ||
        node->type == NODE_BLOCK) {
        newDepth++;
    }
    
    int maxDepth = newDepth;
    
    if (node->left) {
        int depth = calculateNestingDepth(node->left, newDepth);
        if (depth > maxDepth) maxDepth = depth;
    }
    
    if (node->right) {
        int depth = calculateNestingDepth(node->right, newDepth);
        if (depth > maxDepth) maxDepth = depth;
    }
    
    for (int i = 0; i < node->childCount; i++) {
        if (node->children[i]) {
            int depth = calculateNestingDepth(node->children[i], newDepth);
            if (depth > maxDepth) maxDepth = depth;
        }
    }
    
    if (node->next) {
        int depth = calculateNestingDepth(node->next, originalDepth); // Pass original depth to siblings!
        if (depth > maxDepth) maxDepth = depth;
    }
    
    return maxDepth;
}


int countNodes(ASTNode* root) {
    if (!root) return 0;
    
    int count = 1;
    
    if (root->left) count += countNodes(root->left);
    if (root->right) count += countNodes(root->right);
    
    for (int i = 0; i < root->childCount; i++) {
        if (root->children[i]) count += countNodes(root->children[i]);
    }
    
    if (root->next) count += countNodes(root->next);
    
    return count;
}


void findFunctionCalls(ASTNode* root, char*** calls, int* count) {
    if (!root || !calls || !count) return;
    
    if (root->type == NODE_FUNC_CALL) {
        if (*count < 100) { // Arbitrary limit
            char* call = (char*)malloc(strlen(root->value) + 1);
            strcpy(call, root->value);
            (*calls)[(*count)++] = call;
        }
    }
    
    if (root->left) findFunctionCalls(root->left, calls, count);
    if (root->right) findFunctionCalls(root->right, calls, count);
    
    for (int i = 0; i < root->childCount; i++) {
        if (root->children[i]) findFunctionCalls(root->children[i], calls, count);
    }
    
    if (root->next) findFunctionCalls(root->next, calls, count);
}


void freeAST(ASTNode* root) {
    if (root == NULL) return;
    
    freeAST(root->left);
    freeAST(root->right);
    freeAST(root->next);
    
    for (int i = 0; i < root->childCount; i++) {
        if (root->children[i]) {
            freeAST(root->children[i]);
        }
    }
    if (root->children) {
        free(root->children);
    }
    
    for (int i = 0; i < root->paramCount; i++) {
        if (root->parameters[i]) {
            free(root->parameters[i]);
        }
    }
    
    free(root);
}