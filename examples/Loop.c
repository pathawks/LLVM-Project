#include <stdio.h>

int main(void) {
	puts("First, print 1-10");
	for (int i=1; i<=10; ++i) {
		printf("%d\n", i);
	}

	puts("\nNow print 1-9");
	for (int i=1; i<10; ++i) {
		printf("%u\n", i);
	}

	puts("\nNow print even numbers between 2-10");
	for (int i=2; i<=10; i+=2) {
		printf("%d\n", i);
	}

	return 0;
}
