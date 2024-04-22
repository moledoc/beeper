#pragma once

#ifndef BEEP_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
	double timer;
	char *msg;
	size_t msg_len;
	size_t msg_pxls;
	bool repeat;
} Beep;

char *beep_string(Beep bp);

// TODO: allocate str size smarter
char *beep_string(Beep bp) {
	char *str = calloc((1024+1), sizeof(char));
	if (str == NULL) {
		return NULL;
	}
	snprintf(str, 1024, "{.timer=%0.1f, .repeat=%d, .msg_len=%lu, .msg_pxls=%lu, .msg=%s}", bp.timer, bp.repeat, (unsigned long)bp.msg_len, (unsigned long)bp.msg_pxls, bp.msg);
	return str;
}
	

#endif // BEEP_H_
