#include "pool.h"

int get_request(struct pool_info *pool)
{
	double time1;
	unsigned int dev;
	long long lba;
	unsigned int size;
	unsigned int type;

	time_t time_current;

	if(feof(pool->file_trace))
	{
		printf("************Read file <%s> end************\n",pool->filename_trace);
		return FAILURE;
	}

	fgets(pool->buffer,SIZE_BUFFER,pool->file_trace);
	sscanf(pool->buffer,"%lf %d %lld %d %d\n",&time1,&dev,&lba,&size,&type);

	pool->req->time=(long long)(time1*1000);	//ms -> us
	pool->req->type=type;
	pool->req->lba=lba;
	pool->req->size=size;

	if(pool->req_sum_all%1000000==0)
	{
		time(&time_current);
		printf("Analyzing(%s)%d @%s",pool->filename_trace,pool->req_sum_all,ctime(&time_current));
	}
	return SUCCESS;
} 

void stat_update(struct pool_info *pool)
{
	unsigned int chk_id=(unsigned int)(pool->req->lba/(pool->size_chk*2048));

	pool->req_sum_all++;
	pool->req_size_all+=(long double)pool->req->size/2048;	//blks-->KB
	pool->chunk[chk_id].req_sum_all++;
	pool->chunk[chk_id].req_size_all+=(long double)pool->req->size/2048;
	if(pool->req->type==READ)
	{
		pool->req_sum_read++;
		pool->req_size_read+=(long double)pool->req->size/2048;
		pool->chunk[chk_id].req_sum_read++;
		pool->chunk[chk_id].req_size_read+=(long double)pool->req->size/2048;
	}
	else
	{
		pool->req_sum_write++;
		pool->req_size_write+=(long double)pool->req->size/2048;
		pool->chunk[chk_id].req_sum_write++;
		pool->chunk[chk_id].req_size_write+=(long double)pool->req->size/2048;
	}
	if(pool->record_win[chk_id].accessed==0)
	{
		pool->chunk_win++;
	}
	pool->record_win[chk_id].accessed=1;
}

void stat_print(struct pool_info *pool)
{
	unsigned int i,j;
	long double temp,temp2;

	fprintf(pool->file_output,"%-30s	%s\n","Trace file",pool->filename_trace);
	fprintf(pool->file_output,"\n------------Information of Storage Pool------------\n");
	fprintf(pool->file_output,"%-30s	%d\n","Size of SCM (GB)",pool->size_scm);
	fprintf(pool->file_output,"%-30s	%d\n","Size of SSD (GB)",pool->size_ssd);
	fprintf(pool->file_output,"%-30s	%d\n","Size of HDD (GB)",pool->size_hdd);
	fprintf(pool->file_output,"%-30s	%d\n","Size of chk (MB)",pool->size_chk);

	fprintf(pool->file_output,"%-30s	%d\n","Size of stream",pool->size_stream);
	fprintf(pool->file_output,"%-30s	%d\n","Size of stride  (KB)",pool->size_stride/2);

	fprintf(pool->file_output,"%-30s	%d\n","Chunk sum",pool->chunk_sum);
	fprintf(pool->file_output,"%-30s	%d\n","Chunk max",pool->chunk_max);
	fprintf(pool->file_output,"%-30s	%d\n","Chunk min",pool->chunk_min);
	fprintf(pool->file_output,"%-30s	%-20d	(%Lf%%)\n","Chunk all",pool->chunk_all,100*(long double)pool->chunk_all/(long double)pool->chunk_sum);

	fprintf(pool->file_output,"%-30s	%d\n","Chunks in scm",pool->chunk_scm);
	fprintf(pool->file_output,"%-30s	%d\n","Chunks in ssd",pool->chunk_ssd);
	fprintf(pool->file_output,"%-30s	%d\n","Chunks in hdd",pool->chunk_hdd);
	fprintf(pool->file_output,"%-30s	%d\n","Free Chunks in scm",pool->free_chk_scm);
	fprintf(pool->file_output,"%-30s	%d\n","Free Chunks in ssd",pool->free_chk_ssd);
	fprintf(pool->file_output,"%-30s	%d\n","Free Chunks in hdd",pool->free_chk_hdd);

	fprintf(pool->file_output,"%-30s	%d\n","Window size (min)",pool->window_size);

	fprintf(pool->file_output,"%-30s	%lf\n","Threshold for R/W",pool->threshold_rw);
	fprintf(pool->file_output,"%-30s	%lf\n","Threshold for Seq.CBR (Byte)",pool->threshold_cbr);
	fprintf(pool->file_output,"%-30s	%lf\n","Threshold for Seq.CAR (Access)",pool->threshold_car);
	fprintf(pool->file_output,"%-30s	%d\n","Threshold for Seq.size(KB)",pool->threshold_sequential/2);
	fprintf(pool->file_output,"%-30s	%d\n","Threshold for Inactive",pool->threshold_inactive);
	fprintf(pool->file_output,"%-30s	%d\n","Threshold for Intensive",pool->threshold_intensive);
	fflush(pool->file_output);

	fprintf(pool->file_output,"\n------------Information of IO Trace------------\n");
	fprintf(pool->file_output,"%-30s	%Lf\n","Trace start time (Minutes)",(long double)pool->time_start/60000000);
	fprintf(pool->file_output,"%-30s	%Lf\n","Trace  end  time (Minutes)",(long double)pool->time_end/60000000);
	fprintf(pool->file_output,"%-30s	%d\n","Num of windows",pool->window_sum);
	
	fprintf(pool->file_output,"----IO Request--\n");
	fprintf(pool->file_output,"%-30s	%d\n","Num of all  IO ",pool->req_sum_all);
	fprintf(pool->file_output,"%-30s	%-20d	(%Lf%%)\n","Num of read IO",pool->req_sum_read,100*(long double)pool->req_sum_read/(long double)pool->req_sum_all);
	fprintf(pool->file_output,"%-30s	%-20d	(%Lf%%)\n","Num of wrte IO",pool->req_sum_write,100*(long double)pool->req_sum_write/(long double)pool->req_sum_all);
	fprintf(pool->file_output,"%-30s	%Lf\n","Size of all  IO (MB)",pool->req_size_all);
	fprintf(pool->file_output,"%-30s	%-20Lf	(%Lf%%)\n","Size of read IO (MB)",pool->req_size_read,100*pool->req_size_read/pool->req_size_all);
	fprintf(pool->file_output,"%-30s	%-20Lf	(%Lf%%)\n","Size of wrte IO (MB)",pool->req_size_write,100*pool->req_size_write/pool->req_size_all);
	fprintf(pool->file_output,"%-30s	%Lf\n","Avg Size of all  IO (MB)",pool->req_size_all/pool->req_sum_all);
	fprintf(pool->file_output,"%-30s	%Lf\n","Avg Size of read IO (MB)",pool->req_size_read/pool->req_sum_read);
	fprintf(pool->file_output,"%-30s	%Lf\n","Avg Size of wrte IO (MB)",pool->req_size_write/pool->req_sum_write);
	
	fprintf(pool->file_output,"----Sequential IO Request--\n");
	fprintf(pool->file_output,"%-30s	%d\n","Num of Seq.all  IO",pool->seq_sum_all);
	fprintf(pool->file_output,"%-30s	%-20d	(%Lf%%)\n","Num of Seq.read IO",pool->seq_sum_read,100*(long double)pool->seq_sum_read/(long double)pool->seq_sum_all);
	fprintf(pool->file_output,"%-30s	%-20d	(%Lf%%)\n","Num of Seq.wrte IO",pool->seq_sum_write,100*(long double)pool->seq_sum_write/(long double)pool->seq_sum_all);
	fprintf(pool->file_output,"%-30s	%Lf\n","Size of Seq.all  IO (MB)",pool->seq_size_all);
	fprintf(pool->file_output,"%-30s	%-20Lf	(%Lf%%)\n","Size of Seq.read IO (MB)",pool->seq_size_read,100*pool->seq_size_read/pool->seq_size_all);
	fprintf(pool->file_output,"%-30s	%-20Lf	(%Lf%%)\n","Size of Seq.wrte IO (MB)",pool->seq_size_write,100*pool->seq_size_write/pool->seq_size_all);
	fprintf(pool->file_output,"%-30s	%Lf\n","Avg Size of Seq.all  IO (MB)",pool->seq_size_all/pool->seq_sum_all);
	fprintf(pool->file_output,"%-30s	%Lf\n","Avg Size of Seq.read IO (MB)",pool->seq_size_read/pool->seq_sum_read);
	fprintf(pool->file_output,"%-30s	%Lf\n","Avg Size of Seq.wrte IO (MB)",pool->seq_size_write/pool->seq_sum_write);
	
	fprintf(pool->file_output,"----Sequential Stream--\n");
	fprintf(pool->file_output,"%-30s	%d\n","Num of Seq.all  stream",pool->seq_stream_all);
	fprintf(pool->file_output,"%-30s	%-20d	(%Lf%%)\n","Num of Seq.read stream",pool->seq_stream_read,100*(long double)pool->seq_stream_read/(long double)pool->seq_stream_all);
	fprintf(pool->file_output,"%-30s	%-20d	(%Lf%%)\n","Num of Seq.wrte stream",pool->seq_stream_write,100*(long double)pool->seq_stream_write/(long double)pool->seq_stream_all);
	fprintf(pool->file_output,"%-30s	%Lf MB\n","Avg Size of Seq.all  stream",pool->seq_size_all/pool->seq_stream_all);
	fprintf(pool->file_output,"%-30s	%Lf MB\n","Avg Size of Seq.read stream",pool->seq_size_read/pool->seq_stream_read);
	fprintf(pool->file_output,"%-30s	%Lf MB\n","Avg Size of Seq.wrte stream",pool->seq_size_write/pool->seq_stream_write);
	fflush(pool->file_output);

	fprintf(pool->file_output,"\n------------Information of Data Migration------------\n");
	fprintf(pool->file_output,"%-30s	%d\n","SCM to SCM",pool->migrate_scm_scm);
	fprintf(pool->file_output,"%-30s	%d\n","SCM to SSD",pool->migrate_scm_ssd);
	fprintf(pool->file_output,"%-30s	%d\n","SCM to HDD",pool->migrate_scm_hdd);
	fprintf(pool->file_output,"%-30s	%d\n","SSD to SCM",pool->migrate_ssd_scm);
	fprintf(pool->file_output,"%-30s	%d\n","SSD to SSD",pool->migrate_ssd_ssd);
	fprintf(pool->file_output,"%-30s	%d\n","SSD to HDD",pool->migrate_ssd_hdd);
	fprintf(pool->file_output,"%-30s	%d\n","HDD to SCM",pool->migrate_hdd_scm);
	fprintf(pool->file_output,"%-30s	%d\n","HDD to SSD",pool->migrate_hdd_ssd);
	fprintf(pool->file_output,"%-30s	%d\n","HDD to HDD",pool->migrate_hdd_hdd);
	fflush(pool->file_output);

	fprintf(pool->file_output,"\n------------Information of IO Pattern Ratio------------\n");	//IO pattern ratio of each window
	//total pattern ratio
	temp=0;
	for(j=0;j<pool->window_sum;j++)
	{
		temp+=pool->pattern_noaccess[j];
	}
	temp2=temp/pool->window_sum;

	fprintf(pool->file_output,"%-30s","[pattern inactive]");
	temp=0;
	for(j=0;j<pool->window_sum;j++)
	{
		temp+=pool->pattern_inactive[j];
	}
	fprintf(pool->file_output,"%Lf\n",(temp/pool->window_sum)/(1-temp2));
	
	fprintf(pool->file_output,"%-30s","[pattern active]");
	temp=0;
	for(j=0;j<pool->window_sum;j++)
	{
		temp+=pool->pattern_active[j];
	}
	fprintf(pool->file_output,"%Lf\n",(temp/pool->window_sum)/(1-temp2));
	
	fprintf(pool->file_output,"%-30s","[pattern active_r]");
	temp=0;
	for(j=0;j<pool->window_sum;j++)
	{
		temp+=pool->pattern_active_read[j];
	}
	fprintf(pool->file_output,"%Lf\n",(temp/pool->window_sum)/(1-temp2));
	
	fprintf(pool->file_output,"%-30s","[pattern active_w]");
	temp=0;
	for(j=0;j<pool->window_sum;j++)
	{
		temp+=pool->pattern_active_write[j];
	}
	fprintf(pool->file_output,"%Lf\n",(temp/pool->window_sum)/(1-temp2));
	
	fprintf(pool->file_output,"%-30s","[pattern active_r_seq]");
	temp=0;
	for(j=0;j<pool->window_sum;j++)
	{
		temp+=pool->pattern_active_r_seq[j];
	}
	fprintf(pool->file_output,"%Lf\n",(temp/pool->window_sum)/(1-temp2));
	
	fprintf(pool->file_output,"%-30s","[pattern active_r_rdm]");
	temp=0;
	for(j=0;j<pool->window_sum;j++)
	{
		temp+=pool->pattern_active_r_rdm[j];
	}
	fprintf(pool->file_output,"%Lf\n",(temp/pool->window_sum)/(1-temp2));
	
	fprintf(pool->file_output,"%-30s","[pattern active_w_seq]");
	temp=0;
	for(j=0;j<pool->window_sum;j++)
	{
		temp+=pool->pattern_active_w_seq[j];
	}
	fprintf(pool->file_output,"%Lf\n",(temp/pool->window_sum)/(1-temp2));
	
	fprintf(pool->file_output,"%-30s","[pattern active_w_rdm]");
	temp=0;
	for(j=0;j<pool->window_sum;j++)
	{
		temp+=pool->pattern_active_w_rdm[j];
	}
	fprintf(pool->file_output,"%Lf\n",(temp/pool->window_sum)/(1-temp2));
	
	fprintf(pool->file_output,"%-30s","[pattern active_w_rdm_over]");
	temp=0;
	for(j=0;j<pool->window_sum;j++)
	{
		temp+=pool->pattern_active_w_rdm_over[j];
	}
	fprintf(pool->file_output,"%Lf\n",(temp/pool->window_sum)/(1-temp2));
	
	fprintf(pool->file_output,"%-30s","[pattern active_w_rdm_fuly]");
	temp=0;
	for(j=0;j<pool->window_sum;j++)
	{
		temp+=pool->pattern_active_w_rdm_fuly[j];
	}
	fprintf(pool->file_output,"%Lf\n",(temp/pool->window_sum)/(1-temp2));

	/********************************/
	//pattern ratio in each window
	if(pool->window_sum > SIZE_ARRAY)
	{
		pool->window_sum=SIZE_ARRAY;
	}

	fprintf(pool->file_output,"%-22s","[noaccess]");
	for(j=0;j<pool->window_sum;j++)
	{
		fprintf(pool->file_output,"%lf ",pool->pattern_noaccess[j]);
	}
	fprintf(pool->file_output,"\n");
	fprintf(pool->file_output,"%-22s","[inactive]");
	for(j=0;j<pool->window_sum;j++)
	{
		fprintf(pool->file_output,"%lf ",pool->pattern_inactive[j]);
	}
	fprintf(pool->file_output,"\n");
	fprintf(pool->file_output,"%-22s","[active]");
	for(j=0;j<pool->window_sum;j++)
	{
		fprintf(pool->file_output,"%lf ",pool->pattern_active[j]);
	}
	fprintf(pool->file_output,"\n");
	fprintf(pool->file_output,"%-22s","[active_r]");
	for(j=0;j<pool->window_sum;j++)
	{
		fprintf(pool->file_output,"%lf ",pool->pattern_active_read[j]);
	}
	fprintf(pool->file_output,"\n");
	fprintf(pool->file_output,"%-22s","[active_w]");
	for(j=0;j<pool->window_sum;j++)
	{
		fprintf(pool->file_output,"%lf ",pool->pattern_active_write[j]);
	}
	fprintf(pool->file_output,"\n");
	fprintf(pool->file_output,"%-22s","[active_r_seq]");
	for(j=0;j<pool->window_sum;j++)
	{
		fprintf(pool->file_output,"%lf ",pool->pattern_active_r_seq[j]);
	}
	fprintf(pool->file_output,"\n");
	fprintf(pool->file_output,"%-22s","[active_r_rdm]");
	for(j=0;j<pool->window_sum;j++)
	{
		fprintf(pool->file_output,"%lf ",pool->pattern_active_r_rdm[j]);
	}
	fprintf(pool->file_output,"\n");
	fprintf(pool->file_output,"%-22s","[active_w_seq]");
	for(j=0;j<pool->window_sum;j++)
	{
		fprintf(pool->file_output,"%lf ",pool->pattern_active_w_seq[j]);
	}
	fprintf(pool->file_output,"\n");
	fprintf(pool->file_output,"%-22s","[active_w_rdm]");
	for(j=0;j<pool->window_sum;j++)
	{
		fprintf(pool->file_output,"%lf ",pool->pattern_active_w_rdm[j]);
	}
	fprintf(pool->file_output,"\n");
	fprintf(pool->file_output,"%-22s","[active_w_rdm_over]");
	for(j=0;j<pool->window_sum;j++)
	{
		fprintf(pool->file_output,"%lf ",pool->pattern_active_w_rdm_over[j]);
	}
	fprintf(pool->file_output,"\n");
	fprintf(pool->file_output,"%-22s","[active_w_rdm_fuly]");
	for(j=0;j<pool->window_sum;j++)
	{
		fprintf(pool->file_output,"%lf ",pool->pattern_active_w_rdm_fuly[j]);
	}
	fprintf(pool->file_output,"\n");
	fflush(pool->file_output);

	fprintf(pool->file_output,"\n------------Information of IO Pattern in Each Window------------\n");
	fprintf(pool->file_output,"%-7s	%s\n","CHUNK_ID","PATTERN_HISTORY");
	for(i=0;i<pool->chunk_sum;i++)
	{
		if(pool->record_all[i].accessed!=0)
		{
			fprintf(pool->file_output,"%-7d	",i);
			for(j=0;j<pool->window_sum;j++)
			{
				fprintf(pool->file_output,"%c",pool->chunk[i].history_pattern[j]);
			}
			fprintf(pool->file_output,"\n");
		}
	}
	fflush(pool->file_output);
}

void alloc_assert(void *p,char *s)
{
	if(p!=NULL)
	{
		return;
	}
	printf("malloc %s error\n",s);
	getchar();
	exit(-1);
}

//void print_log(struct pool_info *pool,unsigned int i)
//{
//	if(pool->record_all[i].accessed!=0)
//	{
//		fprintf(pool->file_log,"[%d][%d] %-10s	%d\n",pool->window_sum,i,"a",pool->chunk[i].req_sum_all);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%d\n",pool->window_sum,i,"r ",pool->chunk[i].req_sum_read);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%d\n",pool->window_sum,i,"w ",pool->chunk[i].req_sum_write);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%Lf\n",pool->window_sum,i,"a (MB)",pool->chunk[i].req_size_all);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%Lf\n",pool->window_sum,i,"r (MB)",pool->chunk[i].req_size_read);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%Lf\n",pool->window_sum,i,"w (MB)",pool->chunk[i].req_size_write);
//
//		fprintf(pool->file_log,"[%d][%d] %-10s	%d\n",pool->window_sum,i,"Seq. a",pool->chunk[i].seq_sum_all);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%d\n",pool->window_sum,i,"Seq. r",pool->chunk[i].seq_sum_read);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%d\n",pool->window_sum,i,"Seq. w",pool->chunk[i].seq_sum_write);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%Lf\n",pool->window_sum,i,"Seq. a(MB)",pool->chunk[i].seq_size_all);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%Lf\n",pool->window_sum,i,"Seq. r(MB)",pool->chunk[i].seq_size_read);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%Lf\n",pool->window_sum,i,"Seq. w(MB)",pool->chunk[i].seq_size_write);
//
//		fprintf(pool->file_log,"[%d][%d] %-10s	%d\n",pool->window_sum,i,"Seq. a stream",pool->chunk[i].seq_stream_all);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%d\n",pool->window_sum,i,"Seq. r stream",pool->chunk[i].seq_stream_read);
//		fprintf(pool->file_log,"[%d][%d] %-10s	%d\n",pool->window_sum,i,"Seq. w stream",pool->chunk[i].seq_stream_write);
//		fprintf(pool->file_log,"\n");
//		fflush(pool->file_log);
//	}
//}
