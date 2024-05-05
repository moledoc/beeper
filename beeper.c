#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include "protocol.h"
#include "context.h"
#include "raylib.h"

Context *context = NULL;

void *sock_listener(void *_) {
	char *path = "/tmp/beeper.sock";
	TraceLog(LOG_INFO, "start listening to socket '%s'", path);
	int unlnk = unlink(path);
	if (unlnk == -1) {
		TraceLog(LOG_ERROR, "failed to unlink '%s': %s", path, strerror(errno));
		is_on(context, true);
		return 0;
	}
		
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		TraceLog(LOG_ERROR, "failed to create unix socket at '%s': %s", path, strerror(errno));
		goto exit;
	}
	int option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
	socklen_t addr_len = sizeof(addr);

	TraceLog(LOG_INFO, "binding to socket '%s'", path);
	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		TraceLog(LOG_ERROR, "failed to bind socket '%s': %s\n", path, strerror(errno));
		goto exit;
	}
	TraceLog(LOG_INFO, "bound to socket '%s'", path);
	if (listen(sockfd, 0) == -1) {
		TraceLog(LOG_ERROR, "failed to listen socket '%s': %s\n", path, strerror(errno));
		goto exit;
	}
	TraceLog(LOG_INFO, "listening to socket '%s'", path);

	for (;;) {
		int conn = accept(sockfd, (struct sockaddr *) &addr, &addr_len);
		if (conn == -1) {
			TraceLog(LOG_ERROR, "accepting connection failed: %s", strerror(errno));
			close(conn);
			goto exit;
		}
		TraceLog(LOG_INFO, "connection accepted");

		uint8_t buf[PACKET_SIZE];
		memset(buf, 0, sizeof(buf));
		int n = read(conn, buf, sizeof(buf));
		TraceLog(LOG_INFO, "read %d bytes", n);

		TraceLog(LOG_INFO, "closing connection");
		if(close(conn) == -1) {
			TraceLog(LOG_ERROR, "closing connection failed: %s", strerror(errno));
		}

		// TODO: closing listener
		if (n > 0 && buf[0] == 'q') {
			TraceLog(LOG_INFO, "quitting application");
			break;
		}

		Packet *packet = unmarshal(buf);
		packet_log("unmarshalled payload: %s", packet);
		uint8_t *buff = marshal(*packet);
		buf_log("marshalled payload: %s", buff);

		packet_log("head of queue: %s", stored_packets_head(context));

		store_packet(context, *packet);
		stored_packets_log(context);

		// pop_packet(context);
		// stored_packets_log(context);

		packet_free(packet);
		free(buff);
	}

exit:
	is_on(context, true);
	if(close(sockfd) == -1) {
		TraceLog(LOG_ERROR, "failed to close socket: %s\n", strerror(errno));
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
