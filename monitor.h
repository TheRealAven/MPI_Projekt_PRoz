#ifndef __MESSAGING_HEADER
#define __MESSAGING_HEADER

#include "mpi.h"
#include "common.h"
#include "list.h"

#define MPI_CLOCK_TYPE MPI_UNSIGNED_LONG_LONG

typedef unsigned long long scalar_clock_t;

typedef struct packet {
	scalar_clock_t clock;
	int sender;
	message data;
} packet;

typedef struct semaphore {
	int k;
	list awaiting;
	int locked;
	int confirmed;

} semaphore;


void monitor_init(int* argc, char*** argv);
void initialize_semaphores(int num, int* k);

void lock_semaphore(int sem_id);
void unlock_semaphore(int sem_id);

void monitor_synchronize(void);

void monitor_cleanup(void);

#endif // __MESSAGING_HEADER
