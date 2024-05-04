#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "protocol.h"
#include "context.h"

// TODO: logging

Context *context = NULL;

void *sock_listener(void *_) {
	char *path = "/tmp/beeper.sock";
	unlink(path);
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	int option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	if (sockfd == -1) {
		fprintf(stderr, "failed to get socket file descriptor: %s\n", strerror(errno));
		goto exit;
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
	socklen_t addr_len = sizeof(addr);

	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		fprintf(stderr, "failed to bind socket '%s': %s\n", path, strerror(errno));
		goto exit;
	}
	if (listen(sockfd, 0) == -1) {
		fprintf(stderr, "failed to listen socket '%s': %s\n", path, strerror(errno));
		goto exit;
	}

	for (;;) {
		int conn = accept(sockfd, (struct sockaddr *) &addr, &addr_len);
		if (conn == -1) {
			fprintf(stderr, "failed to accept socket '%s': %s\n", path, strerror(errno));
			close(conn);
			goto exit;
		}
	
		uint8_t buf[PACKET_SIZE];
		memset(buf, 0, sizeof(buf));
		int n = read(conn, buf, sizeof(buf));
		if(close(conn) == -1) {
			fprintf(stderr, "failed to close conn: %s\n", strerror(errno));
		}

		// TODO: closing listener
		if (n > 0 && buf[0] == 'q') {
			break;
		}

		Packet *packet = unmarshal(buf);
		uint8_t *buff = marshal(*packet);
		free_packet(packet);
		free(buff);
	}

exit:
	should_continue(context, true);
	if(close(sockfd) == -1) {
		fprintf(stderr, "failed to close socket: %s\n", strerror(errno));
	}
	return 0;
}

int main() {
	context = init_context();

	uint8_t threads_count = 1;
	pthread_t threads[threads_count];

	pthread_create(&threads[0], NULL, sock_listener, NULL);
	
	for (int i=0; i<threads_count; ++i) {
		pthread_join(threads[i], NULL);
	}
	free_context(context);
}

/*
int main1() {
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
*/