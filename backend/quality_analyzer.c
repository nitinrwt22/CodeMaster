// quality_analyzer.c
// Structured Code Quality Dashboard reporting layer.
// Core analysis logic is unchanged; only the reporting layer is refactored.

#include "quality_analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ─── ANSI Color Codes ──────────────────────────────────────────────────────
#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_RED     "\033[38;5;203m"   // Critical – vivid red
#define ANSI_ORANGE  "\033[38;5;215m"   // High     – orange
#define ANSI_YELLOW  "\033[38;5;220m"   // Medium   – yellow
#define ANSI_BLUE    "\033[38;5;117m"   // Low      – steel blue
#define ANSI_GREEN   "\033[38;5;84m"    // OK / score
#define ANSI_GRAY    "\033[38;5;245m"   // Dim text
#define ANSI_CYAN    "\033[38;5;159m"   // Accent

// ─── Priority Queue ────────────────────────────────────────────────────────

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

    // Bubble up (max-heap by severity)
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
        int left    = 2 * i + 1;
        int right   = 2 * i + 2;
        int largest = i;

        if (left  < pq->size && pq->issues[left].severity  > pq->issues[largest].severity) largest = left;
        if (right < pq->size && pq->issues[right].severity > pq->issues[largest].severity) largest = right;

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

// ─── AST Helpers ──────────────────────────────────────────────────────────

static int get_max_line_recursive(ASTNode* node) {
    if (!node) return 0;
    int max = node->lineNumber;
    if (node->left)  { int l = get_max_line_recursive(node->left);  if (l > max) max = l; }
    if (node->right) { int r = get_max_line_recursive(node->right); if (r > max) max = r; }
    for (int i = 0; i < node->childCount; i++) {
        int c = get_max_line_recursive(node->children[i]);
        if (c > max) max = c;
    }
    return max;
}

// ─── Issue Push Helpers ───────────────────────────────────────────────────

static void push_issue(PriorityQueue* pq,
                        IssueType type, int severity,
                        const char* title,
                        const char* description,
                        const char* func_name,
                        int line)
{
    QualityIssue issue;
    memset(&issue, 0, sizeof(QualityIssue));
    issue.type     = type;
    issue.severity = severity;
    issue.line     = line;
    strncpy(issue.title,         title,       sizeof(issue.title) - 1);
    strncpy(issue.description,   description, sizeof(issue.description) - 1);
    strncpy(issue.function_name, func_name,   MAX_NAME - 1);
    pq_push(pq, issue);
}

// ─── Per-Node Analysis ────────────────────────────────────────────────────

void analyze_node_quality(ASTNode* node, FunctionGraph* graph, PriorityQueue* pq) {
    if (!node) return;

    if (node->type == NODE_FUNC_DECL) {
        char* fn = node->value;
        char desc[256];

        // 1. Too Many Parameters (> 5) — Severity 2 LOW
        if (node->paramCount > 5) {
            snprintf(desc, sizeof(desc),
                "Function '%s' has %d parameters (threshold: 5). "
                "Consider grouping params into a struct.", fn, node->paramCount);
            push_issue(pq, ISSUE_TOO_MANY_PARAMS, 2,
                "Excessive Parameters", desc, fn, node->lineNumber);
        }

        // 2. Large Function (> 50 lines) — Severity 3 MEDIUM
        int start = node->lineNumber;
        int end   = get_max_line_recursive(node);
        int lines = end - start + 1;
        if (lines > 50) {
            snprintf(desc, sizeof(desc),
                "Function '%s' spans %d lines (threshold: 50). "
                "Extract cohesive logic into sub-functions.", fn, lines);
            push_issue(pq, ISSUE_LARGE_FUNC, 3,
                "Large Function", desc, fn, node->lineNumber);
        }

        // 3. Deep Nesting (> 4) — Severity 4 HIGH
        int nesting = calculateNestingDepth(node, 0);
        if (nesting > 4) {
            snprintf(desc, sizeof(desc),
                "Function '%s' has nesting depth %d (threshold: 4). "
                "Refactor with early returns or helper functions.", fn, nesting);
            push_issue(pq, ISSUE_DEEP_NESTING, 4,
                "Deep Nesting", desc, fn, node->lineNumber);
        }

        // 4. High Complexity (cyclomatic > 5) — Severity 3 MEDIUM
        int complexity = calculateComplexity(node);
        if (complexity > 5) {
            snprintf(desc, sizeof(desc),
                "Function '%s' has cyclomatic complexity %d (threshold: 5). "
                "High complexity increases bug probability.", fn, complexity);
            push_issue(pq, ISSUE_HIGH_COMPLEXITY, 3,
                "High Complexity", desc, fn, node->lineNumber);
        }

        // 5. High Fan-out (> 5) — Severity 3 MEDIUM
        if (graph) {
            int idx = graph_find_function(graph, fn);
            if (idx != -1) {
                GraphNode* gnode = graph_get_node(graph, idx);
                if (gnode && gnode->out_degree > 5) {
                    snprintf(desc, sizeof(desc),
                        "Function '%s' calls %d distinct functions (threshold: 5). "
                        "High fan-out increases coupling risk.", fn, gnode->out_degree);
                    push_issue(pq, ISSUE_HIGH_FANOUT, 3,
                        "High Fan-out", desc, fn, node->lineNumber);
                }
            }
        }

        // 6. Unused Variables (DFA) — Severity 5 CRITICAL
        DFA_Result dfa;
        run_dfa_analysis(node, &dfa);
        for (int i = 0; i < dfa.count; i++) {
            if (dfa.vars[i].readCount == 0) {
                if (dfa.vars[i].writeCount == 0) {
                    snprintf(desc, sizeof(desc),
                        "Variable '%s' declared but never used. "
                        "Remove or use it to avoid dead allocation.", dfa.vars[i].name);
                } else {
                    snprintf(desc, sizeof(desc),
                        "Variable '%s' declared and assigned but never read. "
                        "The assignment has no observable effect.", dfa.vars[i].name);
                }
                push_issue(pq, ISSUE_UNUSED_VAR, 5,
                    "Unused Variable", desc, fn, dfa.vars[i].declaredLine);
            }
        }
    }
}

// ─── Recursive Tree Walk ──────────────────────────────────────────────────

static void analyze_tree_recursive(ASTNode* node, FunctionGraph* graph, PriorityQueue* pq) {
    if (!node) return;
    analyze_node_quality(node, graph, pq);
    if (node->left)  analyze_tree_recursive(node->left,  graph, pq);
    if (node->right) analyze_tree_recursive(node->right, graph, pq);
    for (int i = 0; i < node->childCount; i++)
        analyze_tree_recursive(node->children[i], graph, pq);
}

// ─── Severity Helpers ─────────────────────────────────────────────────────

static const char* severity_color(int sev) {
    if (sev == 5) return ANSI_RED;
    if (sev == 4) return ANSI_ORANGE;
    if (sev == 3) return ANSI_YELLOW;
    return ANSI_BLUE;
}

static const char* severity_label(int sev) {
    if (sev == 5) return "CRITICAL";
    if (sev == 4) return "HIGH";
    if (sev == 3) return "MEDIUM";
    if (sev == 2) return "LOW";
    return "INFO";
}

static const char* severity_icon(int sev) {
    if (sev == 5) return "✖";
    if (sev == 4) return "!";
    if (sev == 3) return "~";
    return "·";
}

// ─── Print a Single Issue Card ────────────────────────────────────────────

static void print_issue_card(const QualityIssue* issue) {
    const char* col   = severity_color(issue->severity);
    const char* label = severity_label(issue->severity);
    const char* icon  = severity_icon(issue->severity);

    printf("%s────────────────────────────────────────────────────────────%s\n",
           ANSI_GRAY, ANSI_RESET);

    // Header row: [SEVERITY ICON] [LABEL] Title
    printf(" %s%s%s %s%-10s%s  %s%s%s\n",
           col, ANSI_BOLD, icon, col, label, ANSI_RESET,
           ANSI_BOLD, issue->title, ANSI_RESET);

    // Location row
    printf("   %sFunction:%s %-20s  %sLine:%s %d\n",
           ANSI_GRAY, ANSI_RESET, issue->function_name,
           ANSI_GRAY, ANSI_RESET, issue->line);

    // Detail row (wrapped)
    printf("   %sDetails:%s  %s\n",
           ANSI_GRAY, ANSI_RESET, issue->description);
}

// ─── Main Report Entry Point ──────────────────────────────────────────────

void generate_ranked_report(ASTNode* root, FunctionGraph* graph) {
    PriorityQueue* pq = pq_create(200);

    // Populate from AST
    analyze_tree_recursive(root, graph, pq);

    // Populate graph-level issues (recursion, dead code)
    if (graph) {
        for (int i = 0; i < graph->node_count; i++) {
            GraphNode* gn = &graph->nodes[i];
            char desc[256];

            if (gn->is_recursive) {
                snprintf(desc, sizeof(desc),
                    "Function '%s' calls itself recursively. "
                    "Ensure a proper base case to prevent stack overflow.",
                    gn->function_name);
                push_issue(pq, ISSUE_RECURSIVE, 3,
                    "Recursive Function", desc, gn->function_name, gn->line_number);
            }

            if (gn->is_dead_code) {
                snprintf(desc, sizeof(desc),
                    "Function '%s' is defined but never called from any reachable path. "
                    "Remove or add an explicit entry point.",
                    gn->function_name);
                push_issue(pq, ISSUE_DEAD_CODE, 2,
                    "Dead Code", desc, gn->function_name, gn->line_number);
            }
        }
    }

    // ── Drain into flat buffer ─────────────────────────────────────────
    QualityIssue buffer[200];
    int total = 0;
    while (!pq_is_empty(pq) && total < 200)
        buffer[total++] = pq_pop(pq);
    pq_destroy(pq);

    // ── Count by severity ─────────────────────────────────────────────
    int n_critical = 0, n_high = 0, n_medium = 0, n_low = 0;
    for (int i = 0; i < total; i++) {
        int s = buffer[i].severity;
        if (s == 5)      n_critical++;
        else if (s == 4) n_high++;
        else if (s == 3) n_medium++;
        else             n_low++;
    }

    // ── Stability Score ───────────────────────────────────────────────
    int raw_score = 100 - (n_critical * 10 + n_high * 5 + n_medium * 2 + n_low * 1);
    if (raw_score < 0) raw_score = 0;

    const char* score_color = raw_score >= 80 ? ANSI_GREEN :
                              raw_score >= 50 ? ANSI_YELLOW : ANSI_RED;

    // ════════════════════════════════════════════════════════════════════
    // SUMMARY HEADER
    // ════════════════════════════════════════════════════════════════════
    printf("\n%s", ANSI_BOLD);
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              CODE QUALITY DASHBOARD                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝%s\n\n", ANSI_RESET);

    printf("  %s📊 SUMMARY%s\n", ANSI_CYAN, ANSI_RESET);
    printf("  %-22s %s%d%s\n", "Total Issues:",   ANSI_BOLD, total,      ANSI_RESET);
    printf("  %-22s %s%s%d%s\n", "Critical (✖):", ANSI_RED,    ANSI_BOLD, n_critical, ANSI_RESET);
    printf("  %-22s %s%s%d%s\n", "High (!):    ", ANSI_ORANGE, ANSI_BOLD, n_high,     ANSI_RESET);
    printf("  %-22s %s%s%d%s\n", "Medium (~):  ", ANSI_YELLOW, ANSI_BOLD, n_medium,   ANSI_RESET);
    printf("  %-22s %s%s%d%s\n", "Low (·):     ", ANSI_BLUE,   ANSI_BOLD, n_low,      ANSI_RESET);
    printf("\n  %sStability Score:%s %s%s%d%%%s\n\n",
           ANSI_GRAY, ANSI_RESET, score_color, ANSI_BOLD, raw_score, ANSI_RESET);

    if (total == 0) {
        printf("  %s✓ No code quality issues detected. Clean code!%s\n\n", ANSI_GREEN, ANSI_RESET);
        return;
    }

    // ── Print grouped sections ────────────────────────────────────────
    int groups[] = { 5, 4, 3, 2, 1 };
    const char* group_labels[] = {
        "  ✖  CRITICAL  —  Immediate action required",
        "  !  HIGH      —  Should be fixed soon",
        "  ~  MEDIUM    —  Moderate risk, plan a fix",
        "  ·  LOW       —  Minor, informational",
        "  ·  INFO",
    };

    for (int g = 0; g < 5; g++) {
        int sev = groups[g];
        const char* col = severity_color(sev);

        // Check if any issues in this group
        int has = 0;
        for (int i = 0; i < total; i++) if (buffer[i].severity == sev) { has = 1; break; }
        if (!has) continue;

        // Section header
        printf("\n%s%s%s%s\n", col, ANSI_BOLD, group_labels[g], ANSI_RESET);
        printf("%s════════════════════════════════════════════════════════════%s\n",
               col, ANSI_RESET);

        for (int i = 0; i < total; i++) {
            if (buffer[i].severity == sev)
                print_issue_card(&buffer[i]);
        }

        printf("%s────────────────────────────────────────────────────────────%s\n",
               ANSI_GRAY, ANSI_RESET);
    }

    printf("\n%s  ℹ  Total %d issue(s) found. Fix CRITICAL and HIGH first.%s\n\n",
           ANSI_GRAY, total, ANSI_RESET);
}
