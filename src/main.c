#include "pool.h"

int main()
{
	//char path_i[100]="../trace/";
	//char path_o[100]="../result/";
	//char path_source[100];
	//char path_target[100];

	//pool_run(strcat(strcpy(path_source,path_i),"11.ascii"),"../config.ini",strcat(strcpy(path_target,path_o),"11.txt"),"log.txt");
	/* pool_run_static("trace/prn_1.ascii","config/config_prn_1.ini","result/prn_1s.txt","log.txt");
	pool_run_static("trace/prxy_1.ascii","config/config_prxy_1.ini","result/prxy_1s.txt","log.txt");
	pool_run_static("trace/hadoop10.ascii","config/config_hadoop10.ini","result/hadoop10s.txt","log.txt");
	pool_run_static("trace/backup1.ascii","config/config_backup1.ini","result/backup1s.txt","log.txt");
	
	pool_run_dynamic("trace/prn_1.ascii","config/config_prn_1D.ini","result/prn_1d.txt","log.txt");
	pool_run_dynamic("trace/prxy_1.ascii","config/config_prxy_1D.ini","result/prxy_1d.txt","log.txt");
	pool_run_dynamic("trace/hadoop10.ascii","config/config_hadoop10D.ini","result/hadoop10d.txt","log.txt");
	pool_run_dynamic("trace/backup1.ascii","config/config_backup1D.ini","result/backup1d.txt","log.txt"); */
	
	pool_run_iops("trace/11.ascii","config.ini","result/11.txt","log.txt");
	
	//printf("%s\n",path_i);
	//printf("%s\n",path_o);

	return 1;
}
