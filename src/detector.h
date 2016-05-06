#ifndef _DETECTOR_H
#define _DETECTOR_H

/*Sequential Accesses Detection*/
struct stream_info{
	unsigned int chk_id;
	unsigned int type;	//read or write
	unsigned int sum;	//IO requests absorbed in
	unsigned int size;	//Sectors(512Bytes)
	long long min;		//start lba
	long long max;		//current max lba
	long long time;		//time when the first req arrived
};

#endif
