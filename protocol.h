#pragma once

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

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
	uint8_t version_minor;
	uint8_t version_major;
	bool repeat;
	double timer;
	unsigned char *msg; //[MSG_SIZE+1];
} Packet;

void print_packet(Packet packet);
void free_packet(Packet *packet);
uint8_t *marshal(Packet packet); // allocs memory
Packet *unmarshal(uint8_t *buf); // allocs memory
void send(); // TODO:
void receive(); // TODO:

void print_packet(Packet packet) {
	printf("Packet = {"
		"\n\t.version_minor=%c"
		"\n\t.version_major=%c"
		"\n\t.repeat=%u"
		"\n\t.timer=%0.1f"
		"\n\t.msg=%s"
		"\n}\n",
		packet.version_minor,
		packet.version_major,
		packet.repeat,
		floor(packet.timer*10.0)/10.0,
		packet.msg
	);
}

void free_packet(Packet *packet) {
	if (packet == NULL) {
		return;
	}
	if (packet->msg != NULL) {
		free(packet->msg);
	}
	free(packet);
}


uint8_t *marshal(Packet packet) {
	if (packet.version_major != VERSION_MAJOR) {
		fprintf(stderr, "[ERROR]: [MVMAMM]: marshal major version mismatch: expected %c, got %c\n", VERSION_MAJOR, packet.version_major);
		return NULL;
	}
	if (packet.version_minor != VERSION_MINOR) {
		fprintf(stderr, "[ERROR]: [MVMIMM]: marshal minor version mismatch: expected %c, got %c\n", VERSION_MINOR, packet.version_minor);
		return NULL;
	}
	uint8_t *buf = calloc(PACKET_SIZE+1, sizeof(uint8_t));
	buf[IDX_VERSION_MINOR] = packet.version_minor;
	buf[IDX_VERSION_MAJOR] = packet.version_major;
	buf[IDX_REPEAT] = (uint8_t)packet.repeat+'0';
	buf[IDX_TIMER_FRAC] = (uint8_t)((uint32_t)(packet.timer*10.0)%10);
	buf[IDX_TIMER_INT] = (uint8_t)packet.timer;
	mmcp(buf+(uint8_t)METADATA_SIZE, packet.msg, MSG_SIZE);
	return buf;
}

Packet *unmarshal(uint8_t *buf) {
	uint32_t buf_size = sz(buf);
	if (buf_size < METADATA_SIZE) {
		fprintf(stderr, "[ERROR]: [UMBSLTMD]: unmarshal buffer size less than metadata: expected >%d, got %d\n", METADATA_SIZE, buf_size);
		return NULL;
	}
	if (buf[IDX_VERSION_MAJOR] != VERSION_MAJOR) {
		fprintf(stderr, "[ERROR]: [UMVMAMM]: unmarshal major version mismatch: expected %c, got %c\n", VERSION_MAJOR, buf[IDX_VERSION_MAJOR]);
		return NULL;
	}
	if (buf[IDX_VERSION_MINOR] != VERSION_MINOR) {
		fprintf(stderr, "[ERROR]: [UMVMIMM]: unmarshal minor version mismatch: expected %c, got %c\n", VERSION_MINOR, buf[IDX_VERSION_MINOR]);
		return NULL;
	}
	Packet *packet = calloc(1, sizeof(Packet));
	packet->version_minor = buf[IDX_VERSION_MINOR];
	packet->version_major = buf[IDX_VERSION_MAJOR];
	packet->repeat = buf[IDX_REPEAT]-'0';
	packet->timer = buf[IDX_TIMER_INT]+(double)buf[IDX_TIMER_FRAC]/10.0;
	packet->msg = calloc(MSG_SIZE+1, sizeof(unsigned char));
	mmcp(packet->msg, buf+METADATA_SIZE, MSG_SIZE);
	return packet;	
}

#endif // PROTOCOL_H_