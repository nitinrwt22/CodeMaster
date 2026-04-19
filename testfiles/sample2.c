#include <stdio.h>

const unsigned int global_count = 100;
char global_arr[256];

void process(int x) {
    int a, b, c;
    char *ptr, **dptr;
    float numbers[10];
    
    a = 1;
    b = 2;
    c = 3;
    unsigned long sum = a + b + c;
}

int main() {
    struct MyStruct *sptr;
    int index = 0;
    process(index);
    return 0;
}
