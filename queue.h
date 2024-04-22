#pragma once

#ifndef QUEUE_H_
#include <stdlib.h>
#include "beep.h"

typedef struct Queue {
	struct Queue *next;
	Beep bp;
} Queue;

Queue *q_push(Queue *queue, Beep new_beep);
Queue *q_pop(Queue *queue);
void q_print(Queue *queue);
void q_free(Queue *queue);

Queue *q_push(Queue *q, Beep new_bp) {
	Queue *nq = calloc(1, sizeof(Queue));
	nq->bp = new_bp;
	if (q == NULL) {
		return nq;
	}
	Queue *cur = q;
	for (;cur->next != NULL; cur = cur->next) {
		;
	}
	cur->next = nq;
	return q;
}

Queue *q_pop(Queue *q) {
	if (q == NULL) {
		return NULL;
	}
	Queue *nq = q->next;
	free(q);
	return nq;
}

void q_print(Queue *q) {
	if (q == NULL) {
		printf("{NULL}\n");
		return;
	}
		
	for (; q != NULL; q = q->next) {
		char *bp_str = beep_string(q->bp);
		printf("{.next=%p, .bp=%s} ->\n", (void *)q->next, bp_str);
		if (bp_str != NULL) {
			free(bp_str);
		}
	}
	printf("{NULL}\n");
}

void q_free(Queue *q) {
	if (q == NULL) {
		return;
	}
	for (; q != NULL;) {
		Queue *tmp = q->next;
		free(q);
		q = tmp;
	}
}

#endif // QUEUE_H_