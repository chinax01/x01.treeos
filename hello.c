#include <stdio.h>

void test_static();

int main()
{
	test_static();
	test_static();
	test_static();
	return 0;
}

void test_static()
{
	static int i = 5;
	printf("%d => ", i);
	i+=5;
	printf("%d\n", i);
}