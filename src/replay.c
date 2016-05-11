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
	
	queue_print(trace);
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
		if((req->pcn > 0)&&(req->pcn < pool->chunk_scm))
		{
			//submit_aio(fd[0],buf,req,trace);
			/*RAMDisk*/
            //printf("++++into SCM\n");
			req->lba=req->lba%(60*2048);
            submit_aio(fd[0],buf,req,trace);
		}
		else if(req->pcn < pool->chunk_scm+pool->chunk_ssd)
		{
			submit_aio(fd[1],buf,req,trace);
		}
		else if(req->pcn < pool->chunk_sum)
		{
			submit_aio(fd[2],buf,req,trace);
            //printf("++++into HDD\n");
		}
		else
		{
			printf("Error in mapping table\n");
			exit(-1);
		}
		/************************************/
		/************************************/
	}
	while(trace->inNum > trace->outNum)
	{
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
	fprintf(cb->trace->logFile,"%-16lld %-12lld %-5d %-2d %d \n",
				cb->req->time,cb->req->lba,cb->req->size,cb->req->type,latency);
	fflush(cb->trace->logFile);

	cb->trace->outNum++;
	//printf("cb->trace->outNum=%d\n",cb->trace->outNum);
	if(cb->trace->outNum%50==0)
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

long long time_now()
{
	struct timeval now;
	gettimeofday(&now,NULL);
	return 1000000*now.tv_sec+now.tv_usec;	//us
}

long long time_elapsed(long long begin)
{
	return time_now()-begin;	//us
}

void queue_push(struct trace_info *trace,struct req_info *req)
{
	struct req_info* temp;
	temp = (struct req_info *)malloc(sizeof(struct req_info));
	temp->pcn = req->pcn;	//chk number
	temp->time = req->time;	//us
	temp->dev = req->dev;
	temp->lba = req->lba;	//bytes
	temp->size = req->size;	//bytes
	temp->type = req->type;	//0<->Read
	temp->next = NULL;
	if(trace->front == NULL && trace->rear == NULL)
	{
		trace->front = trace->rear = temp;
	}
	else
	{
		trace->rear->next = temp;
		trace->rear = temp;
	}
}

void queue_pop(struct trace_info *trace,struct req_info *req) 
{
	struct req_info* temp = trace->front;
	if(trace->front == NULL) 
	{
		printf("Queue is Empty\n");
		return;
	}
	req->pcn  = trace->front->pcn;
	req->time = trace->front->time;
	req->dev  = trace->front->dev;
	req->lba  = trace->front->lba;
	req->size = trace->front->size;
	req->type = trace->front->type;	
	if(trace->front == trace->rear) 
	{
		trace->front = trace->rear = NULL;
	}
	else 
	{
		trace->front = trace->front->next;
	}
	free(temp);
}

void queue_print(struct trace_info *trace)
{
	struct req_info* temp = trace->front;
	FILE *logfile=fopen("queueLog.txt","w");
	while(temp->next) 
	{
		fprintf(logfile,"%-15lld %-5d %-15lld %-4d %d\n",temp->time,temp->pcn,temp->lba/512,temp->size/512,temp->type);
        fflush(logfile);
		temp = temp->next;
	}
    fclose(logfile);
}
