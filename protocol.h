#pragma once

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

void mmcp(uint8_t *dest, const uint8_t *src, uint8_t len) {
	uint8_t *src_cp = (uint8_t *)src;
	for (uint8_t i=0; i<len || (*src_cp) == '\0'; ++i, ++src_cp) {
		dest[i] = src[i];
	}
}

enum METADATA_IDX {
	IDX_VERSION_MINOR,
	IDX_VERSION_MAJOR,
	IDX_REPEAT,
	IDX_TIMER_FRAC,
	IDX_TIMER_INT,
	METADATA_SIZE,
};

#define VERSION_MAJOR (0)
#define VERSION_MINOR (1)
#define PACKET_SIZE (UINT8_MAX)
#define MSG_SIZE (PACKET_SIZE-METADATA_SIZE)

typedef struct {
	uint8_t version_minor;
	uint8_t version_major;
	bool repeat;
	double timer;
	unsigned char msg[MSG_SIZE+1];
} Packet;

void print_packet(Packet packet);
uint8_t *marshal(Packet packet); // allocs memory
Packet unmarshal(uint8_t *buf); // allocs memory

void print_packet(Packet packet) {
	printf("{\n\t.version_minor=%u,\n\t.version_major=%u,\n\t.repeat=%u\n\t.timer=%0.2f\n\t.msg=%s\n}\n", packet.version_minor, packet.version_major, packet.repeat, packet.timer, packet.msg);
}

uint8_t *marshal(Packet packet) {
	uint8_t *buf = calloc(PACKET_SIZE, sizeof(uint8_t));
	buf[0] = packet.version_minor;
	buf[1] = packet.version_major;
	buf[2] = (uint8_t)packet.repeat;
	buf[3] = (uint8_t)((uint32_t)(packet.timer*10.0)%10);
	buf[4] = (uint8_t)packet.timer;
	mmcp(buf+(uint8_t)5, packet.msg, MSG_SIZE);
	return buf;
}

Packet unmarshal(uint8_t *buf) {
}

#endif // PROTOCOL_H_