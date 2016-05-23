#ifndef _REPLAY_H
#define _REPLAY_H

#define BUFSIZE	300
#define RAMSIZE 60	//size of ramdisk(MB)

#define MEM_ALIGN				512  // Memory alignment
#define USE_GLOBAL_BUFF			1 
#define AIO_THREAD_POOL_SIZE	50

#define BYTE_PER_BLOCK			512 
#define LARGEST_REQUEST_SIZE	10000  // Largest request size in blocks
#define BLOCK_PER_DRIVE			3800000	//2GB blocks number

struct req_info{
/*physical chk number*/
	unsigned int pcn;	
	
	long long time;		//us
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
#endif
