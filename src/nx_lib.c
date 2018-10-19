#include <nx_lib.h>

void *nx_memcpy(void *dest, const void *src, int n)
{
	const char *s = src;
	char *d = dest;

	while (n--)
		*d++ = *s++;

	return dest;
}

void *nx_memset(void *str, int c, int n)
{
	char *pdata = str;
	while (n--)
		*pdata++ = c;
	return str;
}

int nx_memcmp(const void* s1, const void* s2, int n)
{
	const char *src1 = s1, *src2 = s2;

	while (n--) {
		char res = *src1++ - *src2++;
		if (res)
			return (res);
	}
	return 0;
}

//1cycle = 5ns
void udelay(volatile unsigned int delay)
{
    volatile unsigned int i, temp;
    for (i = 0; i < 200*delay; i++) {
        temp ^= i;
    }
}
