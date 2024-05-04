#pragma once

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
	bool shutdown;
	pthread_mutex_t mutex;
} Context;

Context *init_context();
void free_context(Context *ctx);
bool should_continue(Context *ctx, bool shutdown);

Context *init_context() {
	Context *ctx= calloc(1, sizeof(Context));
	ctx->shutdown = false;
	pthread_mutex_init(&ctx->mutex, NULL);
	return ctx;
}

void free_context(Context *ctx) {
	free(ctx);
}

bool should_continue(Context *ctx, bool shutdown) {
	pthread_mutex_lock(&ctx->mutex);
	if (shutdown && !ctx->shutdown) {
		ctx->shutdown = shutdown;
		fprintf(stderr, "shutting down...\n");
	}
	pthread_mutex_unlock(&ctx->mutex);
	return ctx->shutdown;
}

#endif // CONTEXT_H_