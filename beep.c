#include <stdio.h>
#include <string.h>

#include "socks.h"
#include "beep.h"

int main(int argc, char **argv) {
	int sockfd = client_setup();
	if (sockfd == -1) {
		return 1;
	}
	printf("sockfd: %d\n", sockfd);

	Beep beep = {.version_major=version_major, .version_minor=version_minor, .label="hey", .msg="heeehoo"};
	beep.label_len=strlen(beep.label);
	beep.msg_len=strlen(beep.msg);

	print_beep(&beep);

	uint8_t *msg = marshal_beep(&beep);
	printf("msg: %s\n", msg);

	client_handle(sockfd, msg);

	free(msg);
	close(sockfd);
	return 0;
}