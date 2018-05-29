#include <stdio.h>
#include <time.h>

#include "monitor.h"
#include "common.h"


#define CARRIERS_NUM 10

#define IDLE_FROM_MILLIS 1000
#define IDLE_TO_MILLIS 10000

#define PROBE_PERIOD_MILLIS 10
#define SLEEP_MARGIN_MILLIS 1


typedef long long millis_t;


static int plane_no;

void init_semaphores(void) {

	int parking_spaces[CARRIERS_NUM] = {3, 5, 1, 6, 2, 3, 2, 4, 7, 8};
	int k[CARRIERS_NUM * 2];

	int i;
	for (i = 0; i < CARRIERS_NUM * 2; ++i) {

		if (i < CARRIERS_NUM)
			k[i] = parking_spaces[i];
		else
			k[i] = 1;
	}

	initialize_semaphores(CARRIERS_NUM * 2, k);
}

millis_t get_millis(void) {

	clock_t clk = clock();
	double sec = ((double) clk) / CLOCKS_PER_SEC;

	return (millis_t) (sec / 1000);
}

millis_t random_millis(void) {
	return IDLE_FROM_MILLIS + rand() % (RANDOM_TO_MILLIS - RANDOM_FROM_MILLIS);	
}

int choose_carrier(void) {
	return rand() % CARRIERS_NUM;
}

int runway_no(int carrier_no) {
	return carrier_no + CARRIERS_NUM;
}

void idle_state(millis_t idle_time) {

	while (idle_time > 0) {

		millis_t sleep_time = PROBE_PERIOD_MILLIS;

		if (sleep_time > idle_time)
			sleep_time = idle_time + SLEEP_MARGIN_MILLIS;

		idle_time -= sleep_time;

		usleep(sleep_time * 1000);
		monitor_synchronize();
	}
}

void plane_flying(void) {

	printf("Plane %d is now flying...\n", plane_no);

	idle_state(random_millis() * 4);

	printf("Plane %d is going to land now.\n", plane_no);
}

int plane_landing(void) {

	int carrier_no = choose_carrier();

	printf("Plane %d is trying to land on carrier %d.\n", plane_no, carrier_no);

	lock_semaphore(carrier_no); // lock place
	lock_semaphore(runway_no(carrier_no)); // lock runway

	idle_state(random_millis() / 5);

	unlock_semaphore(runway_no(carrier_no)); // unlock runway

	return carrier_no;
}

void plane_standing(void) {

	idle_state(random_millis());
}

void plane_starting(int carrier_no) {

	lock_semaphore(runway_no(carrier_no)); // lock_runway 

	idle_state(random_millis() / 2);

	unlock_semaphore(carrier_no + CARRIERS_NUM); // unlock runway
	unlock_semaphore(carrier_no); // unlock place
}

void simulation(void) {

	while (1) {

		plane_flying();

		int carrier_no = plane_landing();

		plane_standing();
		plane_starting(carrier_no);
	}
}

void signal_handler(int signo) {
	monitor_cleanup();
}

void init_environment() {

	init_semaphores();

	srand(time(0));

	MPI_Comm_size(MPI_COMM_WORLD, &plane_no);
	plane_no = plane_no + 1;

	signal(SIGINT, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGSEGV, signal_handler);
}

int main(int argc, char** argv) {

	monitor_init(&argc, &argv);
	init_environment();

	simulation():

	monitor_cleanup();

	return 0;
}
