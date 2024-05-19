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


#ifndef BEEP_UTILS
#define BEEP_UTILS

int valid_version(uint8_t *buf);

int valid_version(uint8_t *buf) {
	return buf[version_major_idx] == version_major && \
		buf[version_minor_idx] == version_minor;
}

#endif // SOCK_UTILS_

#ifndef BEEP_IMPLEMENTATION
#define BEEP_IMPLEMENTATION

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
	}
	unsigned char *str = beep2str(beep);
	fprintf(stderr, "%s\n", str);
	free(str);
}

uint8_t *marshal_beep(Beep *beep) { // allocs memory
	if (beep == NULL) {
		return NULL;
	}
	uint8_t *buf = calloc(sizeof(Beep)+beep->label_len+beep->msg_len, sizeof(uint8_t));
	buf[version_major_idx] = beep->version_major;
	buf[version_minor_idx] = beep->version_minor;
	buf[msg_start_idx] = beep->label_len;
	strncpy(buf+msg_start_idx+1, beep->label, beep->label_len);
	buf[msg_start_idx+beep->label_len+1] = beep->msg_len;
	strncpy(buf+msg_start_idx+1+beep->label_len+1, beep->msg, beep->msg_len);
	return buf;
}

Beep *unmarshal_beep(uint8_t *buf) { // allocs memory
	if (buf == NULL) {
		return NULL;
	}
	Beep *beep = calloc(1, sizeof(Beep));

	beep->label_len = buf[0];
	beep->label = calloc(beep->label_len+1, sizeof(unsigned char));
	strncpy(beep->label, buf+1, beep->label_len);

	beep->label_len = buf[1+beep->label_len];
	beep->msg = calloc(beep->msg_len+1, sizeof(unsigned char));
	strncpy(beep->msg, buf+1+beep->label_len+1, beep->msg_len);

	print_beep(beep);
	return beep;
}



#endif // BEEP_IMPLEMENTATION