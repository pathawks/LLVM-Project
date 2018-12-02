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
	return 0;
}
