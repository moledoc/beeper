#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCK_CONST
#define SOCK_IMPLEMENTATION
#include "sock.h"

#define BEEP_CONST
#define BEEP_IMPLEMENTATION
#include "beep.h"

void help() {
  char *help_msg = "TODO:\n";
#ifdef TEST_OUT
  write(TEST_OUT, help_msg, strlen(help_msg));
#else
  fprintf(stdout, help_msg);
#endif
}

int main(int argc, char **argv) {

  char *label = NULL;
  char *msg = NULL;
  char *request_buf = NULL;
  Beep *beep = NULL;
  int sock_fd = -1;
  int ret_val = 0;

  for (int i = 1; i < argc; ++i) {
    if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0 ||
        strcmp("help", argv[i]) == 0) {
      help();
      return 0;
    } else if (strcmp("-q", argv[i]) == 0 || strcmp("--quit", argv[i]) == 0 ||
               strcmp("quit", argv[i]) == 0) {
      label = "q";
      msg = "q";
      break;
    } else if ((strcmp("-l", argv[i]) == 0 ||
                strcmp("--label", argv[i]) == 0) &&
               i + 1 < argc) {
      label = strlen(argv[i + 1]) > 0 ? argv[i + 1] : "beep";
      ++i;
    } else if ((strcmp("-m", argv[i]) == 0 || strcmp("--msg", argv[i]) == 0) &&
               i + 1 < argc) {
      msg = argv[i + 1];
      ++i;
    } else {
      fprintf(stderr, "invalid argument '%s' provided\n", argv[i]);
      return 1;
    }
  }

  if (label == NULL || strlen(label) == 0) {
    fprintf(stderr, "something unexpected happened with label flag\n");
    ret_val = 1;
    goto exit;
  }
  if (msg == NULL || strlen(msg) == 0) {
    fprintf(stderr, "empty message not allowed\n");
    ret_val = 1;
    goto exit;
  }

  sock_fd = fd();
  if (sock_fd == -1) {
    ret_val = 1;
    goto exit;
  }
  const char *sock_path = __sock_path;
  if (conn_fd(sock_fd, sock_path) == -1) {
    fprintf(stderr, "failed to connect to '%s'\n", sock_path);
    ret_val = 1;
    goto exit;
  }

  beep = mk_beep(label, msg);
  if (beep == NULL) {
    ret_val = 1;
    goto exit;
  }
  request_buf = marshal(beep);
  if (request_buf == NULL) {
    ret_val = 1;
    goto exit;
  }

  write(sock_fd, request_buf, buf_len(request_buf));

exit:
  if (request_buf != NULL) {
    free(request_buf);
  }
  if (beep != NULL) {
    beep_free(beep);
  }
  if (sock_fd != -1) {
    close(sock_fd);
  }
  return ret_val;
}