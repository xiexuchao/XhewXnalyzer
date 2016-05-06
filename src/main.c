#include "pool.h"

int main()
{
	char path_i[100]="F:\\MSR Trace\\";
	char path_o[100]="F:\\MSR Trace\\results\\";
	char path_source[100];
	char path_target[100];
	//pool_run(strcat(strcpy(path_source,path_i),"prxy_0.ascii"),"config.txt",strcat(strcpy(path_target,path_o),"prxy_0.txt"),"log.txt");
	pool_run(strcat(strcpy(path_source,path_i),"prxy_1.ascii"),"config.txt",strcat(strcpy(path_target,path_o),"prxy_1.txt"),"log.txt");
	pool_run(strcat(strcpy(path_source,path_i),"src1_0.ascii"),"config.txt",strcat(strcpy(path_target,path_o),"src1_0.txt"),"log.txt");
	pool_run(strcat(strcpy(path_source,path_i),"usr_1.ascii"),"config.txt",strcat(strcpy(path_target,path_o),"usr_1.txt"),"log.txt");
	pool_run(strcat(strcpy(path_source,path_i),"web_2.ascii"),"config.txt",strcat(strcpy(path_target,path_o),"web_2.txt"),"log.txt");
	
	//pool_run(strcat(strcpy(path_source,path_i),"hadoop10.ascii"),"config.txt",strcat(strcpy(path_target,path_o),"hadoop10.txt"),"log.txt");
	//pool_run(strcat(strcpy(path_source,path_i),"hadoop13.ascii"),"config.txt",strcat(strcpy(path_target,path_o),"hadoop13.txt"),"log.txt");

	/*pool_run(strcat(strcpy(path_source,path_i),"backup1.ascii"),"config.txt",strcat(strcpy(path_target,path_o),"backup1.txt"),"log.txt");
	pool_run(strcat(strcpy(path_source,path_i),"backup5.ascii"),"config.txt",strcat(strcpy(path_target,path_o),"backup5.txt"),"log.txt");
	pool_run(strcat(strcpy(path_source,path_i),"backup14.ascii"),"config.txt",strcat(strcpy(path_target,path_o),"backup14.txt"),"log.txt");
	pool_run(strcat(strcpy(path_source,path_i),"backup15.ascii"),"config.txt",strcat(strcpy(path_target,path_o),"backup15.txt"),"log.txt");*/
	return 1;
}
