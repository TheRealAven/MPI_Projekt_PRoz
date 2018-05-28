#include "monitor.h"

#include <stddef.h>

// Todo remove this include after debug
#include <stdio.h>

#define MSG_DEFAULT 1

static scalar_clock_t process_clock;
static int process_rank;
static int processes_num;

static MPI_Datatype packet_type;
static MPI_Datatype message_type;

static void init_message_structure(void);
static void init_packet_structure(void);

static void init_structures(void) {

	init_message_structure();
	init_packet_structure();
}

void monitor_init(int* argc, char*** argv) {

	MPI_Init(argc, argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &processes_num);

	init_structures();

	process_clock = 0;
}

void init_semaphores(int num, int* k) {
}

// Todo change this method to static
// Interface depreciated
message receive_message(void) {

	MPI_Status status;
	packet pckt;

	MPI_Recv(&pckt, 1, packet_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	
	if (pckt.clock > process_clock)
		process_clock = pckt.clock;

	process_clock = process_clock + 1;

	return pckt.data;
}

void send_message(message msg) {

	process_clock = process_clock + 1;

	MPI_Status status;
	packet pckt;

	pckt.clock = process_clock;
	pckt.sender = process_rank;
	pckt.data = msg;

	int receiver = (process_rank + 1) % processes_num;

	MPI_Send(&pckt, 1, packet_type, receiver, MSG_DEFAULT, MPI_COMM_WORLD);
}

void lock_semaphore(int sem_id) {
}

void unlock_semaphore(int sem_id) {
}

void monitor_cleanup(void) {
	MPI_Finalize();
}

static void init_message_structure(void) {

	int fields_no = 2;
	int block_lengths[2] = {1, SIZE_OF_MSG};
	MPI_Datatype types[2] = {MPI_INT, MPI_INT};
	MPI_Aint offsets[2];

	offsets[0] = offsetof(message, message_type);
	offsets[1] = offsetof(message, content);

	MPI_Type_create_struct(fields_no, block_lengths, offsets, types, &message_type);
	MPI_Type_commit(&message_type);
}

static void init_packet_structure(void) {

	int fields_no = 3;
	int block_lengths[3] = {1, 1, 1};
	MPI_Datatype types[3] = {MPI_CLOCK_TYPE, MPI_INT, message_type};
	MPI_Aint offsets[3];

	offsets[0] = offsetof(packet, clock);
	offsets[1] = offsetof(packet, sender);
	offsets[2] = offsetof(packet, data);

	MPI_Type_create_struct(fields_no, block_lengths, offsets, types, &packet_type);
	MPI_Type_commit(&packet_type);
}

