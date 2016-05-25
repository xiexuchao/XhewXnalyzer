
inline long long time_now()
{
	struct timeval now;
	gettimeofday(&now,NULL);
	return 1000000*now.tv_sec+now.tv_usec;	//us
}

inline long long time_elapsed(long long begin)
{
	return time_now()-begin;	//us
}
