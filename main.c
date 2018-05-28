#include <stdio.h>

#include "monitor.h"
#include "common.h"

#define CARRIERS_NUM 10

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

int main(int argc, char** argv) {

	monitor_init(&argc, &argv);
	init_semaphores();

	// Todo simulation

	monitor_cleanup();

	return 0;
}
