#include "mpi.h"
#include <stdio.h>
#define SIZE_OF_MSG 3
#define MAX_WAIT 500
#define ALL_CARRIERS 10
#define MSG_IS_FREE 1001
#define MSG_IS_LANDING_FREE 1002
#define MSG_FREE_PLACE 1003
#define MSG_WHERE_ARE_ALL 1004

int chooseShip(){

}

void startLaneReserving(int whichShip, int planeNumber, int allPlanes){
	
}

void notifyWaitingPlanes(MPI_Status *msg_status){
	//?? Possible change
}

void handleMessage(MPI_Status *msg_status){
	switch(*msg_status.TAG){
		case MSG_WHERE_ARE_ALL:
			//TODO
			break;
		case MSG_IS_LANDING_FREE:
			//TODO
			break;
		default:
			//TODO
			break;
	}
}

int main( int argc, char **argv )
{
	int planeNumber, allPlanes;
	int carrierCapacity[ALL_CARRIERS];
	int shipIWant, shipNumber, msg[SIZE_OF_MSG], receiver;

	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int namelen;

	MPI_Init( &argc, &argv );
	
	MPI_Comm_rank( MPI_COMM_WORLD, &planeNumber );
	MPI_Comm_size( MPI_COMM_WORLD, &allPlanes );
	MPI_Get_processor_name(processor_name,&namelen);

	for(int i=0;i<allPlanes-5;i++){
		carrierCapacity[i%ALL_CARRIERS]++;
	}

	MPI_Status msg_status;
	while(1){
		itLeft = rand() % MAX_WAIT;
		//Wait loop 1
		while(itLeft>0){
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &msg_status);
			//obsluga
			handleMessage(msg_status);
			itLeft--;
		}
		//Ship choosing loop
		shipIWant=chooseShip();
		msg[0]=shipIWant;
		for(int j=0;j<allPlanes;j++){
			if(j!=planeNumber){
				receiver=j;
				MPI_Send(msg, SIZE_OF_MSG, MPI_INT, receiver, MSG_WHERE_ARE_ALL, MPI_COMM_WORLD);
			}		
		}
		canGo=0;
		while(canGo<allPlanes-carrierCapacity[shipIWant]){
			MMPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &msg_status);
			//obsluga
			if(msg_status.TAG==MSG_FREE_PLACE){
				canGo++;
			}else{
				handleMessage(&msg_status);
			}
		}
		//Landing loop do ponizszej funkcji
		startLaneReserving();
		shipNumber=shipIWant;
		shipIWant=-1;
		//Landing loop
		msg[0]=shipNumber;
		msg[1]=planeNumber;
		for(int j=0;j<allPlanes;j++){
			if(j!=planeNumber){
				receiver=j;
				MPI_Send(msg, SIZE_OF_MSG, MPI_INT, receiver, MSG_IS_LANDING_FREE, MPI_COMM_WORLD);
			}
		}
		planesLeft=allPlanes;
		while(planesLeft>0){
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &msg_status);
			//obsluga
			if(msg_status.TAG==MSG_IS_FREE){
				planesLeft--;
			}else{
				handleMessage(&msg_status);
			}
		}
		//TODO
		notifyWaitingPlanes();
		itLeft = rand() % MAX_WAIT;
		while(ifLeft>0){
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &msg_status);
			//obsluga
			handleMessage(&msg_status);
			itLeft--;
		}
		//Landing loop again
		startLaneReserving();
		shipNumber=-1;
	}

	MPI_Finalize();
}
