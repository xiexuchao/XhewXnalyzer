#include "pool.h"

/*
	return PCN that free in pool type
*/
int find_free(struct pool_info *pool,int type)
{
	unsigned int i;
	if(type==POOL_SCM)
	{
		for(i=0;i<pool->chunk_scm;i++)
		{
			if(pool->revTab[i].lcn==-1)
			{
				return i;
			}
		}
		return -1;
	}//if
	else if(type==POOL_SSD)
	{
		for(i=pool->chunk_scm;i<(pool->chunk_scm+pool->chunk_ssd);i++)
		{
			if(pool->revTab[i].lcn==-1)
			{
				return i;
			}
		}
		return -1;
	}//else
	else
	{
		for(i=pool->chunk_scm+pool->chunk_ssd;i<pool->chunk_sum;i++)
		{
			if(pool->revTab[i].lcn==-1)
			{
				return i;
			}			
		}
		return -1;
	}//else
}//end

void update_map(struct pool_info *pool,int i)
{
	int free_chk;
	if(pool->chunk[i].location==POOL_SCM)
	{
		if(pool->chunk[i].location_next==POOL_SCM)
		{
			pool->migrate_scm_scm++;
		}
		else if(pool->chunk[i].location_next==POOL_SSD)
		{
			free_chk=find_free(pool,POOL_SSD);
			if(free_chk!=-1)
			{
				pool->migrate_scm_ssd++;
				pool->revTab[pool->mapTab[i].pcn].lcn=-1;	//free the old space

				pool->mapTab[i].pcn=free_chk;
				pool->revTab[free_chk].lcn=i;
			}
			else //else give up migration
			{
				printf("[SCM]-SSD is full now\n");
				pool->migrate_scm_scm++;
				pool->chunk[i].location_next=POOL_SCM;
			}
		}
		else
		{
			free_chk=find_free(pool,POOL_HDD);
			if(free_chk!=-1)
			{
				pool->migrate_scm_hdd++;
				pool->revTab[pool->mapTab[i].pcn].lcn=-1;
				
				pool->mapTab[i].pcn=free_chk;
				pool->revTab[free_chk].lcn=i;
			}
			else
			{
				printf("[SCM]-HDD is full now\n");
				pool->migrate_scm_scm++;
				pool->chunk[i].location_next=POOL_SCM;
			}
		}
	}//if
	else if(pool->chunk[i].location==POOL_SSD)
	{
		if(pool->chunk[i].location_next==POOL_SCM)
		{
			free_chk=find_free(pool,POOL_SCM);
			if(free_chk!=-1)
			{
				pool->migrate_ssd_scm++;
				pool->revTab[pool->mapTab[i].pcn].lcn=-1;

				pool->mapTab[i].pcn=free_chk;
				pool->revTab[free_chk].lcn=i;
			}
			else	//else give up migration
			{
				printf("[SSD]-SCM is full now\n");
				pool->migrate_ssd_ssd++;
				pool->chunk[i].location_next=POOL_SSD;
			}
		}
		else if(pool->chunk[i].location_next==POOL_SSD)
		{
			pool->migrate_ssd_ssd++;
		}
		else
		{
			free_chk=find_free(pool,POOL_HDD);
			if(free_chk!=-1)
			{
				pool->migrate_ssd_hdd++;
				pool->revTab[pool->mapTab[i].pcn].lcn=-1;
				
				pool->mapTab[i].pcn=free_chk;
				pool->revTab[free_chk].lcn=i;
			}
			else //else give up migration
			{
				printf("[SSD]-HDD is full now\n");
				pool->migrate_ssd_ssd++;
				pool->chunk[i].location_next=POOL_SSD;
			}
		}
	}//else
	else //if(pool->chunk[i].location_next==POOL_HDD)
	{
		if(pool->chunk[i].location_next==POOL_SCM)
		{
			free_chk=find_free(pool,POOL_SCM);
			if(free_chk!=-1)
			{
				pool->migrate_hdd_scm++;
				pool->revTab[pool->mapTab[i].pcn].lcn=-1;

				pool->mapTab[i].pcn=free_chk;
				pool->revTab[free_chk].lcn=i;
			}
			else	//else give up migration
			{
				printf("[HDD]-SCM is full now\n");
				pool->migrate_hdd_hdd++;
				pool->chunk[i].location_next=POOL_HDD;
			}
		}
		else if(pool->chunk[i].location_next==POOL_SSD)
		{
			free_chk=find_free(pool,POOL_SSD);
			if(free_chk!=-1)
			{
				pool->migrate_hdd_ssd++;
				pool->revTab[pool->mapTab[i].pcn].lcn=-1;	//free the old space

				pool->mapTab[i].pcn=free_chk;
				pool->revTab[free_chk].lcn=i;
			}
			else
			{
				printf("[HDD]-SSD is full now\n");
				pool->migrate_hdd_hdd++;
				pool->chunk[i].location_next=POOL_HDD;
			}
		}
		else
		{
			pool->migrate_hdd_hdd++;
		}
	}//else
}//end
