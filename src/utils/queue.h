#ifndef _QUEUE_H
#define _QUEUE_H

#include "../pool.h"

//queue.c
void queue_push(struct trace_info *trace,struct req_info *req);
void queue_pop(struct trace_info *trace,struct req_info *req);
void queue_print(struct trace_info *trace);

#endif