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
int conn_fd(int fd, const char *path);

#endif // SOCK_H_

#ifdef SOCK_IMPLEMENTATION

int fd() { return socket(AF_UNIX, SOCK_STREAM, 0); }

int conn_fd(int fd, const char *path) {
  struct sockaddr_un addr;
  socklen_t addr_len;

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
  addr_len = sizeof(addr);

  return connect(fd, (struct sockaddr *)&addr, sizeof(addr));
}

#endif // SOCK_IMPLEMENTATION