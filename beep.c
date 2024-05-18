#include <stdio.h>

#include "socks.h"

int main(int argc, char **argv) {
	int sockfd = client_setup();
	if (sockfd == -1) {
		return 1;
	}
	printf("sockfd: %d\n", sockfd);

	client_handle(sockfd, "lala");
	close(sockfd);
	return 0;
}