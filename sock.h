#pragma once

#ifdef SOCK_CONST

const char *__sock_path = "/tmp/beepd.sock";

#endif // SOCK_CONST

#ifndef SOCK_H_
#define SOCK_H_

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int fd();
struct sockaddr_un *get_addr(const char *path); // allocs memory
int conn_fd(int fd, const char *path);
void set_opts(int fd);
int bind_fd(int fd, struct sockaddr_un *addr);

#endif // SOCK_H_

#ifdef SOCK_IMPLEMENTATION

int fd() { return socket(AF_UNIX, SOCK_STREAM, 0); }

struct sockaddr_un *get_addr(const char *path) {
  struct sockaddr_un *addr = calloc(1, sizeof(struct sockaddr_un));
  memset(addr, 0, sizeof(addr));
  addr->sun_family = AF_UNIX;
  strncpy(addr->sun_path, path, sizeof(addr->sun_path) - 1);
  return addr;
}

int conn_fd(int fd, const char *path) {
  struct sockaddr_un *addr = get_addr(path);
  return connect(fd, (struct sockaddr *)addr, sizeof(*addr));
}

void set_opts(int fd) {
  int opt = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

int bind_fd(int fd, struct sockaddr_un *addr) {
  set_opts(fd);
  return bind(fd, (struct sockaddr *)addr, sizeof(*addr));
}

#endif // SOCK_IMPLEMENTATION