#include <stdio.h>

#include "socks.h"
#include "beep.h"

int main(void) {
	int sockfd = server_setup();
	if (sockfd == -1) {
		return 1;
	}

	printf("sockfd: %d\n", sockfd);
	uint8_t *buf = server_handle(sockfd);

	fprintf(stderr, "beepd: %s\n", buf);

	Beep *beep = unmarshal_beep(buf);
	free_buf(buf);

	print_beep(beep);
	free_beep(beep);

	close(sockfd);
}