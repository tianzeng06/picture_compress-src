#include "compress.h"

int main(int argc,char *argv[])
{
	printf("please input filename: ");
	char s[100];
	scanf("%s",s);

	printf("please compress---0 or Uncompress---1 :");
	int operate;
	scanf("%d",&operate);

	char *filename=s;
	if(operate==0)
		compress(filename);
	else if(operate==1)
		Uncompress(filename);
	else
		fprintf(stderr,"Unsuporrted operate,try 0 or 1\n");
	return 0;
}
