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

	fprintf(stderr, "beepd1: %s\n", buf);

	Beep *beep = unmarshal_beep(buf);

	unsigned char *beep_str = beep2str(beep);
	printf("%s\n", beep_str);
	free(beep_str);

	free_beep(beep);
	fprintf(stderr, "beepd2: %s\n", buf);
	// free(buf);

	close(sockfd);
}