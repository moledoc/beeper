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
void server_handle(int sockfd);

int client_setup();
void client_handle(int sockfd, char *msg);

int valid_version(uint8_t *buf);
#endif // SOCK_H_

#ifndef SOCK_VARS
#define SOCK_VARS

enum metadata {
	version_minor_idx,
	version_major_idx,
	metadata_size,
};

uint8_t version_major = '0';
uint8_t version_minor = '1';

const char *sock_path = "/tmp/beepd.sock";

struct sockaddr_un server_addr;
socklen_t server_addr_len;
#endif // SOCK_VARS

#ifndef SOCK_UTILS
#define SOCK_UTILS

int valid_version(uint8_t *buf) {
	return buf[version_major_idx] == version_major && \
		buf[version_minor_idx] == version_minor;
}

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


void server_handle(int sockfd) {
	int conn;
	char *msg;

	if ((conn = accept(sockfd, (struct sockaddr *) &server_addr, &server_addr_len)) == -1) {
		goto exit;
	}
	uint8_t read_buf[1024];
	memset(read_buf, 0, sizeof(read_buf));
	int n = read(conn, read_buf, sizeof(read_buf));
	if (n < sizeof(read_buf)) {
		read_buf[n] = '\0';
	}
	if (n < metadata_size) {
		goto invalid_msg_size;
	}
	if (!valid_version(read_buf)) {
		goto invalid_version;
	}

	// TODO: unmarshal messages
	// TODO: marshal response
	// MAYBE: TODO: send response

	fprintf(stderr, "%s\n", read_buf);
	msg = "success";
	goto response;

// TODO: better responses
invalid_msg_size:
	msg = "not enough metadata sent";
	goto response;

invalid_version:
	msg = "invalid version";
	goto response;

response:
	fprintf(stderr, "%s\n", msg);
	write(conn, msg, strlen(msg));
	goto exit;

exit:
	close(conn);
	return;
}


int client_setup() {
	return socket(AF_UNIX, SOCK_STREAM, 0);
}

void client_handle(int sockfd, char *msg) {
	struct sockaddr_un addr;
	socklen_t addr_len;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path)-1);
	addr_len = sizeof(addr);

	if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		goto cleanup;
	}

	size_t msg_len = strlen(msg);
	size_t enhanced_msg_len = metadata_size+msg_len;
	char *enhanced_msg = calloc(enhanced_msg_len, sizeof(char));
	enhanced_msg[version_major_idx] = version_major;
	enhanced_msg[version_minor_idx] = version_minor;
	strncpy(enhanced_msg+2, msg, msg_len);

	// TODO: marshal struct	
	write(sockfd, enhanced_msg, enhanced_msg_len);
	free(enhanced_msg);

	// TODO: unmarshal response
	char buf[512];
	memset(buf, 0, sizeof(buf));
	int n = read(sockfd, buf, sizeof(buf));
	printf("response: %s\n", buf);

	goto exit;

cleanup:
	fprintf(stderr, "cleanup: %s\n", strerror(errno));
	goto exit;
exit:
	return;
}

#endif // SOCK_IMPLEMENTATION