#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "monitor.h"
#include "common.h"
#include "termutils.h"

#define IDLE_FROM_MILLIS 1000
#define IDLE_TO_MILLIS 10000

#define PROBE_PERIOD_MILLIS 10
#define SLEEP_MARGIN_MILLIS 1


typedef long long millis_t;

// #define CARRIERS_NUM 10
#define CARRIERS_NUM 1

// static int PARKING_SPACES[CARRIERS_NUM] = {3, 5, 1, 6, 2, 3, 2, 4, 7, 8};
static int PARKING_SPACES[CARRIERS_NUM] = {3};


static int plane_no;


millis_t get_millis(void) {

	clock_t clk = clock();
	double sec = ((double) clk) / CLOCKS_PER_SEC;

	return (millis_t) (sec / 1000);
}

millis_t random_millis(void) {
	return IDLE_FROM_MILLIS + rand() % (IDLE_TO_MILLIS - IDLE_FROM_MILLIS);	
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

	idle_state(random_millis() * 2);
}

int plane_landing(void) {

	int carrier_no = choose_carrier();

	printf("Plane %d is trying to land on carrier %d.\n", plane_no, carrier_no);

	lock_semaphore(carrier_no); // lock place
	lock_semaphore(runway_no(carrier_no)); // lock runway

	printf(ANSI_COLOR_RED "Plane " ANSI_COLOR_BLUE "%d" ANSI_COLOR_RED "(at " ANSI_COLOR_MAGENTA "%d" ANSI_COLOR_RED ") is landing..." ANSI_COLOR_RESET "\n", plane_no, carrier_no);
	idle_state(random_millis() / 5);

	unlock_semaphore(runway_no(carrier_no)); // unlock runway

	printf("Plane %d landed on carrier no %d.\n", plane_no, carrier_no);

	return carrier_no;
}

void plane_standing(int carrier_no) {

	printf("Plane %d is now standing on the carrier no %d.\n", plane_no, carrier_no);
	idle_state(random_millis());
}

void plane_starting(int carrier_no) {

	printf("Plane %d (at %d) is going to set off...\n", plane_no, carrier_no);

	lock_semaphore(runway_no(carrier_no)); // lock_runway 

	printf("Plane %d (at %d) is accelerating and rising into the air...\n", plane_no, carrier_no);

	idle_state(random_millis() / 2);

	unlock_semaphore(runway_no(carrier_no)); // unlock runway
	unlock_semaphore(carrier_no); // unlock place

	printf(ANSI_COLOR_GREEN "Plane " ANSI_COLOR_BLUE "%d" ANSI_COLOR_GREEN " left carrier no " ANSI_COLOR_MAGENTA "%d" ANSI_COLOR_RESET ".\n", plane_no, carrier_no);
}

void simulation(void) {

	while (1) {

		plane_flying();

		int carrier_no = plane_landing();

		plane_standing(carrier_no);
		plane_starting(carrier_no);
	}
}

void signal_handler(int signo) {
	monitor_cleanup();
}

void init_semaphores(void) {

	int k[CARRIERS_NUM * 2];

	int i;
	for (i = 0; i < CARRIERS_NUM; ++i) {
		k[i] = PARKING_SPACES[i];
		k[runway_no(i)] = 1;
	}

	initialize_semaphores(CARRIERS_NUM * 2, k);
}

void init_environment() {

	init_semaphores();

	MPI_Comm_rank(MPI_COMM_WORLD, &plane_no);
	plane_no = plane_no + 1;

	srand(time(0) * plane_no);

	signal(SIGINT, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGSEGV, signal_handler);
}

int main(int argc, char** argv) {

	monitor_init(&argc, &argv);
	init_environment();

	simulation();

	monitor_cleanup();

	return 0;
}
