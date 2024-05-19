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

	// print_beep(beep);

	BeepQueue *queue = NULL;
	queue = queue_push(queue, beep);
	queue = queue_push(queue, beep);
	queue = queue_push(queue, beep);
	print_queue(queue);
	print_beep(queue_head(queue));
	queue = queue_pop(queue);
	print_queue(queue);


	close(sockfd);
}