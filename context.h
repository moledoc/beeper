#pragma once

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#include "raylib.h"

typedef struct {
	pthread_mutex_t mutex;
	bool on;
} Context;

Context *init_context();
void free_context(Context *ctx);
bool should_continue(Context *ctx, bool shutdown);

Context *init_context() {
	Context *ctx= calloc(1, sizeof(Context));
	ctx->on = false;
	pthread_mutex_init(&ctx->mutex, NULL);
	return ctx;
}

void free_context(Context *ctx) {
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

#endif // CONTEXT_H_