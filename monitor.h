#ifndef __MESSAGING_HEADER
#define __MESSAGING_HEADER

#include "mpi.h"
#include "common.h"


typedef unsigned long long scalar_clock_t;

typedef struct packet {
	int message[SIZE_OF_MSG];
	scalar_clock_t clock;
} message;


typedef struct monitor_state {
	scalar_clock_t clock;
} monitor_state;


static monitor_state state;



void init(void);


#endif // __MESSAGING_HEADER
