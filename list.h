#ifndef __LIST_HEADER
#define __LIST_HEADER


typedef unsigned long long scalar_clock_t;

typedef struct list_element {
	int process_rank;
	scalar_clock_t lock_clock;
	struct list_element* next;
} list_element;

typedef struct list {
	list_element* head;
	list_element** tail;
} list;


list empty_list(void);

void add(list* lst, int process_rank, scalar_clock_t lock_clock);

void clear_list(list* lst);

#endif // __LIST_HEADER
