#include <stdio.h>

// Too many parameters (>5) - Severity 2
void many_params(int a, int b, int c, int d, int e, int f, int g) {
    printf("%d %d %d %d %d %d %d\n", a, b, c, d, e, f, g);
}

// Deep nesting (>4) - Severity 4
void deep_nesting() {
    if (1) {
        if (1) {
            if (1) {
                if (1) {
                    if (1) {
                        printf("Nested\n");
                    }
                }
            }
        }
    }
}

// Unused variable - Severity 5
void unused_var_func() {
    int secret_unused;
    int partially_unused = 10;
    // partially_unused is assigned but never read
}

// Large function (>50 lines) - Severity 3
void large_func() {
    printf("Line 1\n");
    printf("Line 2\n");
    printf("Line 3\n");
    printf("Line 4\n");
    printf("Line 5\n");
    printf("Line 6\n");
    printf("Line 7\n");
    printf("Line 8\n");
    printf("Line 9\n");
    printf("Line 10\n");
    printf("Line 11\n");
    printf("Line 12\n");
    printf("Line 13\n");
    printf("Line 14\n");
    printf("Line 15\n");
    printf("Line 16\n");
    printf("Line 17\n");
    printf("Line 18\n");
    printf("Line 19\n");
    printf("Line 20\n");
    printf("Line 21\n");
    printf("Line 22\n");
    printf("Line 23\n");
    printf("Line 24\n");
    printf("Line 25\n");
    printf("Line 26\n");
    printf("Line 27\n");
    printf("Line 28\n");
    printf("Line 29\n");
    printf("Line 30\n");
    printf("Line 31\n");
    printf("Line 32\n");
    printf("Line 33\n");
    printf("Line 34\n");
    printf("Line 35\n");
    printf("Line 36\n");
    printf("Line 37\n");
    printf("Line 38\n");
    printf("Line 39\n");
    printf("Line 40\n");
    printf("Line 41\n");
    printf("Line 42\n");
    printf("Line 43\n");
    printf("Line 44\n");
    printf("Line 45\n");
    printf("Line 46\n");
    printf("Line 47\n");
    printf("Line 48\n");
    printf("Line 49\n");
    printf("Line 50\n");
    printf("Line 51\n");
}

int main() {
    many_params(1, 2, 3, 4, 5, 6, 7);
    deep_nesting();
    unused_var_func();
    large_func();
    return 0;
}
