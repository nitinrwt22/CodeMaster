#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.14159
#define MAX(x, y) ((x) > (y) ? (x) : (y))


int counter = 0;
static int hidden = 42;

enum Status { SUCCESS = 1, FAILURE = 0 };


typedef struct {
    int id;
    char name[50];
    float score;
} Student;

typedef int (*operation)(int, int);

int add(int a, int b);
int multiply(int a, int b);
void greet(const char *msg);
void testScope();
void testArrayAndPointers();
double computeArea(double radius);
int factorial(int n);
void processStudent(Student *s);
void recursivePrint(int n);
void edgeCaseTests(void);

int main(void) {
    printf("=== SAMPLE PROGRAM START ===\n");

    greet("Welcome to sample.c");

    int a = 10, b = 20;
    operation op = add;
    printf("Add: %d\n", op(a, b));

    op = multiply;
    printf("Multiply: %d\n", op(a, b));

    double area = computeArea(5.0);
    printf("Area: %.2lf\n", area);

    printf("Factorial(5) = %d\n", factorial(5));

    Student s1 = {101, "Nitin Rawat", 92.5};
    processStudent(&s1);

    testArrayAndPointers();
    testScope();
    edgeCaseTests();

    recursivePrint(3);

    printf("=== SAMPLE PROGRAM END ===\n");
    return 0;
}

int add(int a, int b) { return a + b; }

int multiply(int a, int b) { return a * b; }

void greet(const char *msg) {
    printf("Message: %s\n", msg);
}

void testScope() {
    int counter = 99;
    {
        int counter = 7;
        printf("Inner counter: %d\n", counter);
    }
    printf("Outer counter: %d\n", counter);
}

void testArrayAndPointers() {
    int arr[5] = {1, 2, 3, 4, 5};
    int *ptr = arr;

    printf("Array elements via pointer: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", *(ptr + i));
    }
    printf("\n");
}

double computeArea(double radius) {
    return PI * radius * radius;
}

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

void processStudent(Student *s) {
    if (!s) {
        printf("Invalid student record.\n");
        return;
    }
    printf("Student ID: %d, Name: %s, Score: %.2f\n", s->id, s->name, s->score);
}

void recursivePrint(int n) {
    if (n == 0) return;
    printf("Recursive call level: %d\n", n);
    recursivePrint(n - 1);
}

void edgeCaseTests(void) {
    printf("\n--- Edge Case Tests ---\n");

    for (int i = 0; i < 3; i++) { }

    int unused = 999;

    int x = 5;
    if (x > 5)
        printf("Greater than 5\n");
    else if (x == 5)
        printf("Exactly 5\n");
    else
        printf("Less than 5\n");

   
    switch (x) {
        case 1:
            printf("Case 1\n");
            break;
        case 5:
            printf("Case 5\n");
            break;
        default:
            printf("Default case\n");
    }

  
    int data[] = {10, 20, 30};
    int *p = data;
    printf("Pointer test: %d %d %d\n", *p, *(p+1), *(p+2));

 
    char *str = NULL;
    if (str == NULL) {
        printf("Null pointer handled safely.\n");
    }

    int denom = 0;
    if (denom != 0)
        printf("Division result: %d\n", 10 / denom);
    else
        printf("Division by zero avoided.\n");

    printf("--- End Edge Case Tests ---\n");
}
