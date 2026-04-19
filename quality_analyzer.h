#ifndef QUALITY_ANALYZER_H
#define QUALITY_ANALYZER_H

#include "ast.h"
#include "graph.h"

typedef enum {
    ISSUE_UNUSED_VAR,
    ISSUE_DEEP_NESTING,
    ISSUE_LARGE_FUNC,
    ISSUE_HIGH_FANOUT,
    ISSUE_TOO_MANY_PARAMS,
    ISSUE_HIGH_COMPLEXITY
} IssueType;

typedef struct {
    IssueType type;
    int severity;       // 1-5
    char description[256];
    int line;
    char function_name[MAX_NAME];
} QualityIssue;

typedef struct {
    QualityIssue* issues;
    int size;
    int capacity;
} PriorityQueue;

// Priority Queue operations
PriorityQueue* pq_create(int capacity);
void pq_destroy(PriorityQueue* pq);
void pq_push(PriorityQueue* pq, QualityIssue issue);
QualityIssue pq_pop(PriorityQueue* pq);
int pq_is_empty(PriorityQueue* pq);

// Analysis operations
void generate_ranked_report(ASTNode* root, FunctionGraph* graph);

#endif // QUALITY_ANALYZER_H
