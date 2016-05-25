#ifndef _POOL_H
#define _POOL_H

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <aio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <errno.h>
#include <pthread.h>
//#include <signal.h>
//#include <sys/types.h>
//#include <sys/stat.h>

#include "detector.h"
#include "migration.h"
#include "replay.h"

#define SUCCESS		0
#define FAILURE		1
#define READ		0
#define WRITE		1
#define BUSY		0
#define FREE		1

#define POOL_SCM			1
#define POOL_SSD			2
#define POOL_HDD			3

#define SIZE_BUFFER 200
#define SIZE_ARRAY	500

//IO Patterns
#define PATTERN_UNKNOWN						' '
//LEVEL 1
#define PATTERN_NOACCESS					'_'	
#define PATTERN_INACTIVE					'I'
#define PATTERN_ACTIVE						'A'
//LEVEL 2
#define PATTERN_ACTIVE_READ					'R'
#define PATTERN_ACTIVE_WRITE				'W'
//LEVEL 3
#define PATTERN_ACTIVE_R_SEQ				'S'
#define PATTERN_ACTIVE_R_RDM				'M'
#define PATTERN_ACTIVE_W_SEQ				's'
#define PATTERN_ACTIVE_W_RDM				'm'
//LEVEL 4
#define PATTERN_ACTIVE_W_RDM_OVER			'O'
#define PATTERN_ACTIVE_W_RDM_FULY			'F'


struct pool_info{
	/*For Storage Pool*/   
	unsigned int size_scm;
    unsigned int size_ssd;
    unsigned int size_hdd;
    unsigned int size_chk;

    unsigned int size_stream;	//sequential detection
    unsigned int size_stride;

    unsigned int chunk_sum;	    //total in storage pool
    unsigned int chunk_max;
    unsigned int chunk_min;
    unsigned int chunk_all;	    //chunks accessed in trace file
    unsigned int chunk_win;	    //chunks accessed in one window
    
	unsigned int chunk_scm;
    unsigned int chunk_ssd;
    unsigned int chunk_hdd;
	unsigned int free_chk_scm;
	unsigned int free_chk_ssd;
	unsigned int free_chk_hdd;

    unsigned int window_size;
    unsigned int window_sum;
    long long window_time_start;		// ns
    long long window_time_end;			// ns

    double threshold_rw;
    double threshold_cbr;
    double threshold_car;
    unsigned int threshold_sequential;
    unsigned int threshold_inactive;
    unsigned int threshold_intensive;

	/*For Trace File*/
    long long time_start;
    long long time_end;

    unsigned int req_sum_all;		// IO num
    unsigned int req_sum_read;
    unsigned int req_sum_write;
    long double  req_size_all;		// IO size
    long double  req_size_read;
    long double  req_size_write;

    unsigned int seq_sum_all;		// Seq. IO num
    unsigned int seq_sum_read;
    unsigned int seq_sum_write;
    long double	 seq_size_all;		// Seq. IO size
    long double  seq_size_read;
    long double  seq_size_write;

    unsigned int seq_stream_all;	// Seq. stream num
    unsigned int seq_stream_read;
    unsigned int seq_stream_write;

	unsigned int migrate_scm_scm;	//data migration
	unsigned int migrate_scm_ssd;
	unsigned int migrate_scm_hdd;
	unsigned int migrate_ssd_scm;
	unsigned int migrate_ssd_ssd;
	unsigned int migrate_ssd_hdd;
	unsigned int migrate_hdd_scm;
	unsigned int migrate_hdd_ssd;
	unsigned int migrate_hdd_hdd;

	unsigned int size_in_window;
	unsigned int req_in_window;
	long double  time_in_window;

	/*
//LEVEL 1
#define PATTERN_NOACCESS					'_'	
#define PATTERN_INACTIVE					'I'
#defien PATTERN_ACTIVE						'A'
//LEVEL 2
#define PATTERN_ACTIVE_READ					'R'
#define PATTERN_ACTIVE_WRITE				'W'
//LEVEL 3
#define PATTERN_ACTIVE_R_SEQ				'S'
#define PATTERN_ACTIVE_R_RDM				'M'
#define PATTERN_ACTIVE_W_SEQ				's'
#define PATTERN_ACTIVE_W_RDM				'm'
//LEVEL 4
#define PATTERN_ACTIVE_W_RDM_FULY			'F'
#define PATTERN_ACTIVE_W_RDM_OVER			'O'
	*/

	double i_noaccess;
	double i_inactive;
	double i_active;
	double i_active_r;
	double i_active_w;
	double i_active_r_s;
	double i_active_r_m;
	double i_active_w_s;
	double i_active_w_m;
	double i_active_w_m_o;
	double i_active_w_m_f;

    unsigned int chunk_access[SIZE_ARRAY];

    char   buffer[SIZE_BUFFER];
	
    double pattern_noaccess[SIZE_ARRAY];
	double pattern_inactive[SIZE_ARRAY];
	double pattern_active[SIZE_ARRAY];
    double pattern_active_read[SIZE_ARRAY];
	double pattern_active_write[SIZE_ARRAY];
    double pattern_active_r_seq[SIZE_ARRAY];
	double pattern_active_r_rdm[SIZE_ARRAY];
    double pattern_active_w_seq[SIZE_ARRAY];
	double pattern_active_w_rdm[SIZE_ARRAY];
	double pattern_active_w_rdm_over[SIZE_ARRAY];
	double pattern_active_w_rdm_fuly[SIZE_ARRAY];

    char filename_trace[100];
    char filename_output[100];
    char filename_config[100];
    //char filename_log[100];
    FILE *file_trace;
    FILE *file_output;
    FILE *file_config;

	//for replay engine
	char device[10][64];
	int  deviceNum;
	char logFileName[64];

	struct map_info		*mapTab;
	struct map_info		*revTab;
    struct chunk_info	*chunk;
    struct request_info *req;
    struct stream_info	*stream;
	struct record_info	*record_win; //how many chks were accessed in a window
    struct record_info	*record_all; //total chks accessed in trace file
};

struct chunk_info{
	unsigned int pcn;
    unsigned int pattern;	
    unsigned int pattern_last;	// related to current IO pattern
    unsigned int location;		//SCM,SSD OR HDD
    unsigned int location_next;

    /*information in a window*/
    unsigned int req_sum_all;		// IO num
    unsigned int req_sum_read;
    unsigned int req_sum_write;
    long double  req_size_all;		// IO size
    long double  req_size_read;
    long double  req_size_write;

    unsigned int seq_sum_all;		// Seq. IO num
    unsigned int seq_sum_read;
    unsigned int seq_sum_write;
    long double	 seq_size_all;		// Seq. IO size
    long double  seq_size_read;
    long double  seq_size_write;

    unsigned int seq_stream_all;	// Seq. stream num
    unsigned int seq_stream_read;
    unsigned int seq_stream_write;

	char  history_pattern[SIZE_ARRAY];
    short history_migrate[SIZE_ARRAY];
};

struct request_info{
	long long time;
    long long lba;
    unsigned int type;//0->Read,1->Write
    unsigned int size;
};

struct map_info{
    int lcn;	//logical chunk number
    int pcn;	//physical chunk number
    unsigned int location;
};

struct record_info{
    unsigned int accessed;//accessed or not in a window
};
//initialize.c
void load_parameters(struct pool_info *pool,char *config);
void initialize(struct pool_info *pool,char *trace,char *output,char *log);
//pool.c
int warmup(struct pool_info *pool);
int get_request(struct pool_info *pool);
void stat_update(struct pool_info *pool);
void stat_print(struct pool_info *pool);
void alloc_assert(void *p,char *s);
//map.c
int find_free(struct pool_info *pool,int type);
void update_map(struct pool_info *pool,int i);

//recognition.c
void pool_run_static(char *trace,char *config,char *output,char *log);
void pool_run_dynamic(char *trace,char *config,char *output,char *log);
void pool_run_iops(char *traceName,char *configName,char *outputName,char *logName);
void pool_run_iopsth(char *traceName,char *configName,char *outputName,char *logName);
void pool_run_fcfs(char *traceName,char *configName,char *outputName,char *logName);

void pattern_recognize_static(struct pool_info *pool);
void pattern_recognize_dynamic(struct pool_info *pool);
void pattern_recognize_iops(struct pool_info *pool);
void pattern_recognize_iopsth(struct pool_info *pool);

void bubble_sort(unsigned int a[],unsigned int b[],int n);
int  find_num(unsigned int a[],int n);

//detector.c
void seq_detect(struct pool_info *pool);
void stream_flush(struct pool_info *pool);
//replay.c
void replay(struct pool_info *pool,struct trace_info *trace);
inline long long time_now();
inline long long time_elapsed(long long begin);
void handle_aio(sigval_t sigval);
void submit_aio(int fd, void *buf,struct req_info *req,struct trace_info *trace);
void init_aio();
//queue.c
void queue_push(struct trace_info *trace,struct req_info *req);
void queue_pop(struct trace_info *trace,struct req_info *req);
void queue_print(struct trace_info *trace);

#endif
