#include "monitor.h"

#include <stdlib.h>
#include <stddef.h>

// Todo remove this include after debug
#include <stdio.h>


static scalar_clock_t process_clock;
static int process_rank;
static int processes_num;

static MPI_Datatype packet_type;
static MPI_Datatype message_type;

static semaphore* sems;
static int sems_no;


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

	sems = NULL;
	sems_no = 0;

	process_clock = 0;
}

static void clean_semaphores() {

	int i;
	for (i = 0; i < sems_no; ++i) {
		semaphore* sem = &sems[i];

		clear_list(&sem->awaiting);
	}

	free(sems);
}

void initialize_semaphores(int num, int* k) {

	if (sems != NULL)
		clean_semaphores();

	sems = malloc(sizeof(semaphore) * num);
	sems_no = num;

	int i;
	for (i = 0; i < num; ++i) {

		sems[i].k = k[i];
		sems[i].awaiting = empty_list();
		sems[i].locked = 0;
	}
}

static message receive_message(void) {

	MPI_Status status;
	packet pckt;

	MPI_Recv(&pckt, 1, packet_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	
	if (pckt.clock > process_clock)
		process_clock = pckt.clock;

	process_clock = process_clock + 1;

	return pckt.data;
}

static void send_message(int receiver, message msg) {

	process_clock = process_clock + 1;

	MPI_Status status;
	packet pckt;

	pckt.clock = process_clock;
	pckt.sender = process_rank;
	pckt.data = msg;

	MPI_Send(&pckt, 1, packet_type, receiver, MSG_DEFAULT, MPI_COMM_WORLD);
}

static void broadcast_lock(int sem_id) {

	// Todo
}

void lock_semaphore(int sem_id) {

	process_clock = process_clock + 1;
	sems[sem_id].locked = process_clock;

	message msg;
	msg.message_type = MSG_LOCK;
	msg.content[0] = sem_id;
	msg.content[1] = process_clock;
}

static void allow_awaiting(int sem_id) {

	list_element* it = sems[sem_id].awaiting.head;

	while (it != NULL) {

		message msg;
		msg.message_type = MSG_ALLOW;
		msg.content[0] = sem_id;
		msg.content[1] = it->lock_clock;

		send_message(it->process_rank, msg);
	}
}

void unlock_semaphore(int sem_id) {

	sems[sem_id].locked = 0;
	allow_awaiting(sem_id);
}

void monitor_synchronize(void) {

	// Todo
}

void monitor_cleanup(void) {
	MPI_Finalize();

	if (sems != NULL)
		clean_semaphores();
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

