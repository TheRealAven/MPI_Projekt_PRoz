#include "monitor.h"

#include <stdlib.h>
#include <stddef.h>

// #define DEBUG

#ifdef DEBUG

#include <stdio.h>

#endif // DEBUG

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

	sems = NULL;
	sems_no = 0;

	process_clock = 1;

	MPI_Init(argc, argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &processes_num);

	init_structures();
}

static void clean_semaphores() {

	int i;
	for (i = 0; i < sems_no; ++i) {
		semaphore* sem = &sems[i];

		clear_list(sem->awaiting);
		free(sem->awaiting);
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
		sems[i].confirmed = 0;
	}
}

static packet receive_packet(void) {

	MPI_Status status;
	packet pckt;

	MPI_Recv(&pckt, 1, packet_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	
	if (pckt.clock > process_clock)
		process_clock = pckt.clock;

	process_clock = process_clock + 1;

	return pckt;
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

static void broadcast_lock(int sem_id, int lock_clock) {

	message msg;
	msg.message_type = MSG_LOCK;
	msg.content[0] = sem_id;
	msg.content[1] = lock_clock;

	int i;
	for (i = 0; i < processes_num; ++i) {

		if (i != process_rank)
			send_message(i, msg);
	}
}

static void allow_enter(int target, int sem_id, scalar_clock_t lock_clock) {

	message msg;
	msg.message_type = MSG_ALLOW;
	msg.content[0] = sem_id;
	msg.content[1] = lock_clock;

	send_message(target, msg);
}

static int is_request_prior(int req_sem_id, scalar_clock_t req_lock_clock, int sender) {

	if (sems[req_sem_id].locked == 0)
		return 1;

	if (sems[req_sem_id].confirmed == 1)
		return 0;

	if (sems[req_sem_id].locked < req_lock_clock)
		return 0;

	if (sems[req_sem_id].locked == req_lock_clock && process_rank < sender)
		return 0;

	return 1;
}

static void handle_request(message msg, int sender) {

	if (msg.message_type != MSG_LOCK)
		return;

	int req_sem_id = msg.content[0];
	scalar_clock_t req_lock_clock = msg.content[1];

	if (is_request_prior(req_sem_id, req_lock_clock, sender)) {

		allow_enter(sender, req_sem_id, req_lock_clock);

#ifdef DEBUG
		printf("Process %d (locked: %d) allowed %d (locked: %d) to enter semaphore\n", process_rank, sems[req_sem_id].locked, sender, req_lock_clock, req_sem_id);
#endif
	}
	else {
		list_append(sems[req_sem_id].awaiting, sender, req_lock_clock);
	}
}

static void wait_for_approval(int locked_sem_id) {
	
	int approvals = 0;

	while (approvals < processes_num - sems[locked_sem_id].k) {

		packet pckt = receive_packet();
		message msg = pckt.data;

		if (msg.message_type == MSG_ALLOW) {

			int sem_id = msg.content[0];
			scalar_clock_t lock_clock = msg.content[1];

			if (sem_id == locked_sem_id && sems[locked_sem_id].locked == lock_clock) {

				approvals = approvals + 1;

#ifdef DEBUG
				printf("Process %d received approval from %d (on semaphore %d) and needs %d more.\n",
						process_rank, pckt.sender, sem_id, processes_num - sems[sem_id].k - approvals);
#endif
			}
		}
		else
			handle_request(msg, pckt.sender);
	}
}

void lock_semaphore(int sem_id) {

	process_clock = process_clock + 1;
	sems[sem_id].locked = process_clock;

	broadcast_lock(sem_id, process_clock);
	wait_for_approval(sem_id);

	sems[sem_id].confirmed = 1;
}

static void allow_awaiting(int sem_id) {

	list_element* it = sems[sem_id].awaiting->head;

	while (it != NULL) {

		allow_enter(it->process_rank, sem_id, it->lock_clock);
		it = it->next;
	}

	clear_list(sems[sem_id].awaiting);
}

void unlock_semaphore(int sem_id) {

	process_clock = process_clock + 1;
	sems[sem_id].locked = 0;
	sems[sem_id].confirmed = 0;
	allow_awaiting(sem_id);
}

void monitor_synchronize(void) {

	MPI_Status status;
	int flag;

	while (1) {
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

		if (flag == 0)
			break;

		packet pckt = receive_packet();
		message msg = pckt.data;
		int sender = pckt.sender;

#ifdef DEBUG		
		printf("Process %d received message from %d during synchronization (message type: %d)\n", process_rank, sender, msg.message_type);
#endif

		handle_request(msg, sender);
	}
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

