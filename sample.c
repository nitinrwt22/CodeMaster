#include <stdio.h>

void sayHello();
void sayBye();
void greetAll();
void compute();

int main() {
    sayHello();
    greetAll();
    compute();
    sayBye();
    return 0;
}

void sayHello() {
    printf("Hello!\n");
}

void sayBye() {
    printf("Goodbye!\n");
}

void greetAll() {
    sayHello();
    sayBye();
}

void compute() {
    int a = 2, b = 3;
    int c = add(a, b);
    printf("Result: %d\n", c);
}

int add(int x, int y) {
    return x + y;
}
