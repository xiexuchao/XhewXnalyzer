#include "queue.h"
void queue_push(struct trace_info *trace,struct req_info *req)
{
	struct req_info* temp;
	temp = (struct req_info *)malloc(sizeof(struct req_info));
	temp->init_lba = req->init_lba;
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
	req->init_lba  = trace->front->init_lba;
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
		fprintf(logfile,"%-15lld %-5d %-15lld %-15lld %-4d %d\n",temp->time,temp->pcn,temp->init_lba/512,temp->lba/512,temp->size/512,temp->type);
        fflush(logfile);
		temp = temp->next;
	}
    fclose(logfile);
}