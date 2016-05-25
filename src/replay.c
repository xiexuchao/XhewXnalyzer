#include "pool.h"

void replay(struct pool_info *pool,struct trace_info *trace)
{
	int fd[10];
	char *buf;
	int i;
	long long initTime,nowTime,reqTime;//,waitTime;

	struct req_info *req;
	req=(struct req_info *)malloc(sizeof(struct req_info));
	alloc_assert(req,"req");
	memset(req,0,sizeof(struct req_info));
	
	//queue_print(trace);
	printf("trace->inNum=%d\n",trace->inNum);
	printf("trace->outNum=%d\n",trace->outNum);
	printf("trace->latencySum=%lld\n",trace->latencySum);

	printf("pool->devNum=%d\n",pool->deviceNum);
	for(i=0;i<pool->deviceNum;i++)
	{
		printf("pool->device[%d]=%s\n",i,pool->device[i]);
	}
	
	for(i=0;i<pool->deviceNum;i++)
	{
		fd[i] = open(pool->device[i], O_DIRECT | O_SYNC | O_RDWR); 
		if(fd[i] < 0) 
		{
			fprintf(stderr, "Value of errno: %d\n", errno);
	       	printf("Cannot open\n");
       		exit(-1);
		}
	}

	if (posix_memalign((void**)&buf, MEM_ALIGN, LARGEST_REQUEST_SIZE * BYTE_PER_BLOCK))
	{
		fprintf(stderr, "Error allocating buffer\n");
		return;
	}
	for(i=0;i<LARGEST_REQUEST_SIZE*BYTE_PER_BLOCK;i++)
	{
		//Generate random alphabets to write to file
		buf[i]=(char)(rand()%26+65);
	}

	init_aio();

	initTime=time_now();
	//printf("initTime=%lld\n",initTime);
	
	fprintf(trace->logFile,"%-16s %-12s %-5s %-12s %-5s %-5s %s \n",
			"Time","LBA","PCN","LBA","Size","Type","Latency");
	fflush(trace->logFile);
	
	while(trace->front)
	{
		queue_pop(trace,req);
		reqTime=req->time;
		nowTime=time_elapsed(initTime);
		//waitTime=reqTime-nowTime;
		while(nowTime < reqTime)
		{
			//usleep(waitTime);
			nowTime=time_elapsed(initTime);
		}
		/************************************/
		/************************************
			replay based on mapping table
		************************************/
		if((req->pcn >= 0)&&(req->pcn < pool->chunk_scm))
		{
			/*RAMDisk*/
			req->lba=req->lba%(RAMSIZE*2048);
            submit_aio(fd[0],buf,req,trace);
		}
		else if(req->pcn < pool->chunk_scm+pool->chunk_ssd)
		{
			req->lba=req->lba-20*1024*1024*2;
			submit_aio(fd[1],buf,req,trace);
		}
		else if(req->pcn < pool->chunk_sum)
		{
			req->lba=(req->lba-220*1024*1024*2)%(500*1024*1024*2);
			submit_aio(fd[2],buf,req,trace);
		}
		else
		{
			printf("Error in mapping table\n");
			exit(-1);
		}
		/************************************/
		/************************************/
	}
    i=0;
	while(trace->inNum > trace->outNum)
	{
        i++;
        if(i>30)
        {
            break;
        }
		printf("trace->inNum=%d\n",trace->inNum);
		printf("trace->outNum=%d\n",trace->outNum);
		printf("begin sleepping 1 second------\n");
		sleep(1);
	}
	printf("average latency= %Lf\n",(long double)trace->latencySum/(long double)trace->inNum);
	free(buf);
	free(req);
}

void handle_aio(sigval_t sigval)
{
	struct aiocb_info *cb;
	int latency;
	int error;
	int count;

	cb=(struct aiocb_info *)sigval.sival_ptr;
	latency=time_elapsed(cb->beginTime);
	cb->trace->latencySum+=latency;

	error=aio_error(cb->aiocb);
	if(error)
	{
		if(error != ECANCELED)
		{
			fprintf(stderr,"Error completing i/o:%d\n",error);
		}
		else
		{
			printf("---ECANCELED error\n");
		}
		return;
	}
	count=aio_return(cb->aiocb);
	if(count<(int)cb->aiocb->aio_nbytes)
	{
		fprintf(stderr, "Warning I/O completed:%db but requested:%ldb\n",
			count,cb->aiocb->aio_nbytes);
	}
	/**[Time, Origial_LBA, Real_LBA, Size, Type, Latency]**/
	fprintf(cb->trace->logFile,"%-16lld %-12lld %-5d %-12lld %-5d %-5d %d \n",
				cb->req->time,cb->req->init_lba,cb->req->pcn,cb->req->lba/512,cb->req->size/512,cb->req->type,latency);
	fflush(cb->trace->logFile);

	cb->trace->outNum++;
	//printf("cb->trace->outNum=%d\n",cb->trace->outNum);
	if(cb->trace->outNum%10000==0)
	{
		printf("---has replayed %d\n",cb->trace->outNum);
	}

	free(cb->aiocb);
	free(cb);
}

void submit_aio(int fd, void *buf, struct req_info *req,struct trace_info *trace)
{
	struct aiocb_info *cb;
	char *buf_new;
	int error=0;
	//struct sigaction *sig_act;

	cb=(struct aiocb_info *)malloc(sizeof(struct aiocb_info));
	memset(cb,0,sizeof(struct aiocb_info));//where to free this?
	cb->aiocb=(struct aiocb *)malloc(sizeof(struct aiocb));
	memset(cb->aiocb,0,sizeof(struct aiocb));//where to free this?
	cb->req=(struct req_info *)malloc(sizeof(struct req_info));
	memset(cb->req,0,sizeof(struct req_info));

	cb->aiocb->aio_fildes = fd;
	cb->aiocb->aio_nbytes = req->size;
	cb->aiocb->aio_offset = req->lba;

	cb->aiocb->aio_sigevent.sigev_notify = SIGEV_THREAD;
	cb->aiocb->aio_sigevent.sigev_notify_function = handle_aio;
	cb->aiocb->aio_sigevent.sigev_notify_attributes = NULL;
	cb->aiocb->aio_sigevent.sigev_value.sival_ptr = cb;

	//error=sigaction(SIGIO,sig_act,NULL);
	//write and read different buffer
	if(USE_GLOBAL_BUFF!=1)
	{
		if (posix_memalign((void**)&buf_new, MEM_ALIGN, req->size)) 
		{
			fprintf(stderr, "Error allocating buffer\n");
		}
		cb->aiocb->aio_buf = buf_new;
	}
	else
	{
		cb->aiocb->aio_buf = buf;
	}

	cb->req->init_lba=req->init_lba;
	cb->req->pcn = req->pcn;
	//cb->req=req;	//WTF
	cb->req->time=req->time;
	cb->req->dev=req->dev;
	cb->req->lba=req->lba;
	cb->req->size=req->size;
	cb->req->type=req->type;

	cb->beginTime=time_now();
	cb->trace=trace;

	if(req->type==1)
	{
		error=aio_write(cb->aiocb);
	}
	else //if(req->type==0)
	{
		error=aio_read(cb->aiocb);
	}
	//while(aio_error(cb->aiocb)==EINPROGRESS);
	if(error)
	{
		fprintf(stderr, "Error performing i/o");
		exit(-1);
	}
}

void init_aio()
{
	struct aioinit aioParam={0};
	//memset(aioParam,0,sizeof(struct aioinit));
	//two thread for each device is better
	aioParam.aio_threads = AIO_THREAD_POOL_SIZE;
	aioParam.aio_num = 2048;
	aioParam.aio_idle_time = 1;	
	aio_init(&aioParam);
}