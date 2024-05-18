#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

const char *sock_path = "/tmp/beepd.sock";

struct sockaddr_un addr;
socklen_t addr_len;

int socket_setup() {
	int sockfd;
	int option;
	unlink(sock_path);
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		goto exit;
	}
	option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path)-1);
	addr_len = sizeof(addr);

	if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		goto cleanup;
	}
	if (listen(sockfd, 5) == -1) {
		goto cleanup;
	}

	goto exit;

cleanup:
	fprintf(stderr, "%s\n", strerror(errno));
	close(sockfd);
	sockfd = -1;
	goto exit;

exit:
	return sockfd;
}

void handle(int sockfd) {
	int conn;

	conn = accept(sockfd, (struct sockaddr *) &addr, &addr_len);
	if (conn == -1) {
		goto exit;
	}
	uint8_t buf[1024];
	memset(buf, 0, sizeof(buf));
	int n = read(conn, buf, sizeof(buf));
	if (n < sizeof(buf)) {
		buf[n] = '\0';
	}
	printf("%s\n", buf);

	goto exit;

exit:
	close(conn);
	return;
}

int main(void) {
	int sockfd = socket_setup();
	printf("sockfd: %d\n", sockfd);
	handle(sockfd);
	if (sockfd != -1) {
		close(sockfd);
	}
}