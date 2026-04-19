#include <stdio.h>
#include <string.h>

#define MAX 100

// structure example
struct Student {
    int id;
    char name[50];
    float marks;
};

// global variable
int globalCounter = 0;

// function prototypes
int factorial(int n);
int fibonacci(int n);
void printStudent(struct Student s);
int sumArray(int arr[], int size);
int maxArray(int arr[], int size);
void unusedFunction(); // dead code
int complexFunction(int x, int y);
int helperFunction(int a);

int main() {
    int numbers[MAX];
    int i, n = 5;

    struct Student s1;
    s1.id = 101;
    strcpy(s1.name, "Rahul");
    s1.marks = 87.5;

    printStudent(s1);

    // fill array
    for(i = 0; i < 10; i++) {
        numbers[i] = i * 2;
    }

    int sum = sumArray(numbers, 10);
    int max = maxArray(numbers, 10);

    printf("Sum: %d\n", sum);
    printf("Max: %d\n", max);

    int fact = factorial(n);
    int fib = fibonacci(n);

    printf("Factorial: %d\n", fact);
    printf("Fibonacci: %d\n", fib);

    int result = complexFunction(5, 3);

    printf("Complex Result: %d\n", result);

    return 0;
}

// recursion example
int factorial(int n) {
    if(n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

// recursion example
int fibonacci(int n) {
    if(n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// print structure
void printStudent(struct Student s) {
    printf("ID: %d\n", s.id);
    printf("Name: %s\n", s.name);
    printf("Marks: %.2f\n", s.marks);
}

// array sum
int sumArray(int arr[], int size) {
    int sum = 0;
    int i;

    for(i = 0; i < size; i++) {
        sum += arr[i];
    }

    return sum;
}

// find max element
int maxArray(int arr[], int size) {
    int max = arr[0];
    int i;

    for(i = 1; i < size; i++) {
        if(arr[i] > max) {
            max = arr[i];
        }
    }

    return max;
}

// complex logic for AST depth
int complexFunction(int x, int y) {
    int result = 0;

    if(x > 0) {
        for(int i = 0; i < x; i++) {

            if(i % 2 == 0) {
                result += helperFunction(i);
            }
            else {

                if(y > 2) {

                    while(y > 0) {
                        result += y;
                        y--;
                    }

                } else {

                    result -= y;

                }

            }

        }
    }
    else {
        result = -1;
    }

    return result;
}

// helper function
int helperFunction(int a) {

    int temp = 0;

    for(int i = 0; i < a; i++) {

        temp += i;

    }

    return temp;
}

// dead code function
void unusedFunction() {

    printf("This function is never called\n");

}