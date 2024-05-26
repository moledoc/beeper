#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCK_CONST
#define SOCK_IMPLEMENTATION
#include "sock.h"

#define BEEP_CONST
#define BEEP_IMPLEMENTATION
#include "beep.h"

void help() { fprintf(stdout, "TODO:\n"); }

int main(int argc, char **argv) {

  char *label = NULL;
  char *msg = NULL;
  int ret_val = 0;

  for (int i = 1; i < argc; ++i) {
    if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0 ||
        strcmp("help", argv[i]) == 0) {
      help();
      return 0;
    } else if (strcmp("-l", argv[i]) == 0 || strcmp("--label", argv[i]) == 0) {
      size_t label_len = strlen(argv[i]);
      label = calloc(label_len, sizeof(char));
      memcpy(label, argv[i], label_len);
    } else if (strcmp("-m", argv[i]) == 0 || strcmp("--msg", argv[i]) == 0) {
      size_t msg_len = strlen(argv[i]);
      msg = calloc(msg_len, sizeof(char));
      memcpy(msg, argv[i], msg_len);
    } else {
      fprintf(stderr, "invalid argument '%s' provided\n", argv[i]);
      return 1;
    }
  }

  int sock_fd = fd();
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

  Beep *beep = mk_beep(label, msg);
  if (beep == NULL) {
    ret_val = 1;
    goto exit;
  }
  char *request_buf = marshal(beep);
  if (request_buf == NULL) {
    ret_val = 1;
    goto exit;
  }

  int n = write(sock_fd, request_buf, buf_len(msg));
  fprintf(stderr, "wrote %d bytes to socket\n", n);

exit:
  if (label != NULL) {
    free(label);
  }
  if (msg != NULL) {
    free(msg);
  }
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