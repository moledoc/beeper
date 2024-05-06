#pragma once

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "raylib.h"

void mmcp(uint8_t *dest, const uint8_t *src, uint8_t len) {
	unsigned char *src_cp = (unsigned char*)src;
	for (uint8_t i=0; i<len || (*src_cp) != '\0'; ++i, ++src_cp) {
		dest[i] = src[i];
	}
}

uint32_t sz(uint8_t *buf) {
	uint32_t size = 0;
	for (; *(buf+size) != '\0' && size<UINT8_MAX; ++size) {
		;
	}
	return size;
}

enum METADATA_IDX {
	IDX_VERSION_MINOR,
	IDX_VERSION_MAJOR,
	IDX_REPEAT,
	IDX_TIMER_FRAC,
	IDX_TIMER_INT,
	METADATA_SIZE,
};

#define VERSION_MAJOR ('0')
#define VERSION_MINOR ('1')
#define PACKET_SIZE (UINT8_MAX)
#define MSG_SIZE (PACKET_SIZE-METADATA_SIZE)

typedef struct {
	pthread_mutex_t mutex;
	uint32_t id;
} AtomicId;

AtomicId *init_atomic_id() { // allocs memory
	AtomicId *id = calloc(1, sizeof(AtomicId));
	id->id = 1;
	pthread_mutex_init(&id->mutex, NULL);
	return id;
}

void free_atomic_id(AtomicId *id) {
	if (id == NULL) {
		return;
	}
	free(id);
}

AtomicId *aid = NULL;

uint32_t atomic_id() {
	if (aid == NULL) {
		TraceLog(LOG_WARNING, "atomic id counter not initialized properly");
		return 0;
	}
	pthread_mutex_lock(&aid->mutex);
	uint32_t id = aid->id;
	++(aid->id);
	pthread_mutex_unlock(&aid->mutex);
	TraceLog(LOG_INFO, "updated id from %u to %u", id, aid->id);
	return id;
}

typedef struct {
	uint8_t version_minor;
	uint8_t version_major;
	bool repeat;
	double timer;
	unsigned char *msg; //[MSG_SIZE+1];
	uint8_t msg_len;
	uint32_t id;
} Packet;
#define PACKET_FIELDS_COUNT 7

char *packet_string(Packet packet); // allocs memory
char *buf_string(uint8_t *buf); // allocs memory
void packet_log(char *format, Packet *packet);
void buf_log(char *format, uint8_t *buf);
void packet_free(Packet *packet);
uint8_t *marshal(Packet packet); // allocs memory
Packet *unmarshal(uint8_t *buf); // allocs memory
void proto_send(); // TODO:
void proto_receive(); // TODO:

char *packet_string(Packet packet) {
	char *str = calloc(512, sizeof(char));
	if (str == NULL) {
		return NULL;
	}
	snprintf(str, 512*sizeof(char), "{"
		"\n\t.version_minor=%c"
		"\n\t.version_major=%c"
		"\n\t.repeat=%u"
		"\n\t.timer=%0.1f"
		"\n\t.msg=%s"
		"\n\t.msg_len=%u"
		"\n\t.id=%u"
		"\n}",
		packet.version_minor,
		packet.version_major,
		packet.repeat,
		floor(packet.timer*10.0)/10.0,
		packet.msg,
		packet.msg_len,
		packet.id
	);
	return str;
}

void packet_log(char *format, Packet *packet) {
	if (packet == NULL) {
		TraceLog(LOG_INFO, format, "{NULL}");
		return;
	}
	char *str = packet_string(*packet);
	if (str == NULL) {
		TraceLog(LOG_ERROR, "failed to make packet to string");
		return;
	}
	TraceLog(LOG_INFO, format, str);
	free(str);
}

char *buf_string(uint8_t *buf) {
	assert(METADATA_SIZE==5);
	char *str = calloc(PACKET_SIZE, sizeof(char));
	if (str == NULL) {
		TraceLog(LOG_ERROR, "failed to make buf to string");
		return NULL;
	}
	for (int i=0;i<METADATA_SIZE;++i) {
		snprintf(str+i, 2*sizeof(char), "%c", buf[i]);
	}
	snprintf(str+METADATA_SIZE, MSG_SIZE*sizeof(char), "%s", buf+METADATA_SIZE);
	return str;
}

void buf_log(char *format, uint8_t *buf) {
	if (buf == NULL) {
		TraceLog(LOG_INFO, format, "{NULL}");
		return;
	}
	char *str = buf_string(buf);
	if (str == NULL) {
		TraceLog(LOG_ERROR, "failed to make buf to string");
		return;
	}
	TraceLog(LOG_INFO, format, str);
	free(str);
}

void packet_free(Packet *packet) {
	if (packet == NULL) {
		return;
	}
	if (packet->msg != NULL) {
		free(packet->msg);
	}
	free(packet);
}


uint8_t *marshal(Packet packet) {
	assert(METADATA_SIZE == 5);
	assert(PACKET_FIELDS_COUNT == 7);
	packet_log("marshalling packet: %s", &packet);

	if (packet.version_major != VERSION_MAJOR) {
		TraceLog(LOG_ERROR, "marshal major version mismatch: expected %c, got %c\n", VERSION_MAJOR, packet.version_major);
		return NULL;
	}
	if (packet.version_minor != VERSION_MINOR) {
		TraceLog(LOG_ERROR, "marshal minor version mismatch: expected %c, got %c\n", VERSION_MINOR, packet.version_minor);
		return NULL;
	}

	uint8_t *buf = calloc(PACKET_SIZE+1, sizeof(uint8_t));
	buf[IDX_VERSION_MINOR] = packet.version_minor;
	buf[IDX_VERSION_MAJOR] = packet.version_major;
	buf[IDX_REPEAT] = (uint8_t)packet.repeat+'0';
	buf[IDX_TIMER_FRAC] = (uint8_t)((uint32_t)(packet.timer*10.0)%10)+'0';
	buf[IDX_TIMER_INT] = (uint8_t)packet.timer+'0';
	mmcp(buf+(uint8_t)METADATA_SIZE, packet.msg, MSG_SIZE);
	return buf;
}

Packet *unmarshal(uint8_t *buf) {
	assert(METADATA_SIZE == 5);
	assert(PACKET_FIELDS_COUNT == 7);
	uint32_t buf_size = sz(buf);
	if (buf_size < METADATA_SIZE) {
		TraceLog(LOG_ERROR, "unmarshal buffer size less than metadata: expected >%d, got %d\n", METADATA_SIZE, buf_size);
		return NULL;
	}

	buf_log("unmarshalling buf: %s", buf);

	if (buf[IDX_VERSION_MAJOR] != VERSION_MAJOR) {
		TraceLog(LOG_ERROR, "unmarshal major version mismatch: expected %c, got %c\n", VERSION_MAJOR, buf[IDX_VERSION_MAJOR]);
		return NULL;
	}
	if (buf[IDX_VERSION_MINOR] != VERSION_MINOR) {
		TraceLog(LOG_ERROR, "unmarshal minor version mismatch: expected %c, got %c\n", VERSION_MINOR, buf[IDX_VERSION_MINOR]);
		return NULL;
	}

	Packet *packet = calloc(1, sizeof(Packet));
	packet->version_minor = buf[IDX_VERSION_MINOR];
	packet->version_major = buf[IDX_VERSION_MAJOR];
	packet->repeat = buf[IDX_REPEAT]-'0';
	packet->timer = buf[IDX_TIMER_INT]-'0'+(double)(buf[IDX_TIMER_FRAC]-'0')/10.0;
	packet->msg = calloc(MSG_SIZE+1, sizeof(unsigned char));
	mmcp(packet->msg, buf+METADATA_SIZE, MSG_SIZE);
	packet->msg_len = sz(packet->msg);
	packet->id = atomic_id();
	return packet;	
}

#endif // PROTOCOL_H_