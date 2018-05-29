#include "list.h"

#include <stdlib.h>

list* empty_list(void) {
	list* lst = malloc(sizeof(list));

	lst->head = NULL;
	lst->tail = &lst->head;

	return lst;
}

void list_append(list* lst, int process_rank, scalar_clock_t lock_clock) {

	(*lst->tail) = malloc(sizeof(list_element));

	(*lst->tail)->process_rank = process_rank;
	(*lst->tail)->lock_clock = lock_clock;
	(*lst->tail)->next = NULL;

	lst->tail = &((*lst->tail)->next);
}

void clear_list(list* lst) {
	list_element* element = lst->head;
	
	while (element != NULL) {

		list_element* next = element->next;

		free(element);
		element = next;
	}

	lst->head = NULL;
	lst->tail = &(lst->head);
}

