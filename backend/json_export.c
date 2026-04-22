// json_export.c
// Implements JSON serialization for all CodeMaster analysis results.
// Uses manual fprintf formatting — no external JSON library required.
#include "json_export.h"
#include "quality_analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
   INTERNAL HELPERS
   ============================================================================ */

/** Safely escapes special JSON characters in a string */
static void write_json_string(FILE* f, const char* s) {
    fputc('"', f);
    for (const char* p = s; *p; p++) {
        switch (*p) {
            case '"':  fputs("\\\"", f); break;
            case '\\': fputs("\\\\", f); break;
            case '\n': fputs("\\n",  f); break;
            case '\r': fputs("\\r",  f); break;
            case '\t': fputs("\\t",  f); break;
            default:   fputc(*p, f);    break;
        }
    }
    fputc('"', f);
}

/** Returns a human-readable string for a NodeType */
static const char* node_type_str(NodeType t) {
    switch (t) {
        case NODE_PROGRAM:   return "PROGRAM";
        case NODE_INT:       return "INT_LITERAL";
        case NODE_FLOAT:     return "FLOAT_LITERAL";
        case NODE_ID:        return "IDENTIFIER";
        case NODE_OP:        return "OPERATOR";
        case NODE_ASSIGN:    return "ASSIGNMENT";
        case NODE_IF:        return "IF";
        case NODE_WHILE:     return "WHILE";
        case NODE_FOR:       return "FOR";
        case NODE_DO_WHILE:  return "DO_WHILE";
        case NODE_RETURN:    return "RETURN";
        case NODE_FUNC_DECL: return "FUNCTION_DECL";
        case NODE_FUNC_CALL: return "FUNCTION_CALL";
        case NODE_VAR_DECL:  return "VAR_DECL";
        case NODE_BLOCK:     return "BLOCK";
        case NODE_EXPR:      return "EXPRESSION";
        case NODE_PARAM:     return "PARAMETER";
        default:             return "UNKNOWN";
    }
}

/** Returns a human-readable string for an IssueType */
static const char* issue_type_str(IssueType t) {
    switch (t) {
        case ISSUE_UNUSED_VAR:      return "unused_variable";
        case ISSUE_DEEP_NESTING:    return "deep_nesting";
        case ISSUE_LARGE_FUNC:      return "large_function";
        case ISSUE_HIGH_FANOUT:     return "high_fanout";
        case ISSUE_TOO_MANY_PARAMS: return "too_many_parameters";
        case ISSUE_HIGH_COMPLEXITY: return "high_complexity";
        case ISSUE_DEAD_CODE:       return "dead_code";
        case ISSUE_RECURSIVE:       return "recursive_function";
        default:                    return "unknown";
    }
}

/** Returns severity label */
static const char* severity_label(int s) {
    if (s == 5) return "critical";
    if (s == 4) return "high";
    if (s == 3) return "medium";
    if (s == 2) return "low";
    return "info";
}

/* ============================================================================
   1. SYMBOL TABLE JSON EXPORT
   ============================================================================ */

void export_symbol_table_json(HashTable* ht, const char* filename) {
    if (!ht || !filename) return;

    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "[JSON] Error: Cannot open '%s' for writing.\n", filename);
        return;
    }

    fprintf(f, "{\n");
    fprintf(f, "  \"symbols\": [\n");

    int first = 1;
    int total = 0;

    /* Walk every bucket in the hash table */
    for (int i = 0; i < ht->size; i++) {
        HTNode* node = ht->buckets[i];
        while (node) {
            SymbolInfo* s = &node->info;

            if (!first) fprintf(f, ",\n");
            first = 0;

            fprintf(f, "    {\n");
            fprintf(f, "      \"name\": ");     write_json_string(f, s->name);     fprintf(f, ",\n");
            fprintf(f, "      \"type\": ");     write_json_string(f, s->type);     fprintf(f, ",\n");
            fprintf(f, "      \"category\": "); write_json_string(f, s->category); fprintf(f, ",\n");
            fprintf(f, "      \"line\": %d\n", s->line);
            fprintf(f, "    }");

            total++;
            node = node->next;
        }
    }

    fprintf(f, "\n  ],\n");
    fprintf(f, "  \"total_symbols\": %d\n", total);
    fprintf(f, "}\n");

    fclose(f);
    printf("  [JSON] ✓ Symbol table exported → %s (%d symbols)\n", filename, total);
}

/* ============================================================================
   2. FUNCTION CALL GRAPH JSON EXPORT
   ============================================================================ */

void export_graph_json(FunctionGraph* graph, const char* filename) {
    if (!graph || !filename) return;

    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "[JSON] Error: Cannot open '%s' for writing.\n", filename);
        return;
    }

    fprintf(f, "{\n");
    fprintf(f, "  \"functions\": [\n");

    for (int i = 0; i < graph->node_count; i++) {
        GraphNode* gn = &graph->nodes[i];

        if (i > 0) fprintf(f, ",\n");

        fprintf(f, "    {\n");
        fprintf(f, "      \"name\": ");        write_json_string(f, gn->function_name); fprintf(f, ",\n");
        fprintf(f, "      \"return_type\": "); write_json_string(f, gn->return_type);   fprintf(f, ",\n");
        fprintf(f, "      \"line\": %d,\n", gn->line_number);
        fprintf(f, "      \"out_degree\": %d,\n", gn->out_degree);
        fprintf(f, "      \"in_degree\": %d,\n",  gn->in_degree);
        fprintf(f, "      \"is_recursive\": %s,\n",     gn->is_recursive     ? "true" : "false");
        fprintf(f, "      \"is_dead_code\": %s,\n",     gn->is_dead_code     ? "true" : "false");
        fprintf(f, "      \"is_highly_coupled\": %s,\n", gn->is_highly_coupled ? "true" : "false");

        /* Build the calls array from the adjacency list */
        fprintf(f, "      \"calls\": [");
        AdjListNode* adj = gn->adj_list;
        int first_call = 1;
        int call_count = 0;
        while (adj) {
            if (!first_call) fprintf(f, ", ");
            first_call = 0;
            int dest = adj->dest_index;
            if (dest >= 0 && dest < graph->node_count) {
                write_json_string(f, graph->nodes[dest].function_name);
            }
            call_count++;
            adj = adj->next;
        }
        fprintf(f, "],\n");
        fprintf(f, "      \"call_count\": %d\n", call_count);
        fprintf(f, "    }");
    }

    fprintf(f, "\n  ],\n");
    fprintf(f, "  \"total_functions\": %d,\n", graph->node_count);
    fprintf(f, "  \"total_calls\": %d,\n", graph->edge_count);
    fprintf(f, "  \"has_cycles\": %s\n", graph->has_cycles ? "true" : "false");
    fprintf(f, "}\n");

    fclose(f);
    printf("  [JSON] ✓ Call graph exported   → %s (%d functions, %d calls)\n",
           filename, graph->node_count, graph->edge_count);
}

/* ============================================================================
   3. AST JSON EXPORT (Recursive)
   ============================================================================ */

static void export_ast_node_json(FILE* f, ASTNode* node, int indent) {
    if (!node) return;

    /* Build indent string */
    char pad[128];
    int padlen = indent * 2;
    if (padlen > 126) padlen = 126;
    memset(pad, ' ', padlen);
    pad[padlen] = '\0';

    fprintf(f, "%s{\n", pad);
    fprintf(f, "%s  \"type\": ", pad);  write_json_string(f, node_type_str(node->type)); fprintf(f, ",\n");
    fprintf(f, "%s  \"value\": ", pad); write_json_string(f, node->value);               fprintf(f, ",\n");
    fprintf(f, "%s  \"line\": %d,\n", pad, node->lineNumber);

    /* Collect children: prefer children[] array, then left/right if no children */
    ASTNode* kids[64];
    int kid_count = 0;

    if (node->childCount > 0) {
        /* Use explicit children array */
        for (int i = 0; i < node->childCount && kid_count < 64; i++) {
            if (node->children[i]) kids[kid_count++] = node->children[i];
        }
    } else {
        /* Fallback to binary left/right children */
        if (node->left  && kid_count < 64) kids[kid_count++] = node->left;
        if (node->right && kid_count < 64) kids[kid_count++] = node->right;
    }
    /* NOTE: do NOT follow node->next — it's a sibling pointer, not a child */

    fprintf(f, "%s  \"children\": [\n", pad);
    for (int i = 0; i < kid_count; i++) {
        export_ast_node_json(f, kids[i], indent + 2);
        if (i < kid_count - 1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    fprintf(f, "%s  ]\n", pad);
    fprintf(f, "%s}", pad);
}

void export_ast_json(ASTNode* root, const char* filename) {
    if (!root || !filename) return;

    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "[JSON] Error: Cannot open '%s' for writing.\n", filename);
        return;
    }

    export_ast_node_json(f, root, 0);
    fprintf(f, "\n");

    fclose(f);
    printf("  [JSON] ✓ AST exported           → %s\n", filename);
}

/* ============================================================================
   4. ISSUES / QUALITY REPORT JSON EXPORT
   ============================================================================ */

void export_issues_json(FunctionGraph* graph, ASTNode* ast, const char* filename) {
    if (!filename) return;

    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "[JSON] Error: Cannot open '%s' for writing.\n", filename);
        return;
    }

    /* Build a temporary priority queue with the same logic as generate_ranked_report */
    PriorityQueue* pq = pq_create(200);

    /* ---- Structural issues from AST ---- */
    if (ast) {
        /* We need to walk the tree; reuse the same helper pattern */
        /* Inline mini-walker since analyze_node_quality is static in quality_analyzer.c */
        /* Collect all FUNC_DECL nodes via a simple stack-based traversal */
        ASTNode* stack[512];
        int top = 0;
        stack[top++] = ast;

        while (top > 0) {
            ASTNode* node = stack[--top];
            if (!node) continue;

            if (node->type == NODE_FUNC_DECL) {
                char* fn = node->value;

                /* Too Many Parameters */
                if (node->paramCount > 5) {
                    QualityIssue issue = {0};
                    issue.type = ISSUE_TOO_MANY_PARAMS;
                    issue.severity = 2;
                    issue.line = node->lineNumber;
                    strncpy(issue.function_name, fn, MAX_NAME - 1);
                    snprintf(issue.description, 255,
                             "Function '%s' has too many parameters (%d > 5)", fn, node->paramCount);
                    pq_push(pq, issue);
                }

                /* Large Function */
                int start = node->lineNumber;
                /* compute max line by visiting children */
                ASTNode* ms[512]; int mt = 0; int max_line = start;
                ms[mt++] = node;
                while (mt > 0) {
                    ASTNode* mn = ms[--mt];
                    if (!mn) continue;
                    if (mn->lineNumber > max_line) max_line = mn->lineNumber;
                    if (mn->left  && mt < 511) ms[mt++] = mn->left;
                    if (mn->right && mt < 511) ms[mt++] = mn->right;
                    for (int ci = 0; ci < mn->childCount && mt < 511; ci++)
                        if (mn->children[ci]) ms[mt++] = mn->children[ci];
                }
                int lines = max_line - start + 1;
                if (lines > 50) {
                    QualityIssue issue = {0};
                    issue.type = ISSUE_LARGE_FUNC;
                    issue.severity = 3;
                    issue.line = start;
                    strncpy(issue.function_name, fn, MAX_NAME - 1);
                    snprintf(issue.description, 255,
                             "Function '%s' is too large (%d lines > 50)", fn, lines);
                    pq_push(pq, issue);
                }

                /* Deep Nesting */
                int nesting = calculateNestingDepth(node, 0);
                if (nesting > 4) {
                    QualityIssue issue = {0};
                    issue.type = ISSUE_DEEP_NESTING;
                    issue.severity = 4;
                    issue.line = node->lineNumber;
                    strncpy(issue.function_name, fn, MAX_NAME - 1);
                    snprintf(issue.description, 255,
                             "Function '%s' has deep nesting (%d > 4)", fn, nesting);
                    pq_push(pq, issue);
                }

                /* High Fan-out */
                if (graph) {
                    int idx = graph_find_function(graph, fn);
                    if (idx != -1) {
                        GraphNode* gn = graph_get_node(graph, idx);
                        if (gn && gn->out_degree > 5) {
                            QualityIssue issue = {0};
                            issue.type = ISSUE_HIGH_FANOUT;
                            issue.severity = 3;
                            issue.line = node->lineNumber;
                            strncpy(issue.title,         "High Fan-out", sizeof(issue.title) - 1);
                            strncpy(issue.function_name, fn, MAX_NAME - 1);
                            snprintf(issue.description, 255,
                                     "Function '%s' calls %d distinct functions (threshold: 5). "
                                     "High fan-out increases coupling risk.", fn, gn->out_degree);
                            pq_push(pq, issue);
                        }
                    }
                }

                /* High Complexity */
                int complexity = calculateComplexity(node);
                if (complexity > 5) {
                    QualityIssue issue = {0};
                    issue.type = ISSUE_HIGH_COMPLEXITY;
                    issue.severity = 3;
                    issue.line = node->lineNumber;
                    strncpy(issue.title,         "High Complexity", sizeof(issue.title) - 1);
                    strncpy(issue.function_name, fn, MAX_NAME - 1);
                    snprintf(issue.description, 255,
                             "Function '%s' has cyclomatic complexity %d (threshold: 5). "
                             "High complexity increases bug probability.", fn, complexity);
                    pq_push(pq, issue);
                }

                /* Unused Variables via DFA */
                DFA_Result dfa;
                run_dfa_analysis(node, &dfa);
                for (int di = 0; di < dfa.count; di++) {
                    if (dfa.vars[di].readCount == 0) {
                        QualityIssue issue = {0};
                        issue.type = ISSUE_UNUSED_VAR;
                        issue.severity = 5;
                        issue.line = dfa.vars[di].declaredLine;
                        strncpy(issue.title,         "Unused Variable", sizeof(issue.title) - 1);
                        strncpy(issue.function_name, fn, MAX_NAME - 1);
                        if (dfa.vars[di].writeCount == 0)
                            snprintf(issue.description, 255,
                                "Variable '%s' declared but never used. "
                                "Remove or use it to avoid dead allocation.", dfa.vars[di].name);
                        else
                            snprintf(issue.description, 255,
                                "Variable '%s' declared and assigned but never read. "
                                "The assignment has no observable effect.", dfa.vars[di].name);
                        pq_push(pq, issue);
                    }
                }
            }

            /* Push children onto traversal stack */
            if (node->left  && top < 511) stack[top++] = node->left;
            if (node->right && top < 511) stack[top++] = node->right;
            for (int ci = 0; ci < node->childCount && top < 511; ci++)
                if (node->children[ci]) stack[top++] = node->children[ci];
        }
    }

    /* ---- Recursion / dead-code issues from graph ---- */
    if (graph) {
        for (int i = 0; i < graph->node_count; i++) {
            GraphNode* gn = &graph->nodes[i];
            char desc[256];

            if (gn->is_recursive) {
                QualityIssue issue = {0};
                issue.type     = ISSUE_RECURSIVE;
                issue.severity = 3;
                issue.line     = gn->line_number;
                strncpy(issue.title,         "Recursive Function",  sizeof(issue.title) - 1);
                strncpy(issue.function_name, gn->function_name,     MAX_NAME - 1);
                snprintf(desc, sizeof(desc),
                    "Function '%s' calls itself recursively. "
                    "Ensure a proper base case to prevent stack overflow.",
                    gn->function_name);
                strncpy(issue.description, desc, sizeof(issue.description) - 1);
                pq_push(pq, issue);
            }

            if (gn->is_dead_code) {
                QualityIssue issue = {0};
                issue.type     = ISSUE_DEAD_CODE;
                issue.severity = 2;
                issue.line     = gn->line_number;
                strncpy(issue.title,         "Dead Code",        sizeof(issue.title) - 1);
                strncpy(issue.function_name, gn->function_name, MAX_NAME - 1);
                snprintf(desc, sizeof(desc),
                    "Function '%s' is defined but never called. "
                    "Remove or add an explicit entry point.",
                    gn->function_name);
                strncpy(issue.description, desc, sizeof(issue.description) - 1);
                pq_push(pq, issue);
            }
        }
    }

    /* ---- Drain the priority queue into JSON ---- */
    /* First pass: copy to a temp buffer so we can count */
    QualityIssue buffer[200];
    int total = 0;
    while (!pq_is_empty(pq) && total < 200)
        buffer[total++] = pq_pop(pq);

    pq_destroy(pq);

    /* Count by severity for summary */
    int n_critical = 0, n_high = 0, n_medium = 0, n_low = 0;
    for (int i = 0; i < total; i++) {
        int s = buffer[i].severity;
        if      (s == 5) n_critical++;
        else if (s == 4) n_high++;
        else if (s == 3) n_medium++;
        else             n_low++;
    }
    int raw_score = 100 - (n_critical * 10 + n_high * 5 + n_medium * 2 + n_low * 1);
    if (raw_score < 0) raw_score = 0;

    fprintf(f, "{\n");

    /* Summary block */
    fprintf(f, "  \"summary\": {\n");
    fprintf(f, "    \"total_issues\": %d,\n", total);
    fprintf(f, "    \"critical\": %d,\n", n_critical);
    fprintf(f, "    \"high\": %d,\n",     n_high);
    fprintf(f, "    \"medium\": %d,\n",   n_medium);
    fprintf(f, "    \"low\": %d,\n",      n_low);
    fprintf(f, "    \"stability_score\": %d\n", raw_score);
    fprintf(f, "  },\n");

    fprintf(f, "  \"issues\": [\n");

    for (int i = 0; i < total; i++) {
        QualityIssue* iss = &buffer[i];
        if (i > 0) fprintf(f, ",\n");
        fprintf(f, "    {\n");
        fprintf(f, "      \"severity\": %d,\n", iss->severity);
        fprintf(f, "      \"severity_label\": "); write_json_string(f, severity_label(iss->severity)); fprintf(f, ",\n");
        fprintf(f, "      \"type\": ");  write_json_string(f, issue_type_str(iss->type));  fprintf(f, ",\n");
        fprintf(f, "      \"title\": "); write_json_string(f, iss->title[0] ? iss->title : issue_type_str(iss->type)); fprintf(f, ",\n");
        fprintf(f, "      \"function\": "); write_json_string(f, iss->function_name); fprintf(f, ",\n");
        fprintf(f, "      \"line\": %d,\n", iss->line);
        fprintf(f, "      \"message\": "); write_json_string(f, iss->description); fprintf(f, "\n");
        fprintf(f, "    }");
    }

    fprintf(f, "\n  ],\n");
    fprintf(f, "  \"total_issues\": %d\n", total);
    fprintf(f, "}\n");

    fclose(f);
    printf("  [JSON] ✓ Issues exported        → %s (%d issues)\n", filename, total);
}

/* ============================================================================
   5. CONVENIENCE WRAPPER
   ============================================================================ */

void export_all_json(HashTable* ht, FunctionGraph* graph, ASTNode* ast) {
    printf("\n┌────────────────────────────────────────────────────────────────┐\n");
    printf("│               EXPORTING JSON ANALYSIS RESULTS                 │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n");

    export_symbol_table_json(ht,       "output/symbols.json");
    export_graph_json(graph,           "output/graph.json");
    export_ast_json(ast,              "output/ast.json");
    export_issues_json(graph, ast,    "output/issues.json");

    printf("\n  All JSON exports written to: output/\n");
    printf("  Ready for React / D3.js / Cytoscape.js integration.\n\n");
}
