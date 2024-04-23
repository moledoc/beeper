#pragma once

#ifndef BEEP_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
	double timer;
	char *msg;
	size_t msg_len;
} Beep;

char *beep_string(Beep bp);

// TODO: allocate str size smarter
char *beep_string(Beep bp) {
	char *str = calloc((1024+1), sizeof(char));
	if (str == NULL) {
		return NULL;
	}
	snprintf(str, 1024, "{.timer=%0.1f, .msg_len=%lu, .msg=%s}", bp.timer, (unsigned long)bp.msg_len, bp.msg);
	return str;
}
	

#endif // BEEP_H_
