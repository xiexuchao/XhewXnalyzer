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

#define PATTERN_NOACCESS					'_'	
#define PATTERN_INACTIVE					'I'

#define PATTERN_ACTIVE_SEQ_R				'S'
#define PATTERN_ACTIVE_SEQ_W				's'

#define PATTERN_ACTIVE_RDM_OVER_R			'R'
#define PATTERN_ACTIVE_RDM_OVER_W			'W'

#define PATTERN_ACTIVE_RDM_FULY_W			'r'
#define PATTERN_ACTIVE_RDM_FULY_R			'w'

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
#define PATTERN_NOACCESS					'_'	
#define PATTERN_INACTIVE					'I'
#define PATTERN_ACTIVE_SEQ_R				'S'
#define PATTERN_ACTIVE_SEQ_W				's'
#define PATTERN_ACTIVE_RDM_OVER_R			'R'
#define PATTERN_ACTIVE_RDM_OVER_W			'W'
#define PATTERN_ACTIVE_RDM_FULY_W			'r'
#define PATTERN_ACTIVE_RDM_FULY_R			'w'
	*/

	double i_noaccess;
	double i_inactive;
	double i_active_seq_r;
	double i_active_seq_w;
	double i_active_rdm_over_r;
	double i_active_rdm_over_w;
	double i_active_rdm_fuly_r;
	double i_active_rdm_fuly_w;

    unsigned int chunk_access[SIZE_ARRAY];

    char   buffer[SIZE_BUFFER];
    double pattern_noaccess[SIZE_ARRAY];
	double pattern_inactive[SIZE_ARRAY];
    double pattern_active_seq_r[SIZE_ARRAY];
	double pattern_active_seq_w[SIZE_ARRAY];
    double pattern_active_rdm_over_r[SIZE_ARRAY];
	double pattern_active_rdm_over_w[SIZE_ARRAY];
    double pattern_active_rdm_fuly_r[SIZE_ARRAY];
	double pattern_active_rdm_fuly_w[SIZE_ARRAY];

    char filename_trace[100];
    char filename_output[100];
    char filename_config[100];
    char filename_log[100];
    FILE *file_trace;
    FILE *file_output;
    FILE *file_config;
    FILE *file_log;

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
void pool_run(char *trace,char *config,char *output,char *log);
void pattern_recognize(struct pool_info *pool);
//detector.c
void seq_detect(struct pool_info *pool);
void stream_flush(struct pool_info *pool);
//replay.c
void replay(struct pool_info *pool,struct trace_info *trace);
long long time_now();
long long time_elapsed(long long begin);
void handle_aio(sigval_t sigval);
void submit_aio(int fd, void *buf,struct req_info *req,struct trace_info *trace);
void init_aio();
//queue.c
void queue_push(struct trace_info *trace,struct req_info *req);
void queue_pop(struct trace_info *trace,struct req_info *req);
void queue_print(struct trace_info *trace);

#endif
