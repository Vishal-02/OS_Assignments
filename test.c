#include <stdio.h>
#include <math.h>

int main()
{
	unsigned int one = 15 & 7;
	unsigned int two = (one > 0) ? 1 : 0;
	printf("%u, %u\n", one, two);
	return 0;
}
