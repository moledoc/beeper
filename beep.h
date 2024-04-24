#pragma once

#ifndef BEEP_H_
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	double timer;
	char *msg;
	size_t msg_len;
} Beep;

typedef struct Bueue {
	struct Bueue *next;
	Beep bp;
} Bueue;

typedef struct {
	Bueue *bueue;
} Broot;

char *bp_string(Beep bp);

Bueue *bpq_push(Bueue *bpq, Beep new_beep);
Bueue *bpq_pop(Bueue *bpq, Beep *beep);
void bpq_print(Bueue *bpq);
void bpq_free(Bueue *bpq);

// TODO: allocate str size smarter
char *bp_string(Beep bp) {
	char *str = calloc((1024+1), sizeof(char));
	if (str == NULL) {
		return NULL;
	}
	snprintf(str, 1024, "{.timer=%0.1f, .msg_len=%lu, .msg=%s}", bp.timer, (unsigned long)bp.msg_len, bp.msg);
	return str;
}

Bueue *bpq_push(Bueue *bpq, Beep new_bp) {
	Bueue *nbpq = calloc(1, sizeof(Bueue));
	nbpq->bp = new_bp;
	if (bpq == NULL) {
		return nbpq;
	}
	Bueue *cur = bpq;
	for (;cur->next != NULL; cur = cur->next) {
		;
	}
	cur->next = nbpq;
	return bpq;
}

Bueue *bpq_pop(Bueue *bpq, Beep *bp) {
	if (bpq == NULL) {
		return NULL;
	}
	Bueue *nbpq = bpq->next;
	(*bp) = bpq->bp;
	free(bpq);
	return nbpq;
}

void bpq_print(Bueue *bpq) {
	if (bpq == NULL) {
		printf("{NULL}\n");
		return;
	}
		
	for (; bpq != NULL; bpq = bpq->next) {
		char *bp_str = bp_string(bpq->bp);
		printf("{.next=%p, .bp=%s} ->\n", (void *)bpq->next, bp_str);
		if (bp_str != NULL) {
			free(bp_str);
		}
	}
	printf("{NULL}\n");
}

void bpq_free(Bueue *bpq) {
	if (bpq == NULL) {
		return;
	}
	for (; bpq != NULL;) {
		Bueue *tmp = bpq->next;
		free(bpq);
		bpq = tmp;
	}
}

#endif // BEEP_H_
