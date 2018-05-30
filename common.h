#ifndef __COMMON_HEADER
#define __COMMON_HEADER


#define MSG_DEFAULT 1
#define MSG_ALLOW 2
#define MSG_LOCK 3
#define MSG_HALT 4

#define SIZE_OF_MSG 2

typedef struct message {
	int message_type;
	int content[SIZE_OF_MSG];
} message;

#endif
