// hashtable.c
#include "hashtable.h"

HashTable* ht_create(int size) {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) {
        fprintf(stderr, "Error: Failed to allocate hash table\n");
        return NULL;
    }
    ht->size = size;
    ht->count = 0;
    ht->buckets = (HTNode**)calloc(size, sizeof(HTNode*));
    if (!ht->buckets) {
        fprintf(stderr, "Error: Failed to allocate buckets\n");
        free(ht);
        return NULL;
    }
    return ht;
}

void ht_destroy(HashTable* ht) {
    if (!ht) return;
    for (int i = 0; i < ht->size; ++i) {
        HTNode* cur = ht->buckets[i];
        while (cur) {
            HTNode* tmp = cur;
            cur = cur->next;
            free(tmp);
        }
    }
    free(ht->buckets);
    free(ht);
}

unsigned long ht_hash(const char* str) {
   
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

void ht_insert(HashTable* ht, SymbolInfo* info) {
    if (!ht || !info) return;
    
    unsigned long h = ht_hash(info->name) % ht->size;
 
    HTNode* cur = ht->buckets[h];
    while (cur) {
        if (strcmp(cur->info.name, info->name) == 0) {
            cur->info = *info; 
            return;
        }
        cur = cur->next;
    }
  
    HTNode* node = (HTNode*)malloc(sizeof(HTNode));
    if (!node) {
        fprintf(stderr, "Error: Failed to allocate hash node\n");
        return;
    }
    node->info = *info;
    node->next = ht->buckets[h];
    ht->buckets[h] = node;
    ht->count++;
}

SymbolInfo* ht_search(HashTable* ht, const char* name) {
    if (!ht || !name) return NULL;
    unsigned long h = ht_hash(name) % ht->size;
    HTNode* cur = ht->buckets[h];
    while (cur) {
        if (strcmp(cur->info.name, name) == 0) {
            return &cur->info;
        }
        cur = cur->next;
    }
    return NULL;
}

int ht_delete(HashTable* ht, const char* name) {
    if (!ht || !name) return 0;
    
    unsigned long h = ht_hash(name) % ht->size;
    HTNode* cur = ht->buckets[h];
    HTNode* prev = NULL;
    
    while (cur) {
        if (strcmp(cur->info.name, name) == 0) {
            if (prev) {
                prev->next = cur->next;
            } else {
                ht->buckets[h] = cur->next;
            }
            free(cur);
            ht->count--;
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

void ht_display(HashTable* ht) {
    if (!ht) return;
    for (int i = 0; i < ht->size; ++i) {
        HTNode* cur = ht->buckets[i];
        while (cur) {
            SymbolInfo* s = &cur->info;
            printf("%s - %s - %s - Line %d\n", s->name, s->type, s->category, s->line);
            cur = cur->next;
        }
    }
}


static int collect_and_sort_symbols(HashTable* ht, SymbolInfo** out_symbols) {
    if (!ht || ht->count == 0) return 0;
    
    SymbolInfo* symbols = (SymbolInfo*)malloc(ht->count * sizeof(SymbolInfo));
    if (!symbols) return 0;
    
    int idx = 0;
    for (int i = 0; i < ht->size; i++) {
        HTNode* cur = ht->buckets[i];
        while (cur) {
            symbols[idx++] = cur->info;
            cur = cur->next;
        }
    }
    

    for (int i = 0; i < ht->count - 1; i++) {
        for (int j = 0; j < ht->count - i - 1; j++) {
            if (symbols[j].line > symbols[j + 1].line) {
                SymbolInfo temp = symbols[j];
                symbols[j] = symbols[j + 1];
                symbols[j + 1] = temp;
            }
        }
    }
    
    *out_symbols = symbols;
    return ht->count;
}

void ht_display_table(HashTable* ht) {
    if (!ht || ht->count == 0) {
        printf("Symbol table is empty.\n");
        return;
    }
    
    SymbolInfo* symbols;
    int count = collect_and_sort_symbols(ht, &symbols);
    if (!symbols) return;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                          SYMBOL TABLE                                      ║\n");
    printf("╠═══════════════════╦═══════════════════╦═══════════════╦═══════════════════╣\n");
    printf("║ %-17s ║ %-17s ║ %-13s ║ %-17s ║\n", "Name", "Type", "Category", "Line");
    printf("╠═══════════════════╬═══════════════════╬═══════════════╬═══════════════════╣\n");
    
    for (int i = 0; i < count; i++) {
        printf("║ %-17.17s ║ %-17.17s ║ %-13.13s ║ %-17d ║\n",
               symbols[i].name,
               symbols[i].type,
               symbols[i].category,
               symbols[i].line);
    }
    
    printf("╚═══════════════════╩═══════════════════╩═══════════════╩═══════════════════╝\n");
    printf("Total symbols: %d\n\n", count);
    
    free(symbols);
}

void ht_display_by_category(HashTable* ht, const char* category) {
    if (!ht || !category) return;
    
    
    int match_count = 0;
    for (int i = 0; i < ht->size; i++) {
        HTNode* cur = ht->buckets[i];
        while (cur) {
            if (strcmp(cur->info.category, category) == 0) {
                match_count++;
            }
            cur = cur->next;
        }
    }
    
    if (match_count == 0) {
        printf("No symbols found with category '%s'\n", category);
        return;
    }
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                  SYMBOLS - Category: %-37s║\n", category);
    printf("╠═══════════════════╦═══════════════════╦═══════════════════════════════════╣\n");
    printf("║ %-17s ║ %-17s ║ %-37s ║\n", "Name", "Type", "Line");
    printf("╠═══════════════════╬═══════════════════╬═══════════════════════════════════╣\n");
    
   
    SymbolInfo* matches = (SymbolInfo*)malloc(match_count * sizeof(SymbolInfo));
    int idx = 0;
    for (int i = 0; i < ht->size; i++) {
        HTNode* cur = ht->buckets[i];
        while (cur) {
            if (strcmp(cur->info.category, category) == 0) {
                matches[idx++] = cur->info;
            }
            cur = cur->next;
        }
    }
    

    for (int i = 0; i < match_count - 1; i++) {
        for (int j = 0; j < match_count - i - 1; j++) {
            if (matches[j].line > matches[j + 1].line) {
                SymbolInfo temp = matches[j];
                matches[j] = matches[j + 1];
                matches[j + 1] = temp;
            }
        }
    }
    
   
    for (int i = 0; i < match_count; i++) {
        printf("║ %-17.17s ║ %-17.17s ║ %-37d ║\n",
               matches[i].name,
               matches[i].type,
               matches[i].line);
    }
    
    printf("╚═══════════════════╩═══════════════════╩═══════════════════════════════════╝\n");
    printf("Total: %d\n\n", match_count);
    
    free(matches);
}

void ht_display_by_type(HashTable* ht, const char* type) {
    if (!ht || !type) return;
    

    int match_count = 0;
    for (int i = 0; i < ht->size; i++) {
        HTNode* cur = ht->buckets[i];
        while (cur) {
            if (strcmp(cur->info.type, type) == 0) {
                match_count++;
            }
            cur = cur->next;
        }
    }
    
    if (match_count == 0) {
        printf("No symbols found with type '%s'\n", type);
        return;
    }
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    SYMBOLS - Type: %-39s║\n", type);
    printf("╠═══════════════════╦═══════════════════╦═══════════════════════════════════╣\n");
    printf("║ %-17s ║ %-17s ║ %-37s ║\n", "Name", "Category", "Line");
    printf("╠═══════════════════╬═══════════════════╬═══════════════════════════════════╣\n");
    

    SymbolInfo* matches = (SymbolInfo*)malloc(match_count * sizeof(SymbolInfo));
    int idx = 0;
    for (int i = 0; i < ht->size; i++) {
        HTNode* cur = ht->buckets[i];
        while (cur) {
            if (strcmp(cur->info.type, type) == 0) {
                matches[idx++] = cur->info;
            }
            cur = cur->next;
        }
    }
    
 
    for (int i = 0; i < match_count - 1; i++) {
        for (int j = 0; j < match_count - i - 1; j++) {
            if (matches[j].line > matches[j + 1].line) {
                SymbolInfo temp = matches[j];
                matches[j] = matches[j + 1];
                matches[j + 1] = temp;
            }
        }
    }
    
    // Display
    for (int i = 0; i < match_count; i++) {
        printf("║ %-17.17s ║ %-17.17s ║ %-37d ║\n",
               matches[i].name,
               matches[i].category,
               matches[i].line);
    }
    
    printf("╚═══════════════════╩═══════════════════╩═══════════════════════════════════╝\n");
    printf("Total: %d\n\n", match_count);
    
    free(matches);
}

void ht_display_by_line_range(HashTable* ht, int start_line, int end_line) {
    if (!ht || start_line > end_line) return;
    
   
    int match_count = 0;
    for (int i = 0; i < ht->size; i++) {
        HTNode* cur = ht->buckets[i];
        while (cur) {
            if (cur->info.line >= start_line && cur->info.line <= end_line) {
                match_count++;
            }
            cur = cur->next;
        }
    }
    
    if (match_count == 0) {
        printf("No symbols found between lines %d-%d\n", start_line, end_line);
        return;
    }
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                  SYMBOLS - Lines %d to %-34d║\n", start_line, end_line);
    printf("╠═══════════════════╦═══════════════════╦═══════════════╦═══════════════════╣\n");
    printf("║ %-17s ║ %-17s ║ %-13s ║ %-17s ║\n", "Name", "Type", "Category", "Line");
    printf("╠═══════════════════╬═══════════════════╬═══════════════╬═══════════════════╣\n");
    
   
    SymbolInfo* matches = (SymbolInfo*)malloc(match_count * sizeof(SymbolInfo));
    int idx = 0;
    for (int i = 0; i < ht->size; i++) {
        HTNode* cur = ht->buckets[i];
        while (cur) {
            if (cur->info.line >= start_line && cur->info.line <= end_line) {
                matches[idx++] = cur->info;
            }
            cur = cur->next;
        }
    }
    

    for (int i = 0; i < match_count - 1; i++) {
        for (int j = 0; j < match_count - i - 1; j++) {
            if (matches[j].line > matches[j + 1].line) {
                SymbolInfo temp = matches[j];
                matches[j] = matches[j + 1];
                matches[j + 1] = temp;
            }
        }
    }
    
    for (int i = 0; i < match_count; i++) {
        printf("║ %-17.17s ║ %-17.17s ║ %-13.13s ║ %-17d ║\n",
               matches[i].name,
               matches[i].type,
               matches[i].category,
               matches[i].line);
    }
    
    printf("╚═══════════════════╩═══════════════════╩═══════════════╩═══════════════════╝\n");
    printf("Total: %d\n\n", match_count);
    
    free(matches);
}

int ht_find_all_by_category(HashTable* ht, const char* category, 
                            SymbolInfo results[], int max_results) {
    if (!ht || !category || !results) return 0;
    
    int count = 0;
    for (int i = 0; i < ht->size && count < max_results; i++) {
        HTNode* cur = ht->buckets[i];
        while (cur && count < max_results) {
            if (strcmp(cur->info.category, category) == 0) {
                results[count++] = cur->info;
            }
            cur = cur->next;
        }
    }
    return count;
}

int ht_find_all_by_type(HashTable* ht, const char* type,
                       SymbolInfo results[], int max_results) {
    if (!ht || !type || !results) return 0;
    
    int count = 0;
    for (int i = 0; i < ht->size && count < max_results; i++) {
        HTNode* cur = ht->buckets[i];
        while (cur && count < max_results) {
            if (strcmp(cur->info.type, type) == 0) {
                results[count++] = cur->info;
            }
            cur = cur->next;
        }
    }
    return count;
}

SymbolInfo* ht_find_at_line(HashTable* ht, int line_number) {
    if (!ht) return NULL;
    
    for (int i = 0; i < ht->size; i++) {
        HTNode* cur = ht->buckets[i];
        while (cur) {
            if (cur->info.line == line_number) {
                return &cur->info;
            }
            cur = cur->next;
        }
    }
    return NULL;
}

void ht_print_stats(HashTable* ht) {
    if (!ht) return;
    
    int empty_buckets = 0;
    int max_chain = 0;
    int total_chain = 0;
    int non_empty = 0;
    
    for (int i = 0; i < ht->size; i++) {
        int chain_len = 0;
        HTNode* cur = ht->buckets[i];
        
        if (!cur) {
            empty_buckets++;
        } else {
            non_empty++;
            while (cur) {
                chain_len++;
                cur = cur->next;
            }
            total_chain += chain_len;
            if (chain_len > max_chain) {
                max_chain = chain_len;
            }
        }
    }
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║            HASH TABLE STATISTICS                       ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Total buckets:          %6d                       ║\n", ht->size);
    printf("║ Total symbols:          %6d                       ║\n", ht->count);
    printf("║ Empty buckets:          %6d (%.1f%%)              ║\n", 
           empty_buckets, 100.0 * empty_buckets / ht->size);
    printf("║ Load factor:            %6.2f                       ║\n", 
           (float)ht->count / ht->size);
    printf("║ Avg chain length:       %6.2f                       ║\n",
           non_empty > 0 ? (float)total_chain / non_empty : 0.0);
    printf("║ Max chain length:       %6d                       ║\n", max_chain);
    printf("║ Memory usage:           ~%5zu KB                   ║\n",
           (sizeof(HashTable) + ht->size * sizeof(HTNode*) + 
            ht->count * sizeof(HTNode)) / 1024);
    printf("╚════════════════════════════════════════════════════════╝\n\n");
}

void ht_export_to_csv(HashTable* ht, const char* filename) {
    if (!ht || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Error: Cannot open %s for writing\n", filename);
        return;
    }
    
    fprintf(f, "Name,Type,Category,Line\n");
    
    SymbolInfo* symbols;
    int count = collect_and_sort_symbols(ht, &symbols);
    
    if (symbols) {
        for (int i = 0; i < count; i++) {
            fprintf(f, "%s,%s,%s,%d\n",
                    symbols[i].name,
                    symbols[i].type,
                    symbols[i].category,
                    symbols[i].line);
        }
        free(symbols);
    }
    
    fclose(f);
    printf("Symbol table exported to %s (%d symbols)\n", filename, count);
}

int ht_get_count(HashTable* ht) {
    return ht ? ht->count : 0;
}