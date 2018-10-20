#include<stdio.h>
#include <string.h>

int main()
{
	char url[10];
	scanf("%s",url);
	char *str;
	str = strrchr(url,'.');

	printf("position %s ",str);

}
