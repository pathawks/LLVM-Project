#include <stdio.h>

int list[] = {1,5,4,3,6,2};
int size = 6;

void printList() {
	for (int i=0; i<size; ++i)
		printf("\t%d\n", list[i]);
}

void swap(int* x, int* y) {
	int temp = *x;
	*x = *y;
	*y = temp;
}

void selectionSort() {
	for (int i=0; i<size-1; ++i) {
		int min = i;
		for (int j=i+1; j<size; ++j)
			if (list[min] > list[j])
				min = j;
		if (min != i)
			swap(list+i, list+min);
	}
}

int main(void) {
	puts("Unsorted:");
	printList();
	selectionSort();
	puts("\nSorted:");
	printList();
	return 0;
}
