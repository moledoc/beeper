#include <stdio.h>
#include <stdlib.h>

#include "protocol.h"

int main() {
	Packet packet = {.version_minor='1', .version_major='0', .repeat=1, .timer=4.76, .msg=(unsigned char*)"hey"};
	print_packet(packet);
	uint8_t *buf = marshal(packet);
	if (buf == NULL) {
		fprintf(stderr, "invalid version: got %d.%d, expected %d.%d\n", packet.version_major, packet.version_minor, VERSION_MAJOR, VERSION_MINOR);
		return 1;
	}
	printf(".msg=%s\n", buf+5);
	for (int i=0;i<METADATA_SIZE;++i) {
		printf("%u\n", buf[i]);
	}

	Packet *pckt = unmarshal(buf);
	if (pckt == NULL) {
		fprintf(stderr, "failed to unmarshal buf\n");
		return 1;
	}
	print_packet(*pckt);

	free(buf);
	free_packet(pckt);
}