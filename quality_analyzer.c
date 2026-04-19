#include "quality_analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PriorityQueue* pq_create(int capacity) {
    PriorityQueue* pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    pq->issues = (QualityIssue*)malloc(sizeof(QualityIssue) * capacity);
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

void pq_destroy(PriorityQueue* pq) {
    if (pq) {
        free(pq->issues);
        free(pq);
    }
}

static void swap_issues(QualityIssue* a, QualityIssue* b) {
    QualityIssue temp = *a;
    *a = *b;
    *b = temp;
}

void pq_push(PriorityQueue* pq, QualityIssue issue) {
    if (pq->size >= pq->capacity) return;
    
    int i = pq->size++;
    pq->issues[i] = issue;
    
    // Bubble up (Max-heap by severity)
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq->issues[i].severity > pq->issues[parent].severity) {
            swap_issues(&pq->issues[i], &pq->issues[parent]);
            i = parent;
        } else {
            break;
        }
    }
}

QualityIssue pq_pop(PriorityQueue* pq) {
    QualityIssue top = pq->issues[0];
    pq->issues[0] = pq->issues[--pq->size];
    
    int i = 0;
    while (1) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int largest = i;
        
        if (left < pq->size && pq->issues[left].severity > pq->issues[largest].severity)
            largest = left;
        if (right < pq->size && pq->issues[right].severity > pq->issues[largest].severity)
            largest = right;
            
        if (largest != i) {
            swap_issues(&pq->issues[i], &pq->issues[largest]);
            i = largest;
        } else {
            break;
        }
    }
    return top;
}

int pq_is_empty(PriorityQueue* pq) {
    return pq->size == 0;
}

static int get_max_line_recursive(ASTNode* node) {
    if (!node) return 0;
    int max = node->lineNumber;
    if (node->left) {
        int l = get_max_line_recursive(node->left);
        if (l > max) max = l;
    }
    if (node->right) {
        int r = get_max_line_recursive(node->right);
        if (r > max) max = r;
    }
    for (int i = 0; i < node->childCount; i++) {
        int c = get_max_line_recursive(node->children[i]);
        if (c > max) max = c;
    }
    return max;
}

void analyze_node_quality(ASTNode* node, FunctionGraph* graph, PriorityQueue* pq) {
    if (!node) return;
    
    if (node->type == NODE_FUNC_DECL) {
        char* func_name = node->value;
        
        // 1. Too Many Parameters (> 5) - Severity 2
        if (node->paramCount > 5) {
            QualityIssue issue;
            issue.type = ISSUE_TOO_MANY_PARAMS;
            issue.severity = 2;
            issue.line = node->lineNumber;
            strncpy(issue.function_name, func_name, MAX_NAME-1);
            snprintf(issue.description, 255, "Function '%s' has too many parameters (%d > 5)", func_name, node->paramCount);
            pq_push(pq, issue);
        }
        
        // 2. Large Function (> 50 lines) - Severity 3
        int start = node->lineNumber;
        int end = get_max_line_recursive(node);
        int lines = end - start + 1;
        if (lines > 50) {
            QualityIssue issue;
            issue.type = ISSUE_LARGE_FUNC;
            issue.severity = 3;
            issue.line = node->lineNumber;
            strncpy(issue.function_name, func_name, MAX_NAME-1);
            snprintf(issue.description, 255, "Function '%s' is too large (%d lines > 50)", func_name, lines);
            pq_push(pq, issue);
        }
        
        // 3. Deep Nesting (> 4) - Severity 4
        int nesting = calculateNestingDepth(node, 0);
        if (nesting > 4) {
            QualityIssue issue;
            issue.type = ISSUE_DEEP_NESTING;
            issue.severity = 4;
            issue.line = node->lineNumber;
            strncpy(issue.function_name, func_name, MAX_NAME-1);
            snprintf(issue.description, 255, "Function '%s' has deep nesting (%d > 4)", func_name, nesting);
            pq_push(pq, issue);
        }
        
        // 4. High Fan-out (> 5) - Severity 3
        if (graph) {
            int idx = graph_find_function(graph, func_name);
            if (idx != -1) {
                GraphNode* gnode = graph_get_node(graph, idx);
                if (gnode && gnode->out_degree > 5) {
                    QualityIssue issue;
                    issue.type = ISSUE_HIGH_FANOUT;
                    issue.severity = 3;
                    issue.line = node->lineNumber;
                    strncpy(issue.function_name, func_name, MAX_NAME-1);
                    snprintf(issue.description, 255, "Function '%s' has high fan-out (%d calls > 5)", func_name, gnode->out_degree);
                    pq_push(pq, issue);
                }
            }
        }
        
        // 5. Unused Variables - Severity 5
        DFA_Result dfa;
        run_dfa_analysis(node, &dfa);
        for (int i = 0; i < dfa.count; i++) {
            if (dfa.vars[i].readCount == 0) {
                QualityIssue issue;
                issue.type = ISSUE_UNUSED_VAR;
                issue.severity = 5;
                issue.line = dfa.vars[i].declaredLine;
                strncpy(issue.function_name, func_name, MAX_NAME-1);
                if (dfa.vars[i].writeCount == 0) {
                    snprintf(issue.description, 255, "Variable '%s' declared but never used", dfa.vars[i].name);
                } else {
                    snprintf(issue.description, 255, "Variable '%s' declared and assigned but never read", dfa.vars[i].name);
                }
                pq_push(pq, issue);
            }
        }
    }
}

static void analyze_tree_recursive(ASTNode* node, FunctionGraph* graph, PriorityQueue* pq) {
    if (!node) return;
    
    analyze_node_quality(node, graph, pq);
    
    if (node->left) analyze_tree_recursive(node->left, graph, pq);
    if (node->right) analyze_tree_recursive(node->right, graph, pq);
    for (int i = 0; i < node->childCount; i++) {
        analyze_tree_recursive(node->children[i], graph, pq);
    }
}

void generate_ranked_report(ASTNode* root, FunctionGraph* graph) {
    printf("\n┌────────────────────────────────────────────────────────────────┐\n");
    printf("│                RANKED CODE QUALITY REPORT                      │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    
    PriorityQueue* pq = pq_create(100);
    
    // Analyze global nodes recursively
    analyze_tree_recursive(root, graph, pq);
    
    if (pq_is_empty(pq)) {
        printf("✓ No major code quality issues detected.\n\n");
    } else {
        printf("%-10s %-8s %-12s %-s\n", "Severity", "Line", "Function", "Issue Description");
        printf("─────────────────────────────────────────────────────────────────\n");
        
        while (!pq_is_empty(pq)) {
            QualityIssue issue = pq_pop(pq);
            char sev_str[16];
            if (issue.severity == 5) strcpy(sev_str, "5 CRITICAL");
            else if (issue.severity == 4) strcpy(sev_str, "4 HIGH");
            else if (issue.severity == 3) strcpy(sev_str, "3 MEDIUM");
            else if (issue.severity == 2) strcpy(sev_str, "2 LOW");
            else strcpy(sev_str, "1 INFO");
            
            printf("%-10s %-8d %-12s %-s\n", 
                   sev_str, issue.line, issue.function_name, issue.description);
        }
        printf("\nTotal issues prioritized by severity.\n\n");
    }
    
    pq_destroy(pq);
}
