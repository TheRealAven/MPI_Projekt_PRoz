#include "mpi.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#define SIZE_OF_MSG 3
#define MAX_WAIT 500
#define ALL_CARRIERS 10
#define MSG_IS_FREE 1001
#define MSG_IS_LANDING_FREE 1002
#define MSG_FREE_PLACE 1003
#define MSG_WHERE_ARE_ALL 1004

MPI_Status *waitingMessages;
int numberOfWaitingMessages, isLanding;

void notifyWaitingPlanes(){
	int msg[SIZE_OF_MSG];
	MPI_Status tempStatus;
	for(int i=0;i<numberOfWaitingMessages;i++){
		tempStatus=waitingMessages[i];
		MPI_Send(msg, SIZE_OF_MSG, MPI_INT, tempStatus.MPI_SOURCE, MSG_IS_FREE, MPI_COMM_WORLD);
		//waitingMessages[i]=NULL;
	}
	numberOfWaitingMessages=0;
}

void resolveConflict(){

}

void handleMessage(MPI_Status *msg_status, int shipIWant, int shipNumber){
	int msg[SIZE_OF_MSG];
	MPI_Status status;
	switch(msg_status->MPI_TAG){
		case MSG_WHERE_ARE_ALL:
			MPI_Recv(msg, SIZE_OF_MSG, MPI_INT, MPI_ANY_SOURCE, MSG_WHERE_ARE_ALL, MPI_COMM_WORLD, &status);
			if(msg[0]!=shipNumber){
				MPI_Send(msg, SIZE_OF_MSG, MPI_INT, status.MPI_SOURCE, MSG_FREE_PLACE, MPI_COMM_WORLD);	
			}else if(msg[0]==shipIWant){
				resolveConflict();
			}
			break;
		case MSG_IS_LANDING_FREE:
			MPI_Recv(msg, SIZE_OF_MSG, MPI_INT, MPI_ANY_SOURCE, MSG_IS_LANDING_FREE, MPI_COMM_WORLD, &status);
			if((msg[0]==shipNumber)&&(isLanding)){
				waitingMessages[numberOfWaitingMessages]=status;
				numberOfWaitingMessages++;
			}else{
				MPI_Send(msg, SIZE_OF_MSG, MPI_INT, status.MPI_SOURCE, MSG_IS_FREE, MPI_COMM_WORLD);
			}
			
			break;
		default:
			
			break;
	}
}

void startLaneReserving(int whichShip, int planeNumber, int allPlanes){
	int msg[SIZE_OF_MSG], planesLeft, receiver, flag;
	isLanding=1;
	MPI_Status msg_status;
	msg[0]=whichShip;
	msg[1]=planeNumber;
	for(int j=0;j<allPlanes;j++){
		if(j!=planeNumber){
			receiver=j;
			MPI_Send(msg, SIZE_OF_MSG, MPI_INT, receiver, MSG_IS_LANDING_FREE, MPI_COMM_WORLD);
		}
	}
	printf("%d oczekuje na pas %d \n", planeNumber, whichShip);
	planesLeft=allPlanes;
	while(planesLeft>0){
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &msg_status);
		//obsluga
		if(flag){
			if(msg_status.MPI_TAG==MSG_IS_FREE){
				MPI_Recv(msg, SIZE_OF_MSG, MPI_INT, MPI_ANY_SOURCE, MSG_IS_FREE, MPI_COMM_WORLD, &msg_status);
				planesLeft--;
			}else{
				handleMessage(&msg_status, -1, whichShip);
			}
		}
	}
	printf("%d uzywa pasa na %d \n", planeNumber, whichShip);
	isLanding=0;
}

int main( int argc, char **argv )
{
	int planeNumber, allPlanes;
	int carrierCapacity[ALL_CARRIERS];
	int shipIWant=-1, shipNumber=-1, msg[SIZE_OF_MSG], receiver, itLeft, canGo, flag;

	srand(time(NULL));
	MPI_Init( &argc, &argv );
	
	printf("init done\n");

	MPI_Comm_rank( MPI_COMM_WORLD, &planeNumber );
	MPI_Comm_size( MPI_COMM_WORLD, &allPlanes );
	
	printf("comm_world done\n");

	waitingMessages = malloc(allPlanes*sizeof(MPI_Status));

	printf("malloc done\n");

	for(int i=0;i<allPlanes-5;i++){
		carrierCapacity[i%ALL_CARRIERS]++;
	}

	printf("carrierCapacity done\n");

	MPI_Status msg_status;
	while(1){
		itLeft = rand() % MAX_WAIT;
		//Wait loop 1
		while(itLeft>0){
printf("test2\n");
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &msg_status);
			//obsluga
printf("test\n");
			if(flag){
				handleMessage(&msg_status, shipIWant, shipNumber);
			}
			itLeft--;
		}
		//Ship choosing loop
		shipIWant=rand()%ALL_CARRIERS;
		printf("%d wybral statek: %d \n", planeNumber, shipIWant);
		msg[0]=shipIWant;
		for(int j=0;j<allPlanes;j++){
			if(j!=planeNumber){
				receiver=j;
				MPI_Send(msg, SIZE_OF_MSG, MPI_INT, receiver, MSG_WHERE_ARE_ALL, MPI_COMM_WORLD);
			}		
		}
		canGo=0;
		while(canGo<allPlanes-carrierCapacity[shipIWant]){
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &msg_status);
			//obsluga
			if(flag){
				if(msg_status.MPI_TAG==MSG_FREE_PLACE){
					MPI_Recv(msg, SIZE_OF_MSG, MPI_INT, MPI_ANY_SOURCE, MSG_FREE_PLACE, MPI_COMM_WORLD, &msg_status);
					canGo++;
				}else{
					handleMessage(&msg_status, shipIWant, shipNumber);
				}
			}
		}
		printf("%d znalazl miejsce na %d \n", planeNumber, shipNumber);
		//Landing loop do ponizszej funkcji
		
		shipNumber=shipIWant;
		shipIWant=-1;
		startLaneReserving(shipNumber, planeNumber, allPlanes);
		printf("%d wyladowal na %d \n", planeNumber, shipNumber);
		//Landing loop
		/*msg[0]=shipNumber;
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
				handleMessage(&msg_status, shipIWant, shipNumber);
			}
		}*/
		notifyWaitingPlanes();
		itLeft = rand() % MAX_WAIT;
		while(itLeft>0){
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &msg_status);
			//obsluga
			if(flag){
				handleMessage(&msg_status, shipIWant, shipNumber);
			}
			itLeft--;
		}
		//Landing loop again
		startLaneReserving(shipNumber, planeNumber, allPlanes);
		printf("%d wystartowal z %d \n", planeNumber, shipNumber);
		shipNumber=-1;
	}

	MPI_Finalize();
}
