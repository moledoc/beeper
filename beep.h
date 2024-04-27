#pragma once

#ifndef BEEP_H_
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
	double timer;
	char *msg;
	size_t msg_len;
} Beep;

typedef struct Bueue {
	struct Bueue *next;
	Beep *bp;
} Bueue;

typedef struct {
	Bueue *bueue;
	pthread_mutex_t mutex;
} Broot;

char *bp_string(Beep *bp);

Broot *binit();
void bpush(Broot *br, Beep *bp);
Beep *bpop(Broot *br);
void bprint(Broot *br);
void bfree(Broot *br);

Bueue *bpq_push(Bueue *bpq, Beep new_beep);
Bueue *bpq_pop(Bueue *bpq, Beep *beep);
void bpq_print(Bueue *bpq);
void bpq_free(Bueue *bpq);

// TODO: allocate str size smarter
char *bp_string(Beep *bp) {
	char *str = calloc((1024+1), sizeof(char));
	if (str == NULL) {
		return NULL;
	}
	snprintf(str, 1024, "{.timer=%0.1f, .msg_len=%lu, .msg=%s}", bp->timer, (unsigned long)bp->msg_len, bp->msg);
	return str;
}

Bueue *bpq_push(Bueue *bpq, Beep new_bp) {
	Bueue *nbpq = calloc(1, sizeof(Bueue));
	nbpq->bp = &new_bp;
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
	(*bp) = (*bpq->bp);
	Bueue *nbpq = bpq->next;
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

// -----------------------

Broot *binit() {
	Broot *tmp = malloc(sizeof(Broot));
	tmp->bueue = NULL;
	pthread_mutex_init(&tmp->mutex, NULL);
	return tmp;
}

void bpush(Broot *br, Beep *bp) {
	pthread_mutex_lock(&(br->mutex));
	Bueue *nbq = malloc(sizeof(Bueue));
	nbq->bp = bp;
	if (br->bueue == NULL) {
		br->bueue = nbq;
		goto exit;
	}
	Bueue *cur = br->bueue;
	for (;cur->next != NULL;cur = cur->next) {
		;
	}
	cur->next = nbq;
exit:
	pthread_mutex_unlock(&(br->mutex));
	return;
}

Beep *bpop(Broot *br) {
	pthread_mutex_lock(&(br->mutex));
	Beep *bp = NULL;
	if (br->bueue == NULL) {
		goto exit;
	}
	bp = br->bueue->bp;
	Bueue *freeme = br->bueue;
	br->bueue = br->bueue->next;
	free(freeme);
exit:
	pthread_mutex_unlock(&(br->mutex));
	return bp;
}

void bprint(Broot *br) {
	pthread_mutex_lock(&(br->mutex));
	Bueue *bpq = br->bueue;
	if (bpq == NULL) {
		goto exit;
	}
		
	for (; bpq != NULL; bpq = bpq->next) {
		char *bp_str = bp_string(bpq->bp);
		printf("{.next=%p, .bp=%s} ->\n", (void *)bpq->next, bp_str);
		if (bp_str != NULL) {
			free(bp_str);
		}
	}
exit:
	printf("{NULL}\n");
	pthread_mutex_unlock(&(br->mutex));
	return;
}

void bfree(Broot *br) {
	pthread_mutex_lock(&(br->mutex));
	Bueue *bpq = br->bueue;
	if (bpq == NULL) {
		goto exit;
	}
	for (; bpq != NULL;) {
		Bueue *tmp = bpq->next;
		free(bpq);
		bpq = tmp;
	}
exit:
	pthread_mutex_unlock(&(br->mutex));
	if (br != NULL) {
		free(br);
	}
	return;
}

#endif // BEEP_H_
