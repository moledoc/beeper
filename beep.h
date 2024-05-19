#pragma once

#ifndef BEEP_H_
#define BEEP_H_

typedef struct {
	uint8_t version_major;
	uint8_t version_minor;
	uint8_t label_len;
	unsigned char *label;
	uint8_t msg_len;
	unsigned char *msg;
} Beep;

enum metadata {
	version_minor_idx,
	version_major_idx,
	msg_start_idx,
	metadata_size,
};

void free_beep(Beep *beep);
unsigned char *beep2str(Beep *beep); // allocs memory
void print_beep(Beep *beep);


uint8_t *marshal_beep(Beep *beep); // allocs memory
Beep *unmarshal_beep(uint8_t *buf); // allocs memory

#endif // BEEP_H_

#ifndef BEEP_VARS
#define BEEP_VARS

uint8_t version_major = '0';
uint8_t version_minor = '1';

#endif // BEEP_VARS

#ifndef BEEP_UTILS
#define BEEP_UTILS

int valid_version_buf(uint8_t *buf);
int valid_version_beep(Beep *beep);

int valid_version_buf(uint8_t *buf) {
	return buf != NULL && \
		buf[version_major_idx] == version_major && \
		buf[version_minor_idx] == version_minor;
}


int valid_version_beep(Beep *beep) {
	return beep != NULL && \
		beep->version_major == version_major && \
		beep->version_minor == version_minor;
}

#endif // SOCK_UTILS

#ifndef BEEP_IMPLEMENTATION
#define BEEP_IMPLEMENTATION

void free_buf(uint8_t *buf) {
	if (buf != NULL) {
		free(buf);
	}
}

void free_beep(Beep *beep) {
	if (beep == NULL) {
		return;
	}
	if (beep->label != NULL) {
		free(beep->label);
	}
	if (beep->msg != NULL) {
		free(beep->msg);
	}
	free(beep);
}

unsigned char *beep2str(Beep *beep) { // allocs memory
	if (beep == NULL) {
		return NULL;
	}
	size_t str_len = (128+beep->label_len+beep->msg_len)*sizeof(unsigned char);
	unsigned char *str = malloc(str_len);
	memset(str, '\0', sizeof(str));
	snprintf(str, str_len, "{"
		"\n\t.version_major=%d"
		"\n\t.version_minor=%d"
		"\n\t.label=(%d)'%s'"
		"\n\t.msg=(%d)'%s'"
		"\n}",
		beep->version_major-'0',
		beep->version_minor-'0',
		beep->label_len, beep->label,
		beep->msg_len, beep->msg
	);
	return str;
}

void print_beep(Beep *beep) {
	if (beep == NULL) {
		fprintf(stderr, "{NULL}\n");
		return;
	}
	unsigned char *str = beep2str(beep);
	fprintf(stderr, "%p: %s\n", beep, str);
	free(str);
}

uint8_t *marshal_beep(Beep *beep) { // allocs memory
	if (beep == NULL || !valid_version_beep(beep)) {
		return NULL;
	}
	uint8_t *buf = calloc(sizeof(Beep)+beep->label_len+beep->msg_len, sizeof(uint8_t));
	buf[version_major_idx] = beep->version_major;
	buf[version_minor_idx] = beep->version_minor;
	buf[msg_start_idx] = beep->label_len+'0';
	strncpy(buf+msg_start_idx+1, beep->label, beep->label_len);
	buf[msg_start_idx+beep->label_len+1] = beep->msg_len+'0';
	strncpy(buf+msg_start_idx+1+beep->label_len+1, beep->msg, beep->msg_len);
	return buf;
}

Beep *unmarshal_beep(uint8_t *buf) { // allocs memory
	if (buf == NULL || !valid_version_buf(buf)) {
		return NULL;
	}
	
	Beep *beep = calloc(1, sizeof(Beep));

	beep->version_major = buf[version_major_idx];
	beep->version_minor = buf[version_minor_idx];
	beep->label_len = buf[msg_start_idx]-'0';
	beep->label = calloc(beep->label_len+1, sizeof(unsigned char));
	strncpy(beep->label, buf+msg_start_idx+1, beep->label_len);

	beep->msg_len = buf[msg_start_idx+1+beep->label_len]-'0';
	beep->msg = calloc(beep->msg_len+1, sizeof(unsigned char));
	strncpy(beep->msg, buf+msg_start_idx+1+beep->label_len+1, beep->msg_len);
	return beep;
}

#endif // BEEP_IMPLEMENTATION

#ifndef BEEP_QUEUE_H_
#define BEEP_QUEUE_H_

typedef struct BeepQueue {
	struct BeepQueue *next;
	Beep *beep;
} BeepQueue;

void free_entire_queue(BeepQueue *queue);
void print_queue(BeepQueue *queue);

BeepQueue *queue_push(BeepQueue *queue, Beep *beep);
BeepQueue *queue_pop(BeepQueue *queue);
Beep *queue_head(BeepQueue *queue);

#endif // BEEP_QUEUE_H_

#ifndef BEEP_QUEUE_IMPLEMENTATION
#define BEEP_QUEUE_IMPLEMENTATION

void free_entire_queue(BeepQueue *queue) {
	for (;queue != NULL; ) {
		BeepQueue *me = queue;
		queue = queue->next;
		free_beep(queue->beep);
		free(me);
	}
}

void print_queue(BeepQueue *queue) {
	fprintf(stderr, "QUEUE: ");
	for (;queue != NULL; queue = queue->next) {
		fprintf(stderr, "-> ");
		print_beep(queue->beep);
	}
	fprintf(stderr, "-> ");
	print_beep(NULL);
}

BeepQueue *queue_push(BeepQueue *queue, Beep *beep) {
	BeepQueue *next = malloc(1*sizeof(BeepQueue));
	next->beep = beep;
	next->next = NULL;
	if (queue == NULL) {
		return next;
	}
	BeepQueue *cur = queue;
	for (;;cur = cur->next) {
		if (cur->beep == beep) {
			fprintf(stderr, "ignoring already queued beep %p\n", beep);
			free(next);
			goto exit;
		}
		if (cur->next == NULL) {
			break;
		}
	}

	cur->next = next;
exit:
	return queue;
}

BeepQueue *queue_pop(BeepQueue *queue) {
	if (queue == NULL) {
		return NULL;
	}
	BeepQueue *me = queue;
	queue = queue->next;
	// free_queue_node(me);
	free_beep(me->beep);
	free(me);
	return queue;
}

Beep *queue_head(BeepQueue *queue) {
	if (queue == NULL) {
		return NULL;
	}
	return queue->beep;
}

#endif // BEEP_QUEUE_IMPLEMENTATION