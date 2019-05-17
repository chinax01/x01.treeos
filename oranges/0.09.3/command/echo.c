#include "stdio.h"

int main(int argc, char * argv[])
{
	// 为什么非要赋值到 count 和 p，而不能直接使用参数？
	int count = argc;
	char** p = argv;

	int i;
	for (i = 1; i < count; i++)
		printf("%s%s", i == 1 ? "" : " ", p[i]);
	printf("\n");

	return 0;
}
