#include "trie.h"

static inline int char_index(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return 26 + (c - 'A');
    if (c >= '0' && c <= '9') return 52 + (c - '0');
    if (c == '_') return 62;
    return -1;
}

static inline char index_to_char(int idx) {
    if (idx < 26) return 'a' + idx;
    if (idx < 52) return 'A' + (idx - 26);
    if (idx < 62) return '0' + (idx - 52);
    return '_';
}

TrieNode* trie_create_node() {
    TrieNode* node = (TrieNode*)calloc(1, sizeof(TrieNode));
    if (!node) {
        fprintf(stderr, "Error: Failed to allocate trie node\n");
        return NULL;
    }
    node->type[0] = '\0';
    node->scope[0] = '\0';
    node->line_number = 0;
    return node;
}

void trie_insert(TrieNode* root, const char* word) {
    trie_insert_with_metadata(root, word, "unknown", "global", 0);
}

static int trie_count_end_nodes(TrieNode* node) {
    if (!node) return 0;
    int cnt = node->is_end ? 1 : 0;
    for (int i = 0; i < 63; ++i) {
        if (node->children[i]) cnt += trie_count_end_nodes(node->children[i]);
    }
    return cnt;
}

void trie_insert_with_metadata(TrieNode* root, const char* word, const char* type, const char* scope, int line_number) {
    if (!root || !word || word[0] == '\0') return;
    TrieNode* cur = root;
    int len = strlen(word);
    for (int i = 0; i < len; ++i) {
        int idx = char_index(word[i]);
        if (idx < 0) {
            fprintf(stderr, "Warning: Invalid character in '%s', skipping\n", word);
            return;
        }
    }
    for (int i = 0; i < len; ++i) {
        int idx = char_index(word[i]);
        if (!cur->children[idx]) {
            cur->children[idx] = trie_create_node();
            if (!cur->children[idx]) return;
        }
        cur = cur->children[idx];
        cur->word_count++;
    }
    if (!cur->is_end) {
        cur->is_end = 1;
        if (root) root->word_count++;
    }
    if (type) strncpy(cur->type, type, MAX_TYPE - 1);
    if (scope) strncpy(cur->scope, scope, MAX_SCOPE - 1);
    cur->line_number = line_number;
}

int trie_search(TrieNode* root, const char* word) {
    if (!root || !word) return 0;
    TrieNode* cur = root;
    for (int i = 0; word[i]; ++i) {
        int idx = char_index(word[i]);
        if (idx < 0 || !cur->children[idx]) return 0;
        cur = cur->children[idx];
    }
    return cur->is_end;
}

int trie_has_prefix(TrieNode* root, const char* prefix) {
    if (!root || !prefix) return 0;
    TrieNode* cur = root;
    for (int i = 0; prefix[i]; ++i) {
        int idx = char_index(prefix[i]);
        if (idx < 0 || !cur->children[idx]) return 0;
        cur = cur->children[idx];
    }
    return 1;
}

TrieNode* trie_find_node(TrieNode* root, const char* word) {
    if (!root || !word) return NULL;
    TrieNode* cur = root;
    for (int i = 0; word[i]; ++i) {
        int idx = char_index(word[i]);
        if (idx < 0 || !cur->children[idx]) return NULL;
        cur = cur->children[idx];
    }
    return cur->is_end ? cur : NULL;
}

static void trie_collect_optimized(TrieNode* node, char* buffer, int depth, char results[][MAX_NAME], int* count, int max_results) {
    if (!node || *count >= max_results) return;
    if (node->is_end) {
        buffer[depth] = '\0';
        strncpy(results[*count], buffer, MAX_NAME - 1);
        results[*count][MAX_NAME - 1] = '\0';
        (*count)++;
        if (*count >= max_results) return;
    }
    for (int i = 0; i < 63; ++i) {
        if (node->children[i]) {
            buffer[depth] = index_to_char(i);
            trie_collect_optimized(node->children[i], buffer, depth + 1, results, count, max_results);
            if (*count >= max_results) return;
        }
    }
}

void trie_collect(TrieNode* node, char* buffer, int depth, char results[][MAX_NAME], int* count, int max_results) {
    trie_collect_optimized(node, buffer, depth, results, count, max_results);
}

void trie_autocomplete(TrieNode* root, const char* prefix, char results[][MAX_NAME], int* count, int max_results) {
    *count = 0;
    if (!root || !prefix) return;
    TrieNode* cur = root;
    int plen = strlen(prefix);
    for (int i = 0; i < plen; ++i) {
        int idx = char_index(prefix[i]);
        if (idx < 0 || !cur->children[idx]) return;
        cur = cur->children[idx];
    }
    if (cur->word_count == 0 && !cur->is_end) return;
    char buffer[256];
    strncpy(buffer, prefix, plen);
    buffer[plen] = '\0';
    trie_collect_optimized(cur, buffer, plen, results, count, max_results);
}

static void print_words_with_metadata(TrieNode* node, char* buffer, int depth) {
    if (!node) return;
    if (node->is_end) {
        buffer[depth] = '\0';
        printf("  %-20s | %-15s | %-15s | Line %d\n", buffer, node->type, node->scope, node->line_number);
    }
    for (int i = 0; i < 63; ++i) {
        if (node->children[i]) {
            buffer[depth] = index_to_char(i);
            print_words_with_metadata(node->children[i], buffer, depth + 1);
        }
    }
}

void trie_autocomplete_with_metadata(TrieNode* root, const char* prefix) {
    if (!root || !prefix) {
        printf("No suggestions available.\n");
        return;
    }
    TrieNode* cur = root;
    int plen = strlen(prefix);
    for (int i = 0; i < plen; ++i) {
        int idx = char_index(prefix[i]);
        if (idx < 0 || !cur->children[idx]) {
            printf("No matches found for prefix '%s'\n", prefix);
            return;
        }
        cur = cur->children[idx];
    }
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║              Autocomplete Results for: %-35s║\n", prefix);
    printf("╠════════════════════════════════════════════════════════════════════════════╣\n");
    printf("  %-20s | %-15s | %-15s | %s\n", "Identifier", "Type", "Scope", "Line");
    printf("  ────────────────────────────────────────────────────────────────────────────\n");
    char buffer[256];
    strncpy(buffer, prefix, plen);
    print_words_with_metadata(cur, buffer, plen);
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n\n");
}

static void collect_detailed(TrieNode* node, char* buffer, int depth, SymbolInfo results[], int* count, int max_results) {
    if (!node || *count >= max_results) return;
    if (node->is_end) {
        buffer[depth] = '\0';
        strncpy(results[*count].name, buffer, MAX_NAME - 1);
        strncpy(results[*count].type, node->type, MAX_TYPE - 1);
        strncpy(results[*count].category, node->scope, MAX_CATEGORY - 1);
        results[*count].line = node->line_number;
        (*count)++;
        if (*count >= max_results) return;
    }
    for (int i = 0; i < 63; ++i) {
        if (node->children[i]) {
            buffer[depth] = index_to_char(i);
            collect_detailed(node->children[i], buffer, depth + 1, results, count, max_results);
            if (*count >= max_results) return;
        }
    }
}

void trie_autocomplete_detailed(TrieNode* root, const char* prefix, SymbolInfo results[], int* count, int max_results) {
    *count = 0;
    if (!root || !prefix || !results) return;
    TrieNode* cur = root;
    int plen = strlen(prefix);
    for (int i = 0; i < plen; ++i) {
        int idx = char_index(prefix[i]);
        if (idx < 0 || !cur->children[idx]) return;
        cur = cur->children[idx];
    }
    char buffer[256];
    strncpy(buffer, prefix, plen);
    buffer[plen] = '\0';
    collect_detailed(cur, buffer, plen, results, count, max_results);
}

int trie_count_words_with_prefix(TrieNode* root, const char* prefix) {
    if (!root || !prefix) return 0;
    TrieNode* cur = root;
    for (int i = 0; prefix[i]; ++i) {
        int idx = char_index(prefix[i]);
        if (idx < 0 || !cur->children[idx]) return 0;
        cur = cur->children[idx];
    }
    return cur->word_count;
}

void trie_autocomplete_ranked(TrieNode* root, const char* prefix, char results[][MAX_NAME], int* count, int max_results) {
    trie_autocomplete(root, prefix, results, count, max_results);
}

int trie_delete(TrieNode* root, const char* word) {
    if (!root || !word || word[0] == '\0') return 0;
    TrieNode* path[256];
    int indices[256];
    int depth = 0;
    TrieNode* cur = root;
    for (int i = 0; word[i] && depth < 256; ++i) {
        int idx = char_index(word[i]);
        if (idx < 0 || !cur->children[idx]) return 0;
        path[depth] = cur;
        indices[depth] = idx;
        depth++;
        cur = cur->children[idx];
    }
    if (!cur->is_end) return 0;
    cur->is_end = 0;
    for (int i = depth - 1; i >= 0; --i) {
        TrieNode* node = path[i]->children[indices[i]];
        int has_children = 0;
        for (int j = 0; j < 63; ++j) {
            if (node->children[j]) {
                has_children = 1;
                break;
            }
        }
        if (!has_children && !node->is_end) {
            free(node);
            path[i]->children[indices[i]] = NULL;
        } else {
            break;
        }
    }
    return 1;
}

size_t trie_memory_usage(TrieNode* node) {
    if (!node) return 0;
    size_t total = sizeof(TrieNode);
    for (int i = 0; i < 63; ++i) {
        if (node->children[i]) {
            total += trie_memory_usage(node->children[i]);
        }
    }
    return total;
}

void trie_print_stats(TrieNode* root) {
    if (!root) {
        printf("Trie is empty\n");
        return;
    }
    size_t memory = trie_memory_usage(root);
    int total_words = trie_count_end_nodes(root);
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║                 TRIE STATISTICS                        ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Total words:            %6d                       ║\n", total_words);
    printf("║ Memory usage:           %6zu bytes (%.2f KB)      ║\n", memory, memory / 1024.0);
    printf("║ Avg bytes per word:     %6.2f                       ║\n", total_words > 0 ? (double)memory / total_words : 0.0);
    printf("╚════════════════════════════════════════════════════════╝\n\n");
}

void trie_display_all_with_metadata(TrieNode* root) {
    if (!root || root->word_count == 0) {
        printf("Trie is empty\n");
        return;
    }
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    ALL IDENTIFIERS IN TRIE                                 ║\n");
    printf("╠════════════════════════════════════════════════════════════════════════════╣\n");
    printf("  %-20s | %-15s | %-15s | %s\n", "Identifier", "Type", "Scope", "Line");
    printf("  ────────────────────────────────────────────────────────────────────────────\n");
    char buffer[256];
    print_words_with_metadata(root, buffer, 0);
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n");
    printf("Total: %d identifiers\n\n", root->word_count);
}

static void print_by_scope(TrieNode* node, char* buffer, int depth, const char* target_scope, int* count) {
    if (!node) return;
    if (node->is_end && strcmp(node->scope, target_scope) == 0) {
        buffer[depth] = '\0';
        printf("  %-20s | %-15s | Line %d\n", buffer, node->type, node->line_number);
        (*count)++;
    }
    for (int i = 0; i < 63; ++i) {
        if (node->children[i]) {
            buffer[depth] = index_to_char(i);
            print_by_scope(node->children[i], buffer, depth + 1, target_scope, count);
        }
    }
}

void trie_display_by_scope(TrieNode* root, const char* scope) {
    if (!root || !scope) return;
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║              Identifiers in Scope: %-39s║\n", scope);
    printf("╠════════════════════════════════════════════════════════════════════════════╣\n");
    printf("  %-20s | %-15s | %s\n", "Identifier", "Type", "Line");
    printf("  ────────────────────────────────────────────────────────────────────────────\n");
    int count = 0;
    char buffer[256];
    print_by_scope(root, buffer, 0, scope, &count);
    if (count == 0) printf("  (No identifiers found in this scope)\n");
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n");
    printf("Total: %d identifiers\n\n", count);
}

void trie_destroy(TrieNode* node) {
    if (!node) return;
    for (int i = 0; i < 63; ++i) {
        if (node->children[i]) {
            trie_destroy(node->children[i]);
        }
    }
    free(node);
}
