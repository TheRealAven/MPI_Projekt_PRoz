#include <stdio.h>

#include "monitor.h"
#include "common.h"


int main(int argc, char** argv) {

	monitor_init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {

		message msg;

		msg.message_type = -1;
		msg.content[0] = 1;
		msg.content[1] = 0;

		printf("%d is sending message:\n\ttype = %d\n\tcontent = (%d, %d)\n", rank, msg.message_type, msg.content[0], msg.content[1]);
		send_message(msg);

		message answer = receive_message();

		printf("%d received answer:\n\ttype = %d, content(%d, %d)\n", rank, answer.message_type, answer.content[0], answer.content[1]);

	} else {

		message msg = receive_message();

		if (rank == 5) {

			msg.message_type = -2;
			
			int temp = msg.content[0];
			msg.content[0] = msg.content[1];
			msg.content[1] = temp;
		}

		send_message(msg);
	}

	monitor_cleanup();

	return 0;
}
