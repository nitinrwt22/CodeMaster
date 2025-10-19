// parser.h
#ifndef PARSER_H
#define PARSER_H

#include "utils.h"
#include "hashtable.h"
#include "trie.h"
#include "avltree.h"
#include "ast.h"

// Parse file and populate all data structures
void parse_file_and_populate(const char* filename, HashTable* ht, TrieNode* trie, AVLNode** avl);

// Parse file and build AST (Phase 2)
ASTNode* parse_file_to_ast(const char* filename);

// Analyze AST and extract metrics
void analyze_ast(ASTNode* root);

#endif // PARSER_H