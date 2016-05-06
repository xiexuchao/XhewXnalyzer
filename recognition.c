#include "pool.h"

void pool_run(char *trace,char *config,char *output,char *log)
{
	struct pool_info *pool;
	/*for trace replayer*/


	pool=(struct pool_info *)malloc(sizeof(struct pool_info));
	alloc_assert(pool,"pool");
	memset(pool,0,sizeof(struct pool_info));

	load_parameters(pool,config);
	initialize(pool,trace,output,log);
	warmup(pool);

	while(get_request(pool)!=FAILURE)
	{			
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

		if(((pool->window_time_end-pool->window_time_start)>=pool->window_size*60*1000*1000)||((feof(pool->file_trace)!=0)&&(pool->window_time_end>0)))
		{
			/*printf("----------------\n");
			printf("req wind=%d \n",pool->req_in_window);
			printf("req time=%lld \n",pool->req->time);
			printf("beg time=%lld \n",pool->window_time_start);
			printf("end time=%lld \n",pool->window_time_end);
			printf("win time=%lld \n",pool->window_time_end-pool->window_time_start);*/

			stream_flush(pool);
			pattern_recognize(pool);
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
	free(pool);
}

void pattern_recognize(struct pool_info *pool)
{
	unsigned int i;
	/*Pattern Detection*/
	//seconds
	pool->time_in_window=(long double)(pool->window_time_end-pool->window_time_start)/(long double)1000;
	
	for(i=pool->chunk_min;i<=pool->chunk_max;i++)
	{
		pool->chunk[i].pattern_last=pool->chunk[i].pattern;
		pool->chunk[i].location=pool->chunk[i].location_next;

		if(pool->chunk[i].req_sum_all==0)//no access
		{
			/*No Access*/
			if(pool->record_all[i].accessed!=0)
			{
				pool->i_noaccess++;
			}
			pool->chunk[i].pattern=PATTERN_NOACCESS;
			pool->chunk[i].location_next=POOL_HDD;
		}
		else if(pool->chunk[i].req_sum_all < pool->threshold_inactive)//inactive
		{
			/*Inactive*/
			pool->i_inactive++;
			pool->chunk[i].pattern=PATTERN_INACTIVE;
			pool->chunk[i].location_next=POOL_HDD;
		}
		else if(((long double)pool->chunk[i].seq_size_all/(long double)pool->chunk[i].req_size_all)>=pool->threshold_cbr &&
			((long double)pool->chunk[i].seq_sum_all/(long double)pool->chunk[i].req_sum_all)>=pool->threshold_car)
		{
			/*SEQUENTIAL*/
			if(((long double)pool->chunk[i].req_sum_read/(long double)pool->chunk[i].req_sum_all)>=pool->threshold_rw)
			{
				/*Sequential Read*/
				pool->i_active_seq_r++;
				pool->chunk[i].pattern=PATTERN_ACTIVE_SEQ_R;
				pool->chunk[i].location_next=POOL_SSD;
			}
			else
			{
				/*Sequential Write*/
				pool->i_active_seq_w++;
				pool->chunk[i].pattern=PATTERN_ACTIVE_SEQ_W;
				pool->chunk[i].location_next=POOL_SCM;
			}
		}
		else
		{
			/*Random*/
			if(pool->chunk[i].req_sum_all>=(pool->req_in_window/pool->chunk_win)*pool->threshold_intensive)
			{
				if(((long double)pool->chunk[i].req_sum_read/(long double)pool->chunk[i].req_sum_all)>=pool->threshold_rw)
				{
					/*Active Random Read*/
					pool->i_active_rdm_over_r++;
					pool->chunk[i].pattern=PATTERN_ACTIVE_RDM_OVER_R;
					pool->chunk[i].location_next=POOL_SSD;
				}
				else
				{
					/*Active Random Write*/
					pool->i_active_rdm_over_w++;
					pool->chunk[i].pattern=PATTERN_ACTIVE_RDM_OVER_W;
					pool->chunk[i].location_next=POOL_SCM;
				}
			}
			else
			{
				if(((long double)pool->chunk[i].req_sum_read/(long double)pool->chunk[i].req_sum_all)>=pool->threshold_rw)
				{
					/*Random Less Intensive Read*/
					pool->i_active_rdm_fuly_r++;
					pool->chunk[i].pattern=PATTERN_ACTIVE_RDM_FULY_R;
					pool->chunk[i].location_next=POOL_SSD;
				}
				else
				{
					/*Random Less Intensive Write*/
					pool->i_active_rdm_fuly_w++;
					pool->chunk[i].pattern=PATTERN_ACTIVE_RDM_FULY_W;
					pool->chunk[i].location_next=POOL_SSD;
				}
			}
		}
		//Only record limited information (the first SIZE_ARRY windows)
		if(pool->window_sum<SIZE_ARRAY)
		{
			pool->chunk_access[pool->window_sum]=pool->chunk_win;

			pool->chunk[i].history_pattern[pool->window_sum]=pool->chunk[i].pattern;

			pool->pattern_noaccess[pool->window_sum]=pool->i_noaccess/(double)pool->chunk_all;
			pool->pattern_inactive[pool->window_sum]=pool->i_inactive/(double)pool->chunk_all;
			pool->pattern_active_seq_r[pool->window_sum]=pool->i_active_seq_r/(double)pool->chunk_all;
			pool->pattern_active_seq_w[pool->window_sum]=pool->i_active_seq_w/(double)pool->chunk_all;
			pool->pattern_active_rdm_over_r[pool->window_sum]=pool->i_active_rdm_over_r/(double)pool->chunk_all;
			pool->pattern_active_rdm_over_w[pool->window_sum]=pool->i_active_rdm_over_w/(double)pool->chunk_all;
			pool->pattern_active_rdm_fuly_r[pool->window_sum]=pool->i_active_rdm_fuly_r/(double)pool->chunk_all;
			pool->pattern_active_rdm_fuly_w[pool->window_sum]=pool->i_active_rdm_fuly_w/(double)pool->chunk_all;
		}

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

		/************************************
			update mapping information
		*************************************/
		update_map(pool,i);		
	}//for

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
	pool->i_active_seq_r=0;
	pool->i_active_seq_w=0;
	pool->i_active_rdm_over_r=0;
	pool->i_active_rdm_over_w=0;
	pool->i_active_rdm_fuly_r=0;
	pool->i_active_rdm_fuly_w=0;

	//accessed chunks in each window
	memset(pool->record_win,0,sizeof(struct record_info)*pool->chunk_sum);
	printf("pool->chunk_win=%d\n",pool->chunk_win);
	pool->chunk_win=0;
}