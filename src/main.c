#include "pool.h"

int main()
{
	char path_i[100]="../trace/";
	char path_o[100]="../result/";
	char path_source[100];
	char path_target[100];

	pool_run(strcat(strcpy(path_source,path_i),"11.ascii"),"../config.txt",strcat(strcpy(path_target,path_o),"11.txt"),"log.txt");
	
	printf("%s\n",path_i);
	printf("%s\n",path_o);

	return 1;
}
