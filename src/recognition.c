#include "pool.h"

void pool_run_static(char *traceName,char *configName,char *outputName,char *logName)
{
	unsigned int chk_id;
	struct pool_info *pool;

	/*for trace replayer*/
	struct trace_info *trace; 
	struct req_info *req;
	
	trace=(struct trace_info *)malloc(sizeof(struct trace_info));
	alloc_assert(trace,"trace");
	memset(trace,0,sizeof(struct trace_info));
	
	req=(struct req_info *)malloc(sizeof(struct req_info));
	alloc_assert(req,"req");
	memset(req,0,sizeof(struct req_info));

	pool=(struct pool_info *)malloc(sizeof(struct pool_info));
	alloc_assert(pool,"pool");
	memset(pool,0,sizeof(struct pool_info));

	load_parameters(pool,configName);
	initialize(pool,traceName,outputName,logName);
	warmup(pool);

	/***************************/
	//initialize trace parameters
	//trace here is a list of I/O requests pending to replay
	trace->inNum=0;
	trace->outNum=0;
	trace->latencySum=0;
	trace->logFile=fopen(pool->logFileName,"w");
	/***************************/

	while(get_request(pool)!=FAILURE)
	{
		chk_id=(int)(pool->req->lba/(pool->size_chk*2048));
		/********************************************
				Push into queue to replay
		********************************************/
		req->pcn=pool->mapTab[chk_id].pcn;
		if(req->pcn < 0)
		{
			printf("Error in lcn<->pcn mapping\n");
			exit(-1);
		}
		/****************************************************************************
				recalculate the lba of each I/O request based on mapping
		*****************************************************************************/
		req->lba=(req->pcn*pool->size_chk*2048)+(pool->req->lba%(pool->size_chk*2048));
		/**push current IO req to the replay queue**/
		trace->inNum++;	//track the process of IO requests
		req->time= pool->req->time;	//us
		req->lba = pool->req->lba * BYTE_PER_BLOCK;
		req->size= pool->req->size * BYTE_PER_BLOCK;
		req->type= pool->req->type;
		queue_push(trace,req);
		/********************************************/

		seq_detect(pool);	//Sequential IO Detection
		stat_update(pool);

		//update window info
		pool->size_in_window+=pool->req->size;
		pool->req_in_window++;
		if(pool->req_in_window==1)
		{
			pool->window_time_start=pool->req->time;	//us
		}	
		pool->window_time_end=pool->req->time;			//us

		if(((pool->window_time_end-pool->window_time_start)>=pool->window_size*60*1000*1000)
			||((feof(pool->file_trace)!=0)&&(pool->window_time_end>0)))
		{
			/*printf("----------------\n");
			printf("req wind=%d \n",pool->req_in_window);
			printf("req time=%lld \n",pool->req->time);
			printf("beg time=%lld \n",pool->window_time_start);
			printf("end time=%lld \n",pool->window_time_end);
			printf("win time=%lld \n",pool->window_time_end-pool->window_time_start);*/

			stream_flush(pool);
			pattern_recognize_static(pool);
		}
	}//while

	stat_print(pool);

	fclose(pool->file_trace);
	fclose(pool->file_output);
	fclose(pool->file_log);

	free(pool->mapTab);
	free(pool->revTab);
	free(pool->chunk);
	free(pool->req);
	free(pool->stream);
	free(pool->record_win);
	free(pool->record_all);

	/**start replay**/
	replay(pool,trace);
	free(req);
	free(trace);
	/****************/

	free(pool);
}

void pool_run_dynamic(char *traceName,char *configName,char *outputName,char *logName)
{
	unsigned int chk_id;
	struct pool_info *pool;

	/*for trace replayer*/
	struct trace_info *trace; 
	struct req_info *req;
	
	trace=(struct trace_info *)malloc(sizeof(struct trace_info));
	alloc_assert(trace,"trace");
	memset(trace,0,sizeof(struct trace_info));
	
	req=(struct req_info *)malloc(sizeof(struct req_info));
	alloc_assert(req,"req");
	memset(req,0,sizeof(struct req_info));

	pool=(struct pool_info *)malloc(sizeof(struct pool_info));
	alloc_assert(pool,"pool");
	memset(pool,0,sizeof(struct pool_info));

	load_parameters(pool,configName);
	initialize(pool,traceName,outputName,logName);
	warmup(pool);

	/***************************/
	//initialize trace parameters
	//trace here is a list of I/O requests pending to replay
	trace->inNum=0;
	trace->outNum=0;
	trace->latencySum=0;
	trace->logFile=fopen(pool->logFileName,"w");
	/***************************/

	while(get_request(pool)!=FAILURE)
	{
		chk_id=(int)(pool->req->lba/(pool->size_chk*2048));
		/********************************************
				Push into queue to replay
		********************************************/
		req->pcn=pool->mapTab[chk_id].pcn;
		if(req->pcn < 0)
		{
			printf("Error in lcn<->pcn mapping\n");
			exit(-1);
		}
		/****************************************************************************
				recalculate the lba of each I/O request based on mapping
		*****************************************************************************/
		req->lba=(req->pcn*pool->size_chk*2048)+(pool->req->lba%(pool->size_chk*2048));
		/**push current IO req to the replay queue**/
		trace->inNum++;	//track the process of IO requests
		req->time= pool->req->time;	//us
		req->lba = pool->req->lba * BYTE_PER_BLOCK;
		req->size= pool->req->size * BYTE_PER_BLOCK;
		req->type= pool->req->type;
		queue_push(trace,req);
		/********************************************/

		seq_detect(pool);	//Sequential IO Detection
		stat_update(pool);

		//update window info
		pool->size_in_window+=pool->req->size;
		pool->req_in_window++;
		if(pool->req_in_window==1)
		{
			pool->window_time_start=pool->req->time;	//us
		}	
		pool->window_time_end=pool->req->time;			//us

		if(((pool->window_time_end-pool->window_time_start)>=pool->window_size*60*1000*1000)
			||((feof(pool->file_trace)!=0)&&(pool->window_time_end>0)))
		{
			/*printf("----------------\n");
			printf("req wind=%d \n",pool->req_in_window);
			printf("req time=%lld \n",pool->req->time);
			printf("beg time=%lld \n",pool->window_time_start);
			printf("end time=%lld \n",pool->window_time_end);
			printf("win time=%lld \n",pool->window_time_end-pool->window_time_start);*/

			stream_flush(pool);
			pattern_recognize_dynamic(pool);
		}
	}//while

	stat_print(pool);

	fclose(pool->file_trace);
	fclose(pool->file_output);
	fclose(pool->file_log);

	free(pool->mapTab);
	free(pool->revTab);
	free(pool->chunk);
	free(pool->req);
	free(pool->stream);
	free(pool->record_win);
	free(pool->record_all);

	/**start replay**/
	replay(pool,trace);
	free(req);
	free(trace);
	/****************/

	free(pool);
}

void pool_run_iops(char *traceName,char *configName,char *outputName,char *logName)
{
	unsigned int chk_id;
	struct pool_info *pool;

	/*for trace replayer*/
	struct trace_info *trace; 
	struct req_info *req;
	
	trace=(struct trace_info *)malloc(sizeof(struct trace_info));
	alloc_assert(trace,"trace");
	memset(trace,0,sizeof(struct trace_info));
	
	req=(struct req_info *)malloc(sizeof(struct req_info));
	alloc_assert(req,"req");
	memset(req,0,sizeof(struct req_info));

	pool=(struct pool_info *)malloc(sizeof(struct pool_info));
	alloc_assert(pool,"pool");
	memset(pool,0,sizeof(struct pool_info));

	load_parameters(pool,configName);
	initialize(pool,traceName,outputName,logName);
	warmup(pool);

	/***************************/
	//initialize trace parameters
	//trace here is a list of I/O requests pending to replay
	trace->inNum=0;
	trace->outNum=0;
	trace->latencySum=0;
	trace->logFile=fopen(pool->logFileName,"w");
	/***************************/

	while(get_request(pool)!=FAILURE)
	{
		chk_id=(int)(pool->req->lba/(pool->size_chk*2048));
		/********************************************
				Push into queue to replay
		********************************************/
		req->pcn=pool->mapTab[chk_id].pcn;
		if(req->pcn < 0)
		{
			printf("Error in lcn<->pcn mapping\n");
			exit(-1);
		}
		/****************************************************************************
				recalculate the lba of each I/O request based on mapping
		*****************************************************************************/
		req->lba=(req->pcn*pool->size_chk*2048)+(pool->req->lba%(pool->size_chk*2048));
		/**push current IO req to the replay queue**/
		trace->inNum++;	//track the process of IO requests
		req->time= pool->req->time;	//us
		req->lba = pool->req->lba * BYTE_PER_BLOCK;
		req->size= pool->req->size * BYTE_PER_BLOCK;
		req->type= pool->req->type;
		queue_push(trace,req);
		/********************************************/

		seq_detect(pool);	//Sequential IO Detection
		stat_update(pool);

		//update window info
		pool->size_in_window+=pool->req->size;
		pool->req_in_window++;
		if(pool->req_in_window==1)
		{
			pool->window_time_start=pool->req->time;	//us
		}	
		pool->window_time_end=pool->req->time;			//us

		if(((pool->window_time_end-pool->window_time_start)>=pool->window_size*60*1000*1000)
			||((feof(pool->file_trace)!=0)&&(pool->window_time_end>0)))
		{
			/*printf("----------------\n");
			printf("req wind=%d \n",pool->req_in_window);
			printf("req time=%lld \n",pool->req->time);
			printf("beg time=%lld \n",pool->window_time_start);
			printf("end time=%lld \n",pool->window_time_end);
			printf("win time=%lld \n",pool->window_time_end-pool->window_time_start);*/

			stream_flush(pool);
			pattern_recognize_iops(pool);
		}
	}//while

	stat_print(pool);

	fclose(pool->file_trace);
	fclose(pool->file_output);
	fclose(pool->file_log);

	free(pool->mapTab);
	free(pool->revTab);
	free(pool->chunk);
	free(pool->req);
	free(pool->stream);
	free(pool->record_win);
	free(pool->record_all);

	/**start replay**/
	replay(pool,trace);
	free(req);
	free(trace);
	/****************/

	free(pool);
}


void pattern_recognize_static(struct pool_info *pool)
{
	unsigned int i;
	//seconds
	pool->time_in_window=(long double)(pool->window_time_end-pool->window_time_start)/(long double)1000000;
	
	/*Pattern Detection*/
	for(i=pool->chunk_min;i<=pool->chunk_max;i++)
	{
		pool->chunk[i].pattern_last=pool->chunk[i].pattern;
		pool->chunk[i].location=pool->chunk[i].location_next;

		if(pool->chunk[i].req_sum_all==0)
		{
			/*No Access*/
			if(pool->record_all[i].accessed!=0)
			{
				pool->i_noaccess++;
			}
			pool->chunk[i].pattern=PATTERN_NOACCESS;
			pool->chunk[i].location_next=POOL_HDD;
		}
		else if(pool->chunk[i].req_sum_all < pool->threshold_inactive)
		{
			/*Inactive*/
			pool->i_inactive++;
			pool->chunk[i].pattern=PATTERN_INACTIVE;
			pool->chunk[i].location_next=POOL_HDD;
		}
		else if(((long double)pool->chunk[i].req_sum_read/(long double)pool->chunk[i].req_sum_all)>=pool->threshold_rw)
		{
			/*Read*/
			if(((long double)pool->chunk[i].seq_size_all/(long double)pool->chunk[i].req_size_all)>=pool->threshold_cbr &&
			((long double)pool->chunk[i].seq_sum_all/(long double)pool->chunk[i].req_sum_all)>=pool->threshold_car)
			{
				/*Sequential*/
				pool->i_active_r_s++;
				pool->chunk[i].pattern=PATTERN_ACTIVE_R_SEQ;
				pool->chunk[i].location_next=POOL_HDD;				
			}
			else
			{
				/*Random*/
				pool->i_active_r_m++;
				pool->chunk[i].pattern=PATTERN_ACTIVE_R_RDM;
				pool->chunk[i].location_next=POOL_SSD;				
			}	
		}
		else
		{
			/*Write*/
			if(((long double)pool->chunk[i].seq_size_all/(long double)pool->chunk[i].req_size_all)>=pool->threshold_cbr &&
			((long double)pool->chunk[i].seq_sum_all/(long double)pool->chunk[i].req_sum_all)>=pool->threshold_car)
			{
				/*Sequential*/
				pool->i_active_w_s++;
				pool->chunk[i].pattern=PATTERN_ACTIVE_W_SEQ;
				pool->chunk[i].location_next=POOL_HDD;				
			}
			else
			{
				/*Random*/
				if(pool->chunk[i].req_sum_all>=(pool->req_in_window/pool->chunk_win)*pool->threshold_intensive)
				{
					/*Overwritten*/
					pool->i_active_w_m_o++;
					pool->chunk[i].pattern=PATTERN_ACTIVE_W_RDM_OVER;
					pool->chunk[i].location_next=POOL_SCM;
				}
				else
				{
					/*Fully Random Write*/
					pool->i_active_w_m_f++;
					pool->chunk[i].pattern=PATTERN_ACTIVE_W_RDM_FULY;
					pool->chunk[i].location_next=POOL_SCM;
				}			
			}			
		}
		pool->chunk[i].history_pattern[pool->window_sum]=pool->chunk[i].pattern;
		/************************************
			update mapping information
		*************************************/
		update_map(pool,i);	
		
		//print_log(pool,i);	//print info of each chunk in this window to log file.
		/*Initialize the statistics in each chunk*/
		pool->chunk[i].req_sum_all=0;
		pool->chunk[i].req_sum_read=0;
		pool->chunk[i].req_sum_write=0;
		pool->chunk[i].req_size_all=0;
		pool->chunk[i].req_size_read=0;
		pool->chunk[i].req_size_write=0;

		pool->chunk[i].seq_sum_all=0;
		pool->chunk[i].seq_sum_read=0;
		pool->chunk[i].seq_sum_write=0;
		pool->chunk[i].seq_size_all=0;
		pool->chunk[i].seq_size_read=0;
		pool->chunk[i].seq_size_write=0;

		pool->chunk[i].seq_stream_all=0;
		pool->chunk[i].seq_stream_read=0;
		pool->chunk[i].seq_stream_write=0;	
	}//for
	
	//Only record limited information (the first SIZE_ARRY windows)
	if(pool->window_sum<SIZE_ARRAY)
	{
		pool->chunk_access[pool->window_sum]=pool->chunk_win;		

		pool->pattern_noaccess[pool->window_sum]=pool->i_noaccess/(double)pool->chunk_all;
		pool->pattern_inactive[pool->window_sum]=pool->i_inactive/(double)pool->chunk_all;
		pool->pattern_active[pool->window_sum]=pool->i_active/(double)pool->chunk_all;
		pool->pattern_active_read[pool->window_sum]=pool->i_active_r/(double)pool->chunk_all;
		pool->pattern_active_write[pool->window_sum]=pool->i_active_w/(double)pool->chunk_all;
		pool->pattern_active_r_seq[pool->window_sum]=pool->i_active_r_s/(double)pool->chunk_all;
		pool->pattern_active_r_rdm[pool->window_sum]=pool->i_active_r_m/(double)pool->chunk_all;
		pool->pattern_active_w_seq[pool->window_sum]=pool->i_active_w_s/(double)pool->chunk_all;
		pool->pattern_active_w_rdm[pool->window_sum]=pool->i_active_w_m/(double)pool->chunk_all;
		pool->pattern_active_w_rdm_fuly[pool->window_sum]=pool->i_active_w_m_o/(double)pool->chunk_all;
		pool->pattern_active_w_rdm_over[pool->window_sum]=pool->i_active_w_m_f/(double)pool->chunk_all;
	}

	/*Update the pool info*/
	pool->window_sum++;
	if(pool->window_sum%50==0)
	{
		printf("------------pool->window_sum=%d---------------\n",pool->window_sum);
	}
	pool->window_time_start=0;
	pool->window_time_end=0;

	/*Start a new window*/
	pool->size_in_window=0;
	pool->req_in_window=0;
	pool->time_in_window=0;

	pool->i_noaccess=0;
	pool->i_inactive=0;
	pool->i_active=0;
	pool->i_active_r=0;
	pool->i_active_w=0;
	pool->i_active_r_s=0;
	pool->i_active_r_m=0;
	pool->i_active_w_s=0;
	pool->i_active_w_m=0;
	pool->i_active_w_m_f=0;
	pool->i_active_w_m_o=0;

	//accessed chunks in each window
	memset(pool->record_win,0,sizeof(struct record_info)*pool->chunk_sum);
	printf("pool->chunk_win=%d\n",pool->chunk_win);
	pool->chunk_win=0;
}

/**********************************************************************
*      Implement Hierachical Classifier  May 11th, 2016
*
*      advantage:    can better utilize faster storage pool capacity
*      disadvantage: might introduce more data migration operations 
*                      (Because inaccurate pattern detection)
* ********************************************************************/
void pattern_recognize_dynamic(struct pool_info *pool)
{
	unsigned int i;
	/*Pattern Detection*/
	//seconds
	pool->time_in_window=(long double)(pool->window_time_end-pool->window_time_start)/(long double)1000000;
	
	/*1st Level in Hierachical Classifier*/
	for(i=pool->chunk_min;i<=pool->chunk_max;i++)
	{
		pool->chunk[i].pattern_last=pool->chunk[i].pattern;
		pool->chunk[i].location=pool->chunk[i].location_next;

		if(pool->chunk[i].req_sum_all==0)
		{
			/*No Access*/
			if(pool->record_all[i].accessed!=0)
			{
				pool->i_noaccess++;
			}
			pool->chunk[i].pattern=PATTERN_NOACCESS;
			pool->chunk[i].location_next=POOL_HDD;
		}
		else if(pool->chunk[i].req_sum_all < pool->threshold_inactive)
		{
			/*Inactive*/
			pool->i_inactive++;
			pool->chunk[i].pattern=PATTERN_INACTIVE;
			pool->chunk[i].location_next=POOL_SSD;
		}
		else 
		{
			/*Active*/
			pool->i_active++;
			pool->chunk[i].pattern=PATTERN_ACTIVE;
			pool->chunk[i].location_next=POOL_SCM;			
		}	
		pool->chunk[i].history_pattern[pool->window_sum]=pool->chunk[i].pattern;
	}
	if(pool->i_active > pool->chunk_scm)//if SCM can hold all active chks or not
	{
		/*2nd Level in Hierachical Classifier*/
		for(i=pool->chunk_min;i<=pool->chunk_max;i++)
		{
			//move inactive chunks to HDD
			if(pool->chunk[i].pattern==PATTERN_INACTIVE)
			{
				pool->chunk[i].location_next=POOL_HDD;
			}
	
			if(pool->chunk[i].pattern==PATTERN_ACTIVE)
			{
				if (((long double)pool->chunk[i].req_sum_read/(long double)pool->chunk[i].req_sum_all)
					>= pool->threshold_rw)
				{
					/*Read*/
					pool->i_active--;
					pool->i_active_r++;
					pool->chunk[i].pattern=PATTERN_ACTIVE_READ;
					pool->chunk[i].location_next=POOL_SSD;	
				}
				else
				{
					/*Write*/
					pool->i_active--;
					pool->i_active_w++;
					pool->chunk[i].pattern=PATTERN_ACTIVE_WRITE;
					pool->chunk[i].location_next=POOL_SCM;
				}				
			}
			pool->chunk[i].history_pattern[pool->window_sum]=pool->chunk[i].pattern;
		}//for		
		
		//If SCM+SSD cannot hold all active chks.
		if((pool->i_active+pool->i_active_r+pool->i_active_w) > (pool->chunk_scm+pool->chunk_ssd))
		{
			/*3rd Level in Hierachical Classifier*/
			for(i=pool->chunk_min;i<=pool->chunk_max;i++)
			{
				if(pool->chunk[i].pattern==PATTERN_ACTIVE_READ)
				{
					if(((long double)pool->chunk[i].seq_size_all/(long double)pool->chunk[i].req_size_all)>=pool->threshold_cbr &&
					((long double)pool->chunk[i].seq_sum_all/(long double)pool->chunk[i].req_sum_all)>=pool->threshold_car)
					{
						/*Sequential*/
						pool->i_active_r--;
						pool->i_active_r_s++;
						pool->chunk[i].pattern=PATTERN_ACTIVE_R_SEQ;
						pool->chunk[i].location_next=POOL_HDD;
					}
					else
					{
						/*Random*/
						pool->i_active_r--;
						pool->i_active_r_m++;
						pool->chunk[i].pattern=PATTERN_ACTIVE_R_RDM;
						pool->chunk[i].location_next=POOL_SSD;
					}		
				}
				else if(pool->chunk[i].pattern==PATTERN_ACTIVE_WRITE)
				{
					if(((long double)pool->chunk[i].seq_size_all/(long double)pool->chunk[i].req_size_all)>=pool->threshold_cbr &&
					((long double)pool->chunk[i].seq_sum_all/(long double)pool->chunk[i].req_sum_all)>=pool->threshold_car)
					{
						/*Sequential*/
						pool->i_active_w--;
						pool->i_active_w_s++;
						pool->chunk[i].pattern=PATTERN_ACTIVE_W_SEQ;
						pool->chunk[i].location_next=POOL_HDD;
					}
					else
					{
						/*Random*/
						pool->i_active_w--;
						pool->i_active_w_m++;
						pool->chunk[i].pattern=PATTERN_ACTIVE_W_RDM;
						pool->chunk[i].location_next=POOL_SCM;
					}
				}		
				pool->chunk[i].history_pattern[pool->window_sum]=pool->chunk[i].pattern;
			}//for
			
			if(pool->i_active_w_m > pool->chunk_scm)
			{
				/*4th Level in Hierachical Classifier*/
				for(i=pool->chunk_min;i<=pool->chunk_max;i++)
				{
					if(pool->chunk[i].pattern==PATTERN_ACTIVE_W_RDM)
					{
						if(pool->chunk[i].req_sum_all>=(pool->req_in_window/pool->chunk_win)*pool->threshold_intensive)
						{
							/*Overwritten*/
							pool->i_active_w_m--;
							pool->i_active_w_m_o++;
							pool->chunk[i].pattern=PATTERN_ACTIVE_W_RDM_OVER;
							pool->chunk[i].location_next=POOL_SCM;
						}
						else
						{
							/*Fully Random*/
							pool->i_active_w_m--;
							pool->i_active_w_m_f++;
							pool->chunk[i].pattern=PATTERN_ACTIVE_W_RDM_FULY;
							pool->chunk[i].location_next=POOL_SSD;
						}
					}
					pool->chunk[i].history_pattern[pool->window_sum]=pool->chunk[i].pattern;
				}//for
			}//if
		}//if
	}//if
		
	for(i=pool->chunk_min;i<=pool->chunk_max;i++)
	{		
		/************************************
			update mapping information
		*************************************/
		update_map(pool,i);
		//print_log(pool,i);	//print info of each chunk in this window to log file.
		/*Initialize the statistics in each chunk*/
		pool->chunk[i].req_sum_all=0;
		pool->chunk[i].req_sum_read=0;
		pool->chunk[i].req_sum_write=0;
		pool->chunk[i].req_size_all=0;
		pool->chunk[i].req_size_read=0;
		pool->chunk[i].req_size_write=0;

		pool->chunk[i].seq_sum_all=0;
		pool->chunk[i].seq_sum_read=0;
		pool->chunk[i].seq_sum_write=0;
		pool->chunk[i].seq_size_all=0;
		pool->chunk[i].seq_size_read=0;
		pool->chunk[i].seq_size_write=0;

		pool->chunk[i].seq_stream_all=0;
		pool->chunk[i].seq_stream_read=0;
		pool->chunk[i].seq_stream_write=0;		
	}//for
	
	//Only record limited information (the first SIZE_ARRY windows)
	if(pool->window_sum<SIZE_ARRAY)
	{
		pool->chunk_access[pool->window_sum]=pool->chunk_win;
		
		pool->pattern_noaccess[pool->window_sum]=pool->i_noaccess/(double)pool->chunk_all;
		pool->pattern_inactive[pool->window_sum]=pool->i_inactive/(double)pool->chunk_all;
		pool->pattern_active[pool->window_sum]=pool->i_active/(double)pool->chunk_all;
		pool->pattern_active_read[pool->window_sum]=pool->i_active_r/(double)pool->chunk_all;
		pool->pattern_active_write[pool->window_sum]=pool->i_active_w/(double)pool->chunk_all;
		pool->pattern_active_r_seq[pool->window_sum]=pool->i_active_r_s/(double)pool->chunk_all;
		pool->pattern_active_r_rdm[pool->window_sum]=pool->i_active_r_m/(double)pool->chunk_all;
		pool->pattern_active_w_seq[pool->window_sum]=pool->i_active_w_s/(double)pool->chunk_all;
		pool->pattern_active_w_rdm[pool->window_sum]=pool->i_active_w_m/(double)pool->chunk_all;
		pool->pattern_active_w_rdm_fuly[pool->window_sum]=pool->i_active_w_m_o/(double)pool->chunk_all;
		pool->pattern_active_w_rdm_over[pool->window_sum]=pool->i_active_w_m_f/(double)pool->chunk_all;
	}

	/*Update the pool info*/
	pool->window_sum++;
	if(pool->window_sum%100==0)
	{
		printf("------------pool->window_sum=%d---------------\n",pool->window_sum);
	}
	pool->window_time_start=0;
	pool->window_time_end=0;

	/*Start a new window*/
	pool->size_in_window=0;
	pool->req_in_window=0;
	pool->time_in_window=0;

	pool->i_noaccess=0;
	pool->i_inactive=0;
	pool->i_active=0;
	pool->i_active_r=0;
	pool->i_active_w=0;
	pool->i_active_r_s=0;
	pool->i_active_r_m=0;
	pool->i_active_w_s=0;
	pool->i_active_w_m=0;
	pool->i_active_w_m_f=0;
	pool->i_active_w_m_o=0;

	//accessed chunks in each window
	memset(pool->record_win,0,sizeof(struct record_info)*pool->chunk_sum);
	printf("pool->chunk_win=%d\n",pool->chunk_win);
	pool->chunk_win=0;
}

/****************************************
			Bubble Sort
****************************************/
void bubble_sort(unsigned int a[],unsigned int b[],int n)
{
	int i,j;
	int flag=0;
	int temp,temp1;
	j=n;
	printf("[5] bubble sorting....\n");
	while(flag==0)
	{
		flag=1;
		for(i=0;i<n;i++)
		{
			for(j=i+1;j<n;j++)
			{
				if(a[i]<a[j])
				{
					//sort base on weight
					temp=a[j];
					a[j]=a[i];
					a[i]=temp;
					//record initial sequence
					temp1=b[j];
					b[j]=b[i];
					b[i]=temp1;
					
					flag=0;
				}
			}//for
		}//for
	}//while
}
int find_num(unsigned int a[],int n)
{
	int i;
	for(i=0;i<100000;i++)
	{
		if(a[i] == n)
		{
			return i;
		}
	}
	return -1;
}
/**********************************************************************
*       							IOPS only
*      							weight = f(IOPS)
* ********************************************************************/

void pattern_recognize_iops(struct pool_info *pool)
{
	unsigned int i,m;
	unsigned int weight[100000];	//IOPS only
	unsigned int num[100000];	
	
	for(i=0;i<100000;i++)
	{
		weight[i]=i;
		num[i]=i;
	}
	
	/*Pattern Detection*/
	pool->time_in_window=(long double)(pool->window_time_end-pool->window_time_start)/(long double)1000000;
	
	for(i=pool->chunk_min;i<=pool->chunk_max;i++)
	{
		pool->chunk[i].location=pool->chunk[i].location_next;
		
		weight[i]=pool->chunk[i].req_sum_all;		
	}
	
	for(i=0;i<50;i++)
	{
		printf("++%d %d\n",weight[i],num[i]);
	}
	/*get the sorted chunk sequence base on IOPS*/
	bubble_sort(weight,pool->chunk_sum);
	for(i=0;i<50;i++)
	{
		printf("--%d %d\n",weight[i],num[i]);
	}
	
	for(i=pool->chunk_min;i<=pool->chunk_max;i++)
	{
		m=find_num(num,i);
		if(m == -1)
		{
			printf("Error in IOPS only \n");
			exit(-1);
		}
		//tier-by-tier migration in chunk level
		if(pool->chunk[i].location == POOL_SCM)
		{
			if(m < pool->chunk_scm)
			{
				pool->chunk[i].location_next=POOL_SCM;
			}
			else
			{
				pool->chunk[i].location_next=POOL_SSD;
			}
		}
		if(pool->chunk[i].location == POOL_SSD)
		{
			if(m < pool->chunk_scm)
			{
				pool->chunk[i].location_next=POOL_SCM;
			}
			else if(m < pool->chunk_scm + pool->chunk_ssd)
			{
				pool->chunk[i].location_next=POOL_SSD;
			}
			else
			{
				pool->chunk[i].location_next=POOL_HDD;
			}
		}
		if(pool->chunk[i].location == POOL_HDD)
		{
			if(m < pool->chunk_scm + pool->chunk_ssd)
			{
				pool->chunk[i].location_next=POOL_SSD;
			}
			else
			{
				pool->chunk[i].location_next=POOL_HDD;
			}
		}
	}//for
		
	for(i=pool->chunk_min;i<=pool->chunk_max;i++)
	{		
		/************************************
			update mapping information
		*************************************/
		update_map(pool,i);
		//print_log(pool,i);	//print info of each chunk in this window to log file.
		/*Initialize the statistics in each chunk*/
		pool->chunk[i].req_sum_all=0;
		pool->chunk[i].req_sum_read=0;
		pool->chunk[i].req_sum_write=0;
		pool->chunk[i].req_size_all=0;
		pool->chunk[i].req_size_read=0;
		pool->chunk[i].req_size_write=0;

		pool->chunk[i].seq_sum_all=0;
		pool->chunk[i].seq_sum_read=0;
		pool->chunk[i].seq_sum_write=0;
		pool->chunk[i].seq_size_all=0;
		pool->chunk[i].seq_size_read=0;
		pool->chunk[i].seq_size_write=0;

		pool->chunk[i].seq_stream_all=0;
		pool->chunk[i].seq_stream_read=0;
		pool->chunk[i].seq_stream_write=0;		
	}//for

	/*Update the pool info*/
	pool->window_sum++;
	if(pool->window_sum%100==0)
	{
		printf("------------pool->window_sum=%d---------------\n",pool->window_sum);
	}
	pool->window_time_start=0;
	pool->window_time_end=0;

	/*Start a new window*/
	pool->size_in_window=0;
	pool->req_in_window=0;
	pool->time_in_window=0;

	pool->i_noaccess=0;
	pool->i_inactive=0;
	pool->i_active=0;
	pool->i_active_r=0;
	pool->i_active_w=0;
	pool->i_active_r_s=0;
	pool->i_active_r_m=0;
	pool->i_active_w_s=0;
	pool->i_active_w_m=0;
	pool->i_active_w_m_f=0;
	pool->i_active_w_m_o=0;

	//accessed chunks in each window
	memset(pool->record_win,0,sizeof(struct record_info)*pool->chunk_sum);
	printf("pool->chunk_win=%d\n",pool->chunk_win);
	pool->chunk_win=0;
}