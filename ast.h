// ast.h
#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHILDREN 10
#define MAX_PARAMS 20

/**
 * Enumeration of all possible AST node types
 */
typedef enum {
    NODE_PROGRAM,       // Root program node
    NODE_INT,           // Integer literal
    NODE_FLOAT,         // Floating-point literal
    NODE_ID,            // Identifier
    NODE_OP,            // Binary operator
    NODE_ASSIGN,        // Assignment operator
    NODE_IF,            // If statement
    NODE_WHILE,         // While loop
    NODE_FOR,           // For loop
    NODE_DO_WHILE,      // Do-while loop
    NODE_RETURN,        // Return statement
    NODE_FUNC_DECL,     // Function declaration
    NODE_FUNC_CALL,     // Function call
    NODE_VAR_DECL,      // Variable declaration
    NODE_BLOCK,         // Code block
    NODE_EXPR,          // Expression
    NODE_PARAM,         // Function parameter
    NODE_UNKNOWN        // Unknown node type
} NodeType;

/**
 * Abstract Syntax Tree Node Structure with enhanced metadata
 */
typedef struct ASTNode {
    NodeType type;                          // Type of this node
    char value[128];                        // Lexeme/value
    char dataType[64];                      // Data type (for declarations)
    int lineNumber;                         // Source line number
    int complexity;                         // Cyclomatic complexity counter
    int nesting_depth;                      // Nesting depth in function
    
    struct ASTNode* parent;                 // Parent node
    struct ASTNode* left;                   // Left child
    struct ASTNode* right;                  // Right child
    struct ASTNode** children;              // Array of children (dynamic)
    int childCapacity;                      // Capacity of children array
    int childCount;                         // Number of children
    struct ASTNode* next;                   // Next sibling (for statement lists)
    
    // Function-specific metadata
    char* parameters[MAX_PARAMS];           // Function parameters
    int paramCount;
    char returnType[64];                    // Return type for functions
    
} ASTNode;

/**
 * Creates a new AST node with the given type and value
 */
ASTNode* createNode(NodeType type, const char* value);

/**
 * Creates a node with line number information
 */
ASTNode* createNodeWithLine(NodeType type, const char* value, int lineNumber);

/**
 * Connects a parent node to its left and right children
 */
ASTNode* connectNodes(ASTNode* parent, ASTNode* left, ASTNode* right);

/**
 * Adds a child to a parent node
 */
void addChild(ASTNode* parent, ASTNode* child);

/**
 * Appends a statement node to a statement list
 */
void appendStatement(ASTNode** list, ASTNode* stmt);

/**
 * Sets parent pointers for all nodes in the tree
 */
void setParentPointers(ASTNode* root, ASTNode* parent);

/**
 * Prints the AST in a tree-like format with indentation
 */
void printAST(ASTNode* root, int level);

/**
 * Prints AST with detailed metadata
 */
void printASTDetailed(ASTNode* root, int level);

/**
 * Calculates cyclomatic complexity for a function
 */
int calculateComplexity(ASTNode* funcNode);

/**
 * Calculates maximum nesting depth in a function
 */
int calculateNestingDepth(ASTNode* node, int currentDepth);

/**
 * Counts total nodes in the tree
 */
int countNodes(ASTNode* root);

/**
 * Finds all function calls within a function
 */
void findFunctionCalls(ASTNode* root, char*** calls, int* count);

/**
 * Frees all memory occupied by the AST recursively
 */
void freeAST(ASTNode* root);

#endif // AST_H