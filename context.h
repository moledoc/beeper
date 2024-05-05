#pragma once

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#include "raylib.h"
#include "queue.h"
#include "protocol.h"

typedef struct {
	pthread_mutex_t mutex;
	bool on;
	Queue *queue;
} Context;

Context *init_context();
void free_context(Context *ctx);
bool should_continue(Context *ctx, bool shutdown);

// TODO: not sure this is the way to go
void store_packet(Context *ctx, Packet packet);
void stored_packets_log(Context *ctx);
Packet *stored_packets_head(Context *ctx);
Packet *pop_packet(Context *ctx); // allocs memory

Context *init_context() {
	Context *ctx= calloc(1, sizeof(Context));
	ctx->on = false;
	ctx->queue = NULL;
	pthread_mutex_init(&ctx->mutex, NULL);
	return ctx;
}

void free_context(Context *ctx) {
	queue_free(ctx->queue);
	free(ctx);
}

bool is_on(Context *ctx, bool on) {
	pthread_mutex_lock(&ctx->mutex);
	if (on && !ctx->on) {
		ctx->on = on;
		TraceLog(LOG_INFO, "shutting down...");
	}
	pthread_mutex_unlock(&ctx->mutex);
	return ctx->on;
}

void store_packet(Context *ctx, Packet packet) {
	pthread_mutex_lock(&ctx->mutex);
	ctx->queue = queue_push(ctx->queue, packet);
	pthread_mutex_unlock(&ctx->mutex);
}

void stored_packets_log(Context *ctx) {
	pthread_mutex_lock(&ctx->mutex);
	queue_log("stored packets:", ctx->queue);
	pthread_mutex_unlock(&ctx->mutex);
}

Packet *stored_packets_head(Context *ctx) {
	pthread_mutex_lock(&ctx->mutex);
	Packet *packet = queue_head(ctx->queue);
	pthread_mutex_unlock(&ctx->mutex);
	return packet;
}

Packet *pop_packet(Context *ctx) {
	pthread_mutex_lock(&ctx->mutex);
	Packet *packet = calloc(1, sizeof(Packet));
	ctx->queue = queue_pop(ctx->queue, packet);
	pthread_mutex_unlock(&ctx->mutex);
	return packet;
}

#endif // CONTEXT_H_