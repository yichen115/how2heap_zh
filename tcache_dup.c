#include <stdio.h>
#include <stdlib.h>

int main()
{
	fprintf(stderr, "先申请一块内存\n");
	int *a = malloc(8);

	fprintf(stderr, "申请的内存地址是: %p\n", a);
	fprintf(stderr, "对这块内存地址 free两次\n");
	free(a);
	free(a);

	fprintf(stderr, "这时候链表是这样的 [ %p, %p ].\n", a, a);
	fprintf(stderr, "接下来再去 malloc 两次: [ %p, %p ].\n", malloc(8), malloc(8));
    fprintf(stderr, "ojbk\n");
	return 0;
}