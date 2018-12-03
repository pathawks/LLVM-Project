#include <stdio.h>

int list[] = {1,5,4,3,6,2};
int size = 6;

void printList() {
	for (int i=0; i<size; ++i) {
		printf("\t%d\n", list[i]);
	}
}

void swap(int* x, int* y) {
	int temp = *x;
	*x = *y;
	*y = temp;
}

void sortList() {
	for (int i=0; i<size-1; ++i)
		for (int j=i+1; j<size; ++j) {
			if (list[i] > list[j])
				swap(list+i, list+j);
		}
}

int main(void) {
	puts("Unsorted:");
	printList();
	sortList();
	puts("\nSorted:");
	printList();
	return 0;
}
