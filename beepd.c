#include <stdio.h>

#include "socks.h"

int main(void) {
	int sockfd = server_setup();
	if (sockfd == -1) {
		return 1;
	}

	printf("sockfd: %d\n", sockfd);
	server_handle(sockfd);
	close(sockfd);
}