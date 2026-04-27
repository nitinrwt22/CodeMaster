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
    
    node->readVarCount = 0;
    for (int i = 0; i < 10; i++) node->readVars[i] = NULL;
    
    node->writeVarCount = 0;
    for (int i = 0; i < 5; i++) node->writeVars[i] = NULL;

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
    
    // NOTE: Do NOT follow root->next here.
    // Sibling (next) pointers create a flat linked list at each depth level.
    // Following both children[] AND next would revisit the same nodes
    // exponentially on large files, causing infinite-like recursion.
    // Parents are already set by addChild() so this is safe to skip.
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
    
    if (node->type == NODE_IF) {
        if (strcmp(node->value, "if") == 0 || 
            strcmp(node->value, "else-if") == 0 || 
            strcmp(node->value, "case") == 0) {
            c = 1;
        }
    } else if (node->type == NODE_WHILE || 
               node->type == NODE_FOR ||
               node->type == NODE_DO_WHILE) {
        c = 1;
    }
    
    c += node->complexity;

    if (node->left) c += calcComplexityHelper(node->left);
    if (node->right) c += calcComplexityHelper(node->right);
    for (int i = 0; i < node->childCount; i++) {
        if (node->children[i]) c += calcComplexityHelper(node->children[i]);
    }
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
    
    return maxDepth;
}

typedef struct {
    char name[64];
    int declaredLine;
    int writeCount;
    int readCount;
    int unreadWriteLine; // 0 if no unread write
} VarInfo;

typedef struct {
    VarInfo vars[100];
    int count;
} DataFlowContext;

static void track_data_flow_recursive(ASTNode* node, DataFlowContext* ctx) {
    if (!node) return;
    
    for (int r = 0; r < node->readVarCount; r++) {
        for (int i = 0; i < ctx->count; i++) {
            if (strcmp(ctx->vars[i].name, node->readVars[r]) == 0) {
                ctx->vars[i].readCount++;
                ctx->vars[i].unreadWriteLine = 0;
            }
        }
    }
    
    for (int w = 0; w < node->writeVarCount; w++) {
        int found = 0;
        for (int i = 0; i < ctx->count; i++) {
            if (strcmp(ctx->vars[i].name, node->writeVars[w]) == 0) {
                found = 1;
                if (ctx->vars[i].unreadWriteLine != 0 && node->type == NODE_ASSIGN) {
                    printf("  │  └─ Warning: Variable '%s' overwritten before use (Line %d)\n", 
                           ctx->vars[i].name, node->lineNumber);
                }
                ctx->vars[i].writeCount++;
                ctx->vars[i].unreadWriteLine = node->lineNumber;
            }
        }
        if (!found && node->type == NODE_VAR_DECL) {
            strncpy(ctx->vars[ctx->count].name, node->writeVars[w], 63);
            ctx->vars[ctx->count].name[63] = '\0';
            ctx->vars[ctx->count].declaredLine = node->lineNumber;
            ctx->vars[ctx->count].writeCount = (node->writeVarCount > 0 ? 1 : 0);
            ctx->vars[ctx->count].readCount = 0;
            ctx->vars[ctx->count].unreadWriteLine = 0;
            ctx->count++;
        }
    }

    if (node->left) track_data_flow_recursive(node->left, ctx);
    if (node->right) track_data_flow_recursive(node->right, ctx);
    for (int i = 0; i < node->childCount; i++) {
        if (node->children[i]) track_data_flow_recursive(node->children[i], ctx);
    }
}


void run_dfa_analysis(ASTNode* funcNode, DFA_Result* result) {
    if (!funcNode || funcNode->type != NODE_FUNC_DECL || !result) return;
    
    result->count = 0;
    
    if (funcNode->left) track_data_flow_recursive(funcNode->left, (DataFlowContext*)result);
    if (funcNode->right) track_data_flow_recursive(funcNode->right, (DataFlowContext*)result);
    for (int i = 0; i < funcNode->childCount; i++) {
        if (funcNode->children[i]) track_data_flow_recursive(funcNode->children[i], (DataFlowContext*)result);
    }
}

void analyzeDataFlowFunction(ASTNode* funcNode) {
    if (!funcNode || funcNode->type != NODE_FUNC_DECL) return;
    
    DFA_Result result;
    run_dfa_analysis(funcNode, &result);
    
    printf("  ├─ Data Flow Analysis:\n");
    int issues = 0;
    for (int i = 0; i < result.count; i++) {
        if (result.vars[i].readCount == 0 && result.vars[i].writeCount == 0) {
            printf("  │  └─ Warning: Variable '%s' declared but never used (Line %d)\n", 
                   result.vars[i].name, result.vars[i].declaredLine);
            issues++;
        } else if (result.vars[i].readCount == 0 && result.vars[i].writeCount > 0) {
            printf("  │  └─ Warning: Variable '%s' declared but never read (Line %d)\n", 
                   result.vars[i].name, result.vars[i].declaredLine);
            issues++;
        }
    }
    if (issues == 0) printf("  │  └─ No unused or unread variables detected.\n");
}


int countNodes(ASTNode* root) {
    if (!root) return 0;
    
    int count = 1;
    
    if (root->left) count += countNodes(root->left);
    if (root->right) count += countNodes(root->right);
    
    for (int i = 0; i < root->childCount; i++) {
        if (root->children[i]) count += countNodes(root->children[i]);
    }
    
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
    
    for (int i = 0; i < root->readVarCount; i++) {
        if (root->readVars[i]) {
            free(root->readVars[i]);
        }
    }
    
    for (int i = 0; i < root->writeVarCount; i++) {
        if (root->writeVars[i]) {
            free(root->writeVars[i]);
        }
    }
    
    free(root);
}