//Compile: clang -shared -fPIC -Wl,-soname,libmylibrary.so -ldl -o libmylibrary.so mylibrary.c -lc

#include "mylibrary.h"


void func1(void) {
    int a=2+3;
	a=a+1;
	return;
}

void func2(void) {
   int b=3+4;
	b=b+1;
	return;
}

void func3(void) {
    int c=4+5;
	c=c+1;
	return;
}

void func4(void) {
    int d=5+6;
	d=d+1;
	return;
}

