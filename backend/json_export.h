// json_export.h
// JSON export interface for CodeMaster analysis results.
// Enables React/D3.js/Cytoscape frontend integration.
#ifndef JSON_EXPORT_H
#define JSON_EXPORT_H

#include "hashtable.h"
#include "graph.h"
#include "ast.h"

/**
 * Exports the full symbol table as structured JSON.
 * Output: output/symbols.json
 */
void export_symbol_table_json(HashTable* ht, const char* filename);

/**
 * Exports the function call graph as structured JSON.
 * Includes per-function metadata (recursion, dead code, coupling, calls).
 * Output: output/graph.json
 */
void export_graph_json(FunctionGraph* graph, const char* filename);

/**
 * Recursively exports the Abstract Syntax Tree as structured JSON.
 * Output: output/ast.json
 */
void export_ast_json(ASTNode* root, const char* filename);

/**
 * Runs the quality analysis and exports issues ranked by severity as JSON.
 * Output: output/issues.json
 */
void export_issues_json(FunctionGraph* graph, ASTNode* ast, const char* filename);

/**
 * Convenience wrapper: exports all four JSON files in one call.
 */
void export_all_json(HashTable* ht, FunctionGraph* graph, ASTNode* ast);

#endif // JSON_EXPORT_H
