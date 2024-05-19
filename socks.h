#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef SOCK_H_
#define SOCK_H_
int server_setup();
uint8_t *server_handle(int sockfd);

int client_setup();
void client_handle(int sockfd, uint8_t *msg);

int valid_version(uint8_t *buf);
#endif // SOCK_H_

#ifndef SOCK_VARS
#define SOCK_VARS

const char *sock_path = "/tmp/beepd.sock";

struct sockaddr_un server_addr;
socklen_t server_addr_len;
#endif // SOCK_VARS

#ifndef SOCK_UTILS
#define SOCK_UTILS

#endif // SOCK_UTILS_

#ifndef SOCK_IMPLEMENTATION
#define SOCK_IMPLEMENTATION

int server_setup() {
	int sockfd;
	int option;

	unlink(sock_path);

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		goto exit;
	}
	option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	strncpy(server_addr.sun_path, sock_path, sizeof(server_addr.sun_path)-1);
	server_addr_len = sizeof(server_addr);

	if (bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1) {
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


uint8_t *server_handle(int sockfd) {
	int conn;
	uint8_t *buf = NULL;

	if ((conn = accept(sockfd, (struct sockaddr *) &server_addr, &server_addr_len)) == -1) {
		goto exit;
	}
	size_t buf_size = 1024;
	buf = calloc(buf_size, sizeof(uint8_t));
	int n = read(conn, buf, buf_size);
	if (n < sizeof(buf)) {
		buf[n] = '\0';
	}
	goto exit;

exit:
	close(conn);
	return buf;
}


int client_setup() {
	return socket(AF_UNIX, SOCK_STREAM, 0);
}

void client_handle(int sockfd, uint8_t *msg) {
	struct sockaddr_un addr;
	socklen_t addr_len;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path)-1);
	addr_len = sizeof(addr);

	if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		goto cleanup;
	}

	write(sockfd, msg, strlen(msg));

	goto exit;

cleanup:
	fprintf(stderr, "error: %s\n", strerror(errno));
	goto exit;
exit:
	return;
}

#endif // SOCK_IMPLEMENTATION