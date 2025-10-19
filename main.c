
#include "utils.h"
#include "hashtable.h"
#include "trie.h"
#include "avltree.h"
#include "ast.h"
#include "parser.h"

#include <stdio.h>

#define HT_SIZE 1031
#define MAX_SUGGEST 50


static void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


static void print_header() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                        CodeMaster                              ║\n");
    printf("║                   C Code Analyzer & Symbol Browser             ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

static void print_menu() {
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│                         MAIN MENU                              │\n");
    printf("├────────────────────────────────────────────────────────────────┤\n");
    printf("│  1. Search symbol by name                                      │\n");
    printf("│  2. Autocomplete prefix                                        │\n");
    printf("│  3. Display all symbols (formatted table)                      │\n");
    printf("│  4. Display by category (function/variable/loop/condition)     │\n");
    printf("│  5. Display by type (int/float/char/etc)                       │\n");
    printf("│  6. Display symbols in line range                              │\n");
    printf("│  7. Show hash table statistics                                 │\n");
    printf("│  8. Show trie statistics                                       │\n");
    printf("│  9. Export symbol table to CSV                                 │\n");
    printf("│ 10. Check if word exists in trie                               │\n");
    printf("│ 11. Display AVL tree (sorted view)                             │\n");
    printf("│ 12. Show AVL tree statistics                                   │\n");
    printf("│ 13. Build and display AST                                      │\n");
//  printf("│ 14. Analyze AST (complexity & nesting depth)                   │\n");
   // printf("│ 15. Extract function call graph                                │\n");
    printf("├────────────────────────────────────────────────────────────────┤\n");
    printf("│  0. Exit                                                       │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n");
    printf("Enter choice: ");
}


static void wait_for_enter() {
    printf("\nPress ENTER to continue...");
    while (getchar() != '\n');
    getchar();
}

static void flush_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <source.c>\n", argv[0]);
        printf("Example: %s mycode.c\n", argv[0]);
        return 1;
    }
    
    const char* filename = argv[1];
    
    HashTable* ht = ht_create(HT_SIZE);
    if (!ht) {
        fprintf(stderr, "Error: Failed to create hash table\n");
        return 1;
    }
    
    TrieNode* trie = trie_create_node();
    if (!trie) {
        fprintf(stderr, "Error: Failed to create trie\n");
        ht_destroy(ht);
        return 1;
    }
    
    AVLNode* avl = NULL;    
    
    clear_screen();
    print_header();
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│ Parsing file: %-48s │\n", filename);
    printf("└────────────────────────────────────────────────────────────────┘\n");
    printf("\n[*] Analyzing C source code...\n");
    printf("[*] Extracting symbols...\n");
    printf("[*] Building symbol table, trie, and AVL tree...\n\n");
    
    parse_file_and_populate(filename, ht, trie, &avl);
    
    int symbol_count = ht_get_count(ht);
    printf("✓ Parsing complete!\n");
    printf("✓ Indexed %d symbols\n", symbol_count);
    
    if (symbol_count == 0) {
        printf("\n⚠ Warning: No symbols found in file.\n");
        printf("   Make sure the file contains valid C code.\n\n");
    }
    
    wait_for_enter();
    
    int running = 1;
    while (running) {
        clear_screen();
        print_header();
        print_menu();
        
        int choice = 0;
        if (scanf("%d", &choice) != 1) {
            flush_input();
            printf("\n⚠ Invalid input. Please enter a number.\n");
            wait_for_enter();
            continue;
        }
        flush_input();
        
        printf("\n");
        
        switch (choice) {
            case 1: { 
                char name[MAX_NAME];
                printf("Enter symbol name: ");
                if (scanf("%s", name) != 1) {
                    flush_input();
                    break;
                }
                flush_input();
                
                SymbolInfo* s = ht_search(ht, name);
                if (s) {
                    printf("\n┌────────────────────────────────────────────────────────────────┐\n");
                    printf("│                      SYMBOL FOUND                              │\n");
                    printf("├────────────────────────────────────────────────────────────────┤\n");
                    printf("│ Name:     %-52s │\n", s->name);
                    printf("│ Type:     %-52s │\n", s->type);
                    printf("│ Category: %-52s │\n", s->category);
                    printf("│ Line:     %-52d │\n", s->line);
                    printf("└────────────────────────────────────────────────────────────────┘\n");
                } else {
                    printf("✗ Symbol '%s' not found in symbol table.\n", name);
                }
                wait_for_enter();
                break;
            }
            
            case 2: { 
                char prefix[MAX_NAME];
                printf("Enter prefix: ");
                if (scanf("%s", prefix) != 1) {
                    flush_input();
                    break;
                }
                flush_input();
                
                char results[MAX_SUGGEST][MAX_NAME];
                int count = 0;
                trie_autocomplete(trie, prefix, results, &count, MAX_SUGGEST);
                
                if (count == 0) {
                    printf("\n✗ No suggestions found for '%s'\n", prefix);
                } else {
                    printf("\n┌────────────────────────────────────────────────────────────────┐\n");
                    printf("│ Autocomplete suggestions for: %-32s │\n", prefix);
                    printf("├────────────────────────────────────────────────────────────────┤\n");
                    for (int i = 0; i < count && i < 20; ++i) {
                        printf("│ %2d. %-58s │\n", i+1, results[i]);
                    }
                    if (count > 20) {
                        printf("│ ... and %d more                                              │\n", count - 20);
                    }
                    printf("└────────────────────────────────────────────────────────────────┘\n");
                    printf("Total: %d suggestions\n", count);
                }
                wait_for_enter();
                break;
            }
            
            case 3: {  
                ht_display_table(ht);
                wait_for_enter();
                break;
            }
            
            case 4: { 
                printf("Available categories:\n");
                printf("  - function\n");
                printf("  - variable\n");
                printf("  - loop\n");
                printf("  - condition\n");
                printf("\nEnter category: ");
                char category[MAX_CATEGORY];
                if (scanf("%s", category) != 1) {
                    flush_input();
                    break;
                }
                flush_input();
                
                ht_display_by_category(ht, category);
                wait_for_enter();
                break;
            }
            
            case 5: { 
                printf("Enter type (e.g., int, float, char, void): ");
                char type[MAX_TYPE];
                if (scanf("%s", type) != 1) {
                    flush_input();
                    break;
                }
                flush_input();
                
                ht_display_by_type(ht, type);
                wait_for_enter();
                break;
            }
            
            case 6: { 
                int start, end;
                printf("Enter start line: ");
                if (scanf("%d", &start) != 1) {
                    flush_input();
                    break;
                }
                printf("Enter end line: ");
                if (scanf("%d", &end) != 1) {
                    flush_input();
                    break;
                }
                flush_input();
                
                if (start > end) {
                    printf("\n✗ Start line must be <= end line\n");
                } else {
                    ht_display_by_line_range(ht, start, end);
                }
                wait_for_enter();
                break;
            }
            
            case 7: {    
                ht_print_stats(ht);
                wait_for_enter();
                break;
            }
            
            case 8: {  
                trie_print_stats(trie);
                wait_for_enter();
                break;
            }
            
            case 9: { 
                char csv_filename[256];
                printf("Enter output filename (e.g., symbols.csv): ");
                if (scanf("%s", csv_filename) != 1) {
                    flush_input();
                    break;
                }
                flush_input();
                
                ht_export_to_csv(ht, csv_filename);
                wait_for_enter();
                break;
            }
            
            case 10: {     
                char word[MAX_NAME];
                printf("Enter word to check: ");
                if (scanf("%s", word) != 1) {
                    flush_input();
                    break;
                }
                flush_input();
                
                if (trie_search(trie, word)) {
                    printf("\n✓ '%s' exists in the trie\n", word);
                } else {
                    printf("\n✗ '%s' not found in the trie\n", word);
                }
                
                SymbolInfo* sym = ht_search(ht, word);
                if (sym) {
                    printf("✓ '%s' found in hash table (Line %d, Type: %s)\n", 
                           word, sym->line, sym->type);
                }
                wait_for_enter();
                break;
            }
            
            case 11: {    
                avl_display_sorted(avl);
                wait_for_enter();
                break;
            }
            
            case 12: { 
                avl_print_stats(avl);
                wait_for_enter();
                break;
            }



            case 13: { 
                printf("Building Abstract Syntax Tree...\n");
                ASTNode* ast = parse_file_to_ast(filename);
                
                if (ast) {
                    printf("\n┌────────────────────────────────────────────────────────────────┐\n");
                    printf("│                    AST STRUCTURE                                 │\n");
                    printf("└────────────────────────────────────────────────────────────────┘\n\n");
                    printAST(ast, 0);
                    
                    printf("\n┌────────────────────────────────────────────────────────────────┐\n");
                    printf("│                  AST DETAILED VIEW                               │\n");
                    printf("└────────────────────────────────────────────────────────────────┘\n\n");
                    printASTDetailed(ast, 0);
                    
                    printf("\nTotal AST nodes: %d\n", countNodes(ast));
                    freeAST(ast);
                } else {
                    printf("✗ Failed to build AST\n");
                }
                wait_for_enter();
                break;
            }
            
            case 14: {
                printf("Analyzing AST for metrics...\n");
                ASTNode* ast = parse_file_to_ast(filename);
                
                if (ast) {
                    analyze_ast(ast);
                    freeAST(ast);
                } else {
                    printf("✗ Failed to analyze AST\n");
                }
                wait_for_enter();
                break;
            }
            
            case 15: { 
                printf("Extracting function call graph...\n");
                ASTNode* ast = parse_file_to_ast(filename);
                
                if (ast) {
                    char** calls = (char**)malloc(100 * sizeof(char*));
                    int count = 0;
                    findFunctionCalls(ast, &calls, &count);
                    
                    printf("\n┌────────────────────────────────────────────────────────────────┐\n");
                    printf("│                  FUNCTION CALLS FOUND                            │\n");
                    printf("└────────────────────────────────────────────────────────────────┘\n\n");
                    
                    if (count == 0) {
                        printf("No function calls detected\n");
                    } else {
                        for (int i = 0; i < count; i++) {
                            printf("%d. %s\n", i+1, calls[i]);
                            free(calls[i]);
                        }
                    }
                    
                    free(calls);
                    freeAST(ast);
                } else {
                    printf("✗ Failed to extract call graph\n");
                }
                wait_for_enter();
                break;
            }

            
            case 0: { 
                printf("Thank you for using CodeMaster!\n");
                printf("Cleaning up...\n");
                running = 0;
                break;
            }
            
            default: {
                printf("⚠ Invalid choice. Please select 0-12.\n");
                wait_for_enter();
                break;
            }
        }
    } 
    
    ht_destroy(ht);
    trie_destroy(trie);
    avl_destroy(avl);
    
    printf("✓ Cleanup complete. Goodbye!\n\n");
    return 0;
}