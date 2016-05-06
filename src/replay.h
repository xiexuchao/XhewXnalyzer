#ifndef _REPLAY_H
#define _REPLAY_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define SUCCESS 1
#define FAILURE 0
#define BUFSIZE	300

#define MEM_ALIGN		512  // Memory alignment
#define USE_GLOBAL_BUFF		1 
#define AIO_THREAD_POOL_SIZE	50

#define BYTE_PER_BLOCK		512 
#define LARGEST_REQUEST_SIZE	10000  // Largest request size in blocks
#define BLOCK_PER_DRIVE		3800000	//2GB blocks number

struct req_info{
	unsigned int pcn;	//physical chk number

	double time;
	unsigned int dev;
	long long lba;
	unsigned int size;
	unsigned int type;
	struct req_info *next;
};

struct trace_info{
	unsigned int inNum;
	unsigned int outNum;
	long long latencySum;
	FILE *logFile;

	struct req_info *front;
	struct req_info *rear;
};

struct aiocb_info{
	struct aiocb* aiocb;
	///* The order of these fields is implementation-dependent */
	//int             aio_fildes;     /* File descriptor */
	//off_t           aio_offset;     /* File offset */
	//volatile void  *aio_buf;        /* Location of buffer */
	//size_t          aio_nbytes;     /* Length of transfer */
	//int             aio_reqprio;    /* Request priority */
	//struct sigevent aio_sigevent;   /* Notification method */
	//int             aio_lio_opcode; /* Operation to be performed;lio_listio() only */
	//
	struct req_info* req;
	long long beginTime;
	struct trace_info *trace;
};

//replay.c
void replay(struct pool_info *pool,struct trace_info *trace);
long long time_now();
long long time_elapsed(long long begin);
static void handle_aio(sigval_t sigval);
static void submit_aio(int fd, void *buf,struct req_info *req,struct trace_info *trace);
static void init_aio();

//queue.c
void queue_push(struct trace_info *trace,struct req_info *req);
void queue_pop(struct trace_info *trace,struct req_info *req);
void queue_print(struct trace_info *trace);

#endif