#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include "parser.h"
#include "avltree.h"
#include "ast.h"
#include "graph.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "lexer.h"



static const char* basic_types[] = {
    "int","float","double","char","long","short","void","unsigned","signed","size_t",
    "struct","union","enum","const","static","extern","typedef"
};
static int n_basic_types = sizeof(basic_types)/sizeof(basic_types[0]);

static int is_type_token(const char* tok) {
    for (int i = 0; i < n_basic_types; ++i) {
        if (strcmp(tok, basic_types[i]) == 0) return 1;
    }
    return 0;
}

static void trim(char* s) {
    if (!s) return;
    int i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i) memmove(s, s + i, strlen(s + i) + 1);
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';
}

static void strip_comments(char* line) {
    char* comment = strstr(line, "//");
    if (comment) *comment = '\0';
    char* start = strstr(line, "/*");
    if (start) {
        char* end = strstr(start, "*/");
        if (end) {
            memmove(start, end + 2, strlen(end + 2) + 1);
        } else {
            *start = '\0';
        }
    }
}

static int is_in_string_context(const char* line, int pos) {
    int in_string = 0;
    int in_char = 0;
    for (int i = 0; i < pos && line[i]; i++) {
        if (line[i] == '"' && (i == 0 || line[i-1] != '\\')) in_string = !in_string;
        if (line[i] == '\'' && (i == 0 || line[i-1] != '\\')) in_char = !in_char;
    }
    return in_string || in_char;
}

static int looks_like_function_header(const char* line) {
    const char* lparen = strchr(line, '(');
    const char* rparen = strrchr(line, ')');
    if (!lparen || !rparen || lparen >= rparen) return 0;
    if (strchr(line, ';')) return 0;
    if (is_in_string_context(line, lparen - line)) return 0;
    char before_paren[256];
    int len = lparen - line;
    if (len >= 256) len = 255;
    strncpy(before_paren, line, len);
    before_paren[len] = '\0';
    trim(before_paren);
    if (strlen(before_paren) == 0) return 0;
    char after[64];
    strncpy(after, rparen + 1, 63);
    after[63] = '\0';
    trim(after);
    if (strlen(after) > 0 && after[0] != '{') return 0;
    return 1;
}

static void extract_name_from_function(const char* line, char* out_name, char* out_type) {
    out_name[0] = '\0';
    out_type[0] = '\0';
    const char* lparen = strchr(line, '(');
    if (!lparen) return;
    int left_len = lparen - line;
    char left[512];
    if (left_len >= 512) left_len = 511;
    strncpy(left, line, left_len);
    left[left_len] = '\0';
    trim(left);
    char tokens[32][MAX_NAME];
    int token_count = 0;
    char* temp = strdup(left);
    char* tok = strtok(temp, " \t*&");
    while (tok && token_count < 32) {
        if (strlen(tok) > 0) {
            strncpy(tokens[token_count++], tok, MAX_NAME-1);
            tokens[token_count-1][MAX_NAME-1] = '\0';
        }
        tok = strtok(NULL, " \t*&");
    }
    if (token_count > 0) {
        strncpy(out_name, tokens[token_count-1], MAX_NAME-1);
        char type_buf[MAX_TYPE] = "";
        for (int i = 0; i < token_count - 1; i++) {
            if (i > 0) strcat(type_buf, " ");
            strncat(type_buf, tokens[i], MAX_TYPE - strlen(type_buf) - 1);
        }
        if (strlen(type_buf) > 0) strncpy(out_type, type_buf, MAX_TYPE-1);
        else strncpy(out_type, "unknown", MAX_TYPE-1);
    }
    free(temp);
}

static void extract_variable_from_line(const char* line, char* out_name, char* out_type) {
    out_name[0] = '\0';
    out_type[0] = '\0';
    char copy[1024];
    strncpy(copy, line, 1023);
    copy[1023] = '\0';
    trim(copy);
    int L = strlen(copy);
    if (L && copy[L-1] == ';') copy[L-1] = '\0';
    trim(copy);
    if (strchr(copy, '(')) return;
    //char* saveptr;
    char* token = strtok(copy, " \t*&");
    if (!token) return;
    char type_buf[MAX_TYPE] = "";
    int found_type = 0;
    while (token) {
        if (is_type_token(token)) {
            if (strlen(type_buf) > 0) strcat(type_buf, " ");
            strncat(type_buf, token, MAX_TYPE - strlen(type_buf) - 1);
            found_type = 1;
           token = strtok(NULL, " \t*&");
        } else if (found_type) break;
        else return;
    }
    if (!found_type || !token) return;
    strncpy(out_type, type_buf, MAX_TYPE-1);
    char namebuf[MAX_NAME] = "";
    int i = 0;
    while (token[i] && token[i] != '=' && token[i] != ',' && token[i] != '[') {
        namebuf[i] = token[i];
        i++;
    }
    namebuf[i] = '\0';
    trim(namebuf);
    if (strlen(namebuf) > 0) strncpy(out_name, namebuf, MAX_NAME-1);
}

static int detect_loop(const char* line, char* loop_type) {
    char temp[1024];
    strncpy(temp, line, 1023);
    temp[1023] = '\0';
    trim(temp);
    if (strstr(temp, "for") && strchr(temp, '(')) {
        const char* pos = strstr(temp, "for");
        if ((pos == temp || !isalnum((unsigned char)pos[-1])) && !isalnum((unsigned char)pos[3])) {
            strcpy(loop_type, "for");
            return 1;
        }
    }
    if (strstr(temp, "while") && strchr(temp, '(')) {
        const char* pos = strstr(temp, "while");
        if ((pos == temp || !isalnum((unsigned char)pos[-1])) && !isalnum((unsigned char)pos[5])) {
            strcpy(loop_type, "while");
            return 1;
        }
    }
    if (strstr(temp, "do") && (strstr(temp, "{") || strlen(temp) == 2)) {
        const char* pos = strstr(temp, "do");
        if ((pos == temp || !isalnum((unsigned char)pos[-1])) && !isalnum((unsigned char)pos[2])) {
            strcpy(loop_type, "do-while");
            return 1;
        }
    }
    return 0;
}

static int detect_condition(const char* line, char* cond_type) {
    char temp[1024];
    strncpy(temp, line, 1023);
    temp[1023] = '\0';
    trim(temp);
    if (strstr(temp, "if") && strchr(temp, '(')) {
        const char* pos = strstr(temp, "if");
        if ((pos == temp || !isalnum((unsigned char)pos[-1])) && !isalnum((unsigned char)pos[2])) {
            strcpy(cond_type, "if");
            return 1;
        }
    }
    if (strstr(temp, "else") && strstr(temp, "if")) {
        strcpy(cond_type, "else-if");
        return 1;
    }
    if (strstr(temp, "else")) {
        const char* pos = strstr(temp, "else");
        if ((pos == temp || !isalnum((unsigned char)pos[-1])) && !isalnum((unsigned char)pos[4])) {
            strcpy(cond_type, "else");
            return 1;
        }
    }
    if (strstr(temp, "switch") && strchr(temp, '(')) {
        const char* pos = strstr(temp, "switch");
        if ((pos == temp || !isalnum((unsigned char)pos[-1])) && !isalnum((unsigned char)pos[6])) {
            strcpy(cond_type, "switch");
            return 1;
        }
    }
    if (strstr(temp, "case")) {
        const char* pos = strstr(temp, "case");
        if ((pos == temp || !isalnum((unsigned char)pos[-1])) && !isalnum((unsigned char)pos[4])) {
            strcpy(cond_type, "case");
            return 1;
        }
    }
    return 0;
}

void parse_file_and_populate(const char* filename, HashTable* ht, TrieNode* trie, AVLNode** avl, FunctionGraph* graph)
{
    // --- Phase 1: Token-based Function Detection ---
    int token_count = 0;
    Token* tokens = tokenize_file(filename, &token_count);
    if (tokens) {
        int brace_depth = 0;
        char current_scope[MAX_NAME] = "global";

        for (int i = 0; i < token_count; i++) {
            if (tokens[i].type == TOKEN_PUNCTUATION) {
                if (strcmp(tokens[i].value, "{") == 0) brace_depth++;
                else if (strcmp(tokens[i].value, "}") == 0) {
                    brace_depth--;
                    if (brace_depth <= 0) {
                        brace_depth = 0;
                        strcpy(current_scope, "global");
                    }
                }
            }

            if (tokens[i].type == TOKEN_PUNCTUATION && strcmp(tokens[i].value, "(") == 0 && i >= 1) {
                if (tokens[i-1].type == TOKEN_IDENTIFIER) {
                    int type_end = i - 2;
                    int has_type = 0;
                    char type_buf[MAX_TYPE] = "";
                    
                    int type_start = type_end;
                    while (type_start >= 0) {
                        if (tokens[type_start].type == TOKEN_KEYWORD || 
                            tokens[type_start].type == TOKEN_IDENTIFIER || 
                            (tokens[type_start].type == TOKEN_OPERATOR && strcmp(tokens[type_start].value, "*") == 0)) {
                            has_type = 1;
                            type_start--;
                        } else {
                            break;
                        }
                    }
                    type_start++;
                    
                    if (has_type && type_start <= type_end) {
                        if (strcmp(tokens[i-1].value, "if") == 0 || strcmp(tokens[i-1].value, "for") == 0 ||
                            strcmp(tokens[i-1].value, "while") == 0 || strcmp(tokens[i-1].value, "switch") == 0) {
                            // not a function
                        } else {
                            for (int t = type_start; t <= type_end; t++) {
                                if (t > type_start && strcmp(tokens[t].value, "*") != 0 && strcmp(tokens[t-1].value, "*") != 0) {
                                    strcat(type_buf, " ");
                                }
                                strncat(type_buf, tokens[t].value, MAX_TYPE - strlen(type_buf) - 1);
                            }
                            
                            int j = i + 1;
                            int paren_depth = 1;
                            while (j < token_count && paren_depth > 0) {
                                if (strcmp(tokens[j].value, "(") == 0) paren_depth++;
                                else if (strcmp(tokens[j].value, ")") == 0) paren_depth--;
                                j++;
                            }
                            
                            if (j < token_count && tokens[j].type == TOKEN_PUNCTUATION && strcmp(tokens[j].value, "{") == 0) {
                                char name[MAX_NAME];
                                strncpy(name, tokens[i-1].value, MAX_NAME-1);
                                name[MAX_NAME-1] = '\0';
                                
                                int lineno = tokens[i-1].line;
                                
                                SymbolInfo s;
                                memset(&s, 0, sizeof(s));
                                strncpy(s.name, name, MAX_NAME-1);
                                strncpy(s.type, type_buf, MAX_TYPE-1);
                                strncpy(s.category, "function", MAX_CATEGORY-1);
                                s.line = lineno;
                                
                                ht_insert(ht, &s);
                                trie_insert_with_metadata(trie, s.name, s.type, "global", s.line);
                                *avl = avl_insert(*avl, s.name, s.type, "global", s.line);
                                
                                if (graph) {
                                    graph_add_function(graph, name, type_buf, lineno);
                                }

                                if (brace_depth == 0) {
                                    strncpy(current_scope, name, MAX_NAME-1);
                                    current_scope[MAX_NAME-1] = '\0';
                                }
                            }
                        }
                    }
                }
            }



            if (is_type_token(tokens[i].value)) {
                if (i > 0 && is_type_token(tokens[i-1].value)) continue;

                int t = i;
                char base_type[MAX_TYPE] = "";
                while (t < token_count && is_type_token(tokens[t].value)) {
                    if (strlen(base_type) > 0) strcat(base_type, " ");
                    strcat(base_type, tokens[t].value);
                    t++;
                }
                

                while (t < token_count) {
                    char full_type[MAX_TYPE];
                    strcpy(full_type, base_type);
                    
                    while (t < token_count && strcmp(tokens[t].value, "*") == 0) {
                        strcat(full_type, "*"); 
                        t++;
                    }
                    
                    if (t >= token_count || tokens[t].type != TOKEN_IDENTIFIER) break;
                    
                    char var_name[MAX_NAME];
                    strcpy(var_name, tokens[t].value);
                    int var_line = tokens[t].line;
                    t++;
                    
                    if (t < token_count && strcmp(tokens[t].value, "[") == 0) {
                        strcat(full_type, "[]");
                        while (t < token_count && strcmp(tokens[t].value, "]") != 0) t++;
                        if (t < token_count) t++;
                    }
                    
                    if (t < token_count && strcmp(tokens[t].value, "(") == 0) {
                        break;
                    }
                    
                    SymbolInfo s;
                    memset(&s, 0, sizeof(s));
                    strncpy(s.name, var_name, MAX_NAME-1);
                    strncpy(s.type, full_type, MAX_TYPE-1);
                    strncpy(s.category, "variable", MAX_CATEGORY-1);
                    s.line = var_line;
                    
                    ht_insert(ht, &s);
                    trie_insert_with_metadata(trie, s.name, s.type, current_scope, s.line);
                    *avl = avl_insert(*avl, s.name, s.type, current_scope, s.line);
                    
                    if (t < token_count && strcmp(tokens[t].value, "=") == 0) {
                        int paren_depth = 0;
                        while (t < token_count) {
                            if (strcmp(tokens[t].value, "(") == 0) paren_depth++;
                            else if (strcmp(tokens[t].value, ")") == 0) paren_depth--;
                            
                            if (paren_depth == 0 && (strcmp(tokens[t].value, ",") == 0 || strcmp(tokens[t].value, ";") == 0)) {
                                break;
                            }
                            t++;
                        }
                    }
                    
                    if (t < token_count && strcmp(tokens[t].value, ",") == 0) {
                        t++;
                        continue;
                    } else if (t < token_count && strcmp(tokens[t].value, ";") == 0) {
                        i = t; 
                        break;
                    } else {
                        break;
                    }
                }
            }
        } // THIS CLOSES PHASE 1A
        
        // Phase 1B: Re-parse to isolate Call Trees successfully mapped against the constructed definitions recursively
        brace_depth = 0;
        strcpy(current_scope, "global");

        for (int i = 0; i < token_count; i++) {
            if (tokens[i].type == TOKEN_PUNCTUATION) {
                if (strcmp(tokens[i].value, "{") == 0) brace_depth++;
                else if (strcmp(tokens[i].value, "}") == 0) {
                    brace_depth--;
                    if (brace_depth <= 0) {
                        brace_depth = 0;
                        strcpy(current_scope, "global");
                    }
                }
            }

            if (tokens[i].type == TOKEN_PUNCTUATION && strcmp(tokens[i].value, "(") == 0 && i >= 1) {
                if (tokens[i-1].type == TOKEN_IDENTIFIER) {
                    int type_end = i - 2;
                    int has_type = 0;
                    char type_buf[MAX_TYPE] = "";
                    int type_start = type_end;
                    while (type_start >= 0) {
                        if (tokens[type_start].type == TOKEN_KEYWORD || 
                            tokens[type_start].type == TOKEN_IDENTIFIER || 
                            (tokens[type_start].type == TOKEN_OPERATOR && strcmp(tokens[type_start].value, "*") == 0)) {
                            has_type = 1;
                            type_start--;
                        } else {
                            break;
                        }
                    }
                    type_start++;
                    if (has_type && type_start <= type_end) {
                        int j = i + 1;
                        int paren_depth = 1;
                        while (j < token_count && paren_depth > 0) {
                            if (strcmp(tokens[j].value, "(") == 0) paren_depth++;
                            else if (strcmp(tokens[j].value, ")") == 0) paren_depth--;
                            j++;
                        }
                        if (j < token_count && tokens[j].type == TOKEN_PUNCTUATION && strcmp(tokens[j].value, "{") == 0) {
                            char name[MAX_NAME];
                            strncpy(name, tokens[i-1].value, MAX_NAME-1);
                            name[MAX_NAME-1] = '\0';
                            if (brace_depth == 0) {
                                strncpy(current_scope, name, MAX_NAME-1);
                                current_scope[MAX_NAME-1] = '\0';
                            }
                        }
                    }
                }
            }
            
            if (tokens[i].type == TOKEN_IDENTIFIER && i + 1 < token_count && strcmp(tokens[i+1].value, "(") == 0) {
                if (strcmp(tokens[i].value, "sizeof") == 0 || strcmp(tokens[i].value, "if") == 0 ||
                    strcmp(tokens[i].value, "for") == 0 || strcmp(tokens[i].value, "while") == 0 ||
                    strcmp(tokens[i].value, "switch") == 0) {
                    // Ignore flow definitions parsing safely natively.
                } else {
                    int is_decl = 0;
                    int j = i - 1;
                    while (j >= 0 && strcmp(tokens[j].value, "*") == 0) j--;
                    if (j >= 0 && is_type_token(tokens[j].value)) {
                        is_decl = 1; 
                    }
                    
                    if (!is_decl) {
                        if (graph) {
                            graph_add_call(graph, current_scope, tokens[i].value, tokens[i].line);
                        }
                    }
                }
            }
        }
        free(tokens);
    }

    // --- Phase 2: Line-based Parsing for Other Symbols ---
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open %s\n", filename);
        return;
    }
    char line[1024];
    int lineno = 0;
    int in_multiline_comment = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        if (in_multiline_comment) {
            char* end = strstr(line, "*/");
            if (end) {
                in_multiline_comment = 0;
                memmove(line, end + 2, strlen(end + 2) + 1);
            } else continue;
        }
        char* start = strstr(line, "/*");
        if (start) {
            char* end = strstr(start, "*/");
            if (!end) {
                in_multiline_comment = 1;
                *start = '\0';
            }
        }
        strip_comments(line);
        char tmp[1024];
        strncpy(tmp, line, sizeof(tmp)-1);
        tmp[sizeof(tmp)-1] = '\0';
        trim(tmp);
        if (strlen(tmp) == 0) continue;
        // Obsolete Phase 2 string-based call iteration mapping explicitly purged from loop sequence.
        char loop_type[32];
        if (detect_loop(tmp, loop_type)) {
            SymbolInfo s;
            memset(&s, 0, sizeof(s));
            snprintf(s.name, MAX_NAME, "LOOP_%s", loop_type);
            strncpy(s.type, loop_type, MAX_TYPE-1);
            strncpy(s.category, "loop", MAX_CATEGORY-1);
            s.line = lineno;
            ht_insert(ht, &s);
            trie_insert_with_metadata(trie, s.name, s.type, s.category, s.line);
            *avl = avl_insert(*avl, s.name, s.type, s.category, s.line);
            continue;
        }
        char cond_type[32];
        if (detect_condition(tmp, cond_type)) {
            SymbolInfo s;
            memset(&s, 0, sizeof(s));
            snprintf(s.name, MAX_NAME, "COND_%s", cond_type);
            strncpy(s.type, cond_type, MAX_TYPE-1);
            strncpy(s.category, "condition", MAX_CATEGORY-1);
            s.line = lineno;
            ht_insert(ht, &s);
            trie_insert_with_metadata(trie, s.name, s.type, s.category, s.line);
            *avl = avl_insert(*avl, s.name, s.type, s.category, s.line);
            continue;
        }
        // Variable extraction string-matcher routine successfully migrated to tokens and deleted.
    }
    fclose(f);
}

ASTNode* parse_file_to_ast(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open %s\n", filename);
        return NULL;
    }
    ASTNode* root = createNode(NODE_PROGRAM, "Program");
    ASTNode* current_stmt = NULL;
    char line[1024];
    int lineno = 0;
    int in_multiline_comment = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        if (in_multiline_comment) {
            char* end = strstr(line, "*/");
            if (end) {
                in_multiline_comment = 0;
                memmove(line, end + 2, strlen(end + 2) + 1);
            } else continue;
        }
        char* start = strstr(line, "/*");
        if (start) {
            char* end = strstr(start, "*/");
            if (!end) {
                in_multiline_comment = 1;
                *start = '\0';
            }
        }
        strip_comments(line);
        char tmp[1024];
        strncpy(tmp, line, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
        trim(tmp);
        if (strlen(tmp) == 0) continue;
        if (looks_like_function_header(tmp)) {
            char name[MAX_NAME], type[MAX_TYPE];
            extract_name_from_function(tmp, name, type);
            if (strlen(name) > 0) {
                ASTNode* func_node = createNodeWithLine(NODE_FUNC_DECL, name, lineno);
                strncpy(func_node->returnType, type, sizeof(func_node->returnType) - 1);
                appendStatement(&current_stmt, func_node);
                addChild(root, func_node);
            }
            continue;
        }
        char loop_type[32];
        if (detect_loop(tmp, loop_type)) {
            ASTNode* loop_node = createNodeWithLine(NODE_FOR, loop_type, lineno);
            appendStatement(&current_stmt, loop_node);
            addChild(root, loop_node);
            continue;
        }
        char cond_type[32];
        if (detect_condition(tmp, cond_type)) {
            ASTNode* cond_node = createNodeWithLine(NODE_IF, cond_type, lineno);
            appendStatement(&current_stmt, cond_node);
            addChild(root, cond_node);
            continue;
        }
        char varname[MAX_NAME], vartype[MAX_TYPE];
        extract_variable_from_line(tmp, varname, vartype);
        if (strlen(varname) > 0) {
            ASTNode* var_node = createNodeWithLine(NODE_VAR_DECL, varname, lineno);
            strncpy(var_node->dataType, vartype, sizeof(var_node->dataType) - 1);
            appendStatement(&current_stmt, var_node);
            addChild(root, var_node);
        }
    }
    fclose(f);
    setParentPointers(root, NULL);
    return root;
}

void analyze_ast(ASTNode* root) {
    if (!root) return;
    printf("\n┌────────────────────────────────────────────────────────────────┐\n");
    printf("│                    AST ANALYSIS REPORT                        │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    int func_count = 0;
    int var_count = 0;
    int loop_count = 0;
    int cond_count = 0;
    ASTNode* curr = root->children[0];
    while (curr) {
        if (curr->type == NODE_FUNC_DECL) func_count++;
        else if (curr->type == NODE_VAR_DECL) var_count++;
        else if (curr->type == NODE_FOR || curr->type == NODE_WHILE) loop_count++;
        else if (curr->type == NODE_IF) cond_count++;
        curr = curr->next;
    }
    printf("Total Nodes: %d\n", countNodes(root));
    printf("Functions: %d\n", func_count);
    printf("Variables: %d\n", var_count);
    printf("Loops: %d\n", loop_count);
    printf("Conditions: %d\n\n", cond_count);
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│                   FUNCTION ANALYSIS                           │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n\n");
    curr = root->children[0];
    while (curr) {
        if (curr->type == NODE_FUNC_DECL) {
            printf("Function: %s (Return type: %s, Line: %d)\n",
                   curr->value, curr->returnType, curr->lineNumber);
            int complexity = calculateComplexity(curr);
            int nesting = calculateNestingDepth(curr, 0);
            printf("  ├─ Cyclomatic Complexity: %d\n", complexity);
            printf("  └─ Max Nesting Depth: %d\n\n", nesting);
        }
        curr = curr->next;
    }
}