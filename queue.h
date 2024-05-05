#pragma once

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdlib.h>

#include "raylib.h"
#include "protocol.h"

size_t slen(const char *s) {
	char *s_cpy = (char *)s;
	size_t size = 0;
	for (;(*s_cpy++) != '\0'; ++size) {
		;
	}
	return size;
}

typedef struct Queue {
	struct Queue *next;
	Packet *packet;
} Queue;

Queue *queue_push(Queue *queue, Packet packet); // allocs memory
Queue *queue_pop(Queue *queue, Packet *packet);
Packet *queue_head(Queue *queue);
void queue_log(char *format, Queue *queue);
void queue_free(Queue *queue);

Queue *queue_push(Queue *queue, Packet packet) {
	Queue *next = calloc(1, sizeof(Queue));
	next->next = NULL;
	next->packet = calloc(1, sizeof(Packet));
	next->packet->msg = calloc(MSG_SIZE, sizeof(unsigned char));
	next->packet->version_minor = packet.version_minor;
	next->packet->version_major = packet.version_major;
	next->packet->repeat = packet.repeat;
	next->packet->timer = packet.timer;
	mmcp(next->packet->msg, packet.msg, PACKET_SIZE);

	if (queue == NULL) {
		packet_log("pushing packet to head of queue: %s", next->packet);
		return next;
	}
	Queue *cur = queue;
	for (;cur->next != NULL; cur = cur->next) {
		;
	}
	cur->next = next;
	packet_log("pushing packet to tail of queue: %s", next->packet);
	return queue;
}

Queue *queue_pop(Queue *queue, Packet *packet) {
	if (queue == NULL) {
		return NULL;
	}
	Queue *next = queue->next;
	packet->msg = calloc(MSG_SIZE, sizeof(unsigned char));
	packet->version_minor = queue->packet->version_minor;
	packet->version_major = queue->packet->version_major;
	packet->repeat = queue->packet->repeat;
	packet->timer = queue->packet->timer;
	mmcp(packet->msg, queue->packet->msg, PACKET_SIZE);
	packet_free(queue->packet);
	free(queue);
	packet_log("popping packet from queue: %s", packet);
	return next;
}


Packet *queue_head(Queue *queue) {
	if (queue == NULL) {
		return NULL;
	}
	return queue->packet;
}


void queue_log(char *format, Queue *queue) {
	TraceLog(LOG_INFO, format);
	for (;queue != NULL; queue = queue->next) {
		char *str_packet = packet_string(*(queue->packet));
		fprintf(stderr, "%s -> ", str_packet);
		free(str_packet);
	}
	fprintf(stderr, "{NULL}\n");
}

void queue_free(Queue *queue) {
	for (;queue != NULL;) {
		Queue *me = queue;
		queue = queue->next;
		packet_free(me->packet);
		free(me);
	}
}


#endif // QUEUE_H_