#include<stdio.h>

int main()
{

	while(1)
	{
		FILE *fp;
		printf("frvr");
		char path[444];
		fgets(path,444,stdin);
		fp = NULL;
		fp = fopen(path,"rb");
		printf("fvf");
	}
}
