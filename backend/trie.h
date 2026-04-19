// trie.h
#ifndef TRIE_H
#define TRIE_H

#include "utils.h"

// Trie node with metadata support
typedef struct TrieNode {
    struct TrieNode* children[63];  // a-z(0-25), A-Z(26-51), 0-9(52-61), _(62)
    int is_end;                     // 1 if this node marks end of word
    int word_count;                 // count of words in subtree (optimization)
    
    // Metadata (only valid when is_end == 1)
    char type[MAX_TYPE];            // e.g., "int", "float", "void"
    char scope[MAX_SCOPE];          // e.g., "global", "main", "function_name"
    int line_number;                // line number where identifier appears
} TrieNode;

// Core operations
TrieNode* trie_create_node();
void trie_insert(TrieNode* root, const char* word);
void trie_insert_with_metadata(TrieNode* root, const char* word, 
                                const char* type, const char* scope, int line_number);
void trie_destroy(TrieNode* node);

// Search operations
int trie_search(TrieNode* root, const char* word);
int trie_has_prefix(TrieNode* root, const char* prefix);
TrieNode* trie_find_node(TrieNode* root, const char* word);

// Autocomplete operations
void trie_autocomplete(TrieNode* root, const char* prefix, 
                      char results[][MAX_NAME], int* count, int max_results);
void trie_autocomplete_with_metadata(TrieNode* root, const char* prefix);
void trie_autocomplete_detailed(TrieNode* root, const char* prefix, 
                                SymbolInfo results[], int* count, int max_results);

// Collection operations (for internal use and compatibility)
void trie_collect(TrieNode* node, char* buffer, int depth, 
                 char results[][MAX_NAME], int* count, int max_results);

// Advanced features
void trie_autocomplete_ranked(TrieNode* root, const char* prefix, 
                             char results[][MAX_NAME], int* count, int max_results);
int trie_count_words_with_prefix(TrieNode* root, const char* prefix);

// Memory management
int trie_delete(TrieNode* root, const char* word);
size_t trie_memory_usage(TrieNode* node);
void trie_print_stats(TrieNode* root);

// Display functions
void trie_display_all_with_metadata(TrieNode* root);
void trie_display_by_scope(TrieNode* root, const char* scope);

#endif // TRIE_H