// hashtable.h
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "utils.h"

typedef struct HTNode {
    SymbolInfo info;
    struct HTNode* next;
} HTNode;

typedef struct {
    int size;
    int count;          
    HTNode** buckets;
} HashTable;

HashTable* ht_create(int size);
void ht_destroy(HashTable* ht);
unsigned long ht_hash(const char* str);
void ht_insert(HashTable* ht, SymbolInfo* info);
SymbolInfo* ht_search(HashTable* ht, const char* name);
void ht_display(HashTable* ht);


void ht_display_table(HashTable* ht);
void ht_display_by_category(HashTable* ht, const char* category);
void ht_display_by_type(HashTable* ht, const char* type);
void ht_display_by_line_range(HashTable* ht, int start_line, int end_line);


int ht_find_all_by_category(HashTable* ht, const char* category, 
                            SymbolInfo results[], int max_results);
int ht_find_all_by_type(HashTable* ht, const char* type,
                       SymbolInfo results[], int max_results);
SymbolInfo* ht_find_at_line(HashTable* ht, int line_number);
int ht_delete(HashTable* ht, const char* name);


void ht_print_stats(HashTable* ht);
void ht_export_to_csv(HashTable* ht, const char* filename);
int ht_get_count(HashTable* ht);

#endif // HASHTABLE_H