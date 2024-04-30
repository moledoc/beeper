#pragma once

#ifndef BEEP_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// TODO: naming

typedef struct {
	uint32_t id;
	double timer;
	bool repeat;
	uint8_t msg_len;
	char *msg;
} Beep;

typedef struct Bueue {
	struct Bueue *next;
	Beep *bp;
} Bueue;

typedef struct {
	pthread_mutex_t mutex;
	Bueue *bueue;
} Broot;

char *bp_string(Beep *bp);
void bp_print(char *desc, Beep *bp, char *filepath, int linenr);


Broot *binit();
void bpush(Broot *br, Beep *bp);
Beep *bpop(Broot *br);
void bprint(Broot *br);
void bfree(Broot *br);

// TODO: allocate str size smarter
char *bp_string(Beep *bp) {
	if (!bp) {
		return NULL;
	}
	char *str = calloc((1024+1), sizeof(char));
	if (str == NULL) {
		return NULL;
	}
	snprintf(str, 1024, "%p -- {.id=%u, .timer=%0.1f, .repeat=%d, .msg_len=%lu, .msg=%s}", (void *)bp, bp->id, bp->timer, bp->repeat, (unsigned long)bp->msg_len, bp->msg);
	return str;
}

void bp_print(char *desc, Beep *bp, char *filepath, int linenr) {
	if (strlen(desc) != 0) {
		printf("%s: ", desc);
	}
	char *tmp = bp_string(bp);
	printf("%s:%d %s\n", filepath, linenr, tmp);
	free(tmp);
}

// TODO: handle br==NULL

Broot *binit() {
	Broot *tmp = malloc(sizeof(Broot));
	tmp->bueue = NULL;
	pthread_mutex_init(&tmp->mutex, NULL);
	return tmp;
}

void bpush(Broot *br, Beep *bp) {
	pthread_mutex_lock(&(br->mutex));
	Bueue *nbq = calloc(1, sizeof(Bueue));
	nbq->bp = bp;
	nbq->next = NULL;

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
	bp_print("bpush beep", nbq->bp, __FILE__, __LINE__);
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
	bp_print("bpop beep", bp, __FILE__, __LINE__);
	pthread_mutex_unlock(&(br->mutex));
	return bp;
}

void bprint(Broot *br) {
	pthread_mutex_lock(&(br->mutex));
	printf("queue: ");
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
