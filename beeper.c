#include <stdio.h>
#include <stdlib.h>

#include "protocol.h"

int main() {
	Packet packet = {.version_minor=1, .version_major=0, .repeat=0, .timer=4.76, .msg="hey"};
	uint8_t *buf = marshal(packet);
	printf("%s\n", buf+5);
	for (int i=0;i<METADATA_SIZE;++i) {
		printf("%u\n", buf[i]);
	}

	print_packet(packet);

	free(buf);
}