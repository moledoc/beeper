#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

int server_setup();
void server_handle(int sockfd);

int client_setup();
void client_handle(int sockfd, char *msg);


const char *sock_path = "/tmp/beepd.sock";

struct sockaddr_un server_addr;
socklen_t server_addr_len;

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

	if ((conn = accept(sockfd, (struct sockaddr *) &server_addr, &server_addr_len)) == -1) {
		goto exit;
	}
	uint8_t read_buf[1024];
	memset(read_buf, 0, sizeof(read_buf));
	int n = read(conn, read_buf, sizeof(read_buf));
	if (n < sizeof(read_buf)) {
		read_buf[n] = '\0';
	}
	printf("%s\n", read_buf);

	// TODO: unmarshal messages
	// TODO: marshal response
	// MAYBE: TODO: send response
	uint8_t write_buf[3];
	memset(write_buf, 0, sizeof(write_buf));
	write_buf[0] = '1';
	write(conn, write_buf, sizeof(write_buf));

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

	// TODO: marshal struct	
	write(sockfd, msg, strlen(msg));

	// TODO: unmarshal response
	char *buf[4];
	int n = read(sockfd, buf, sizeof(buf));
	if (n < sizeof(buf)) {
		buf[n] = '\0';
	}
	printf("%s\n", buf);

	goto exit;

cleanup:
	fprintf(stderr, "%s\n", strerror(errno));
	goto exit;
exit:
	return;
}
