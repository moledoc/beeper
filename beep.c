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
  int ret_val = 0;

  for (int i = 1; i < argc; ++i) {
    if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0 ||
        strcmp("help", argv[i]) == 0) {
      help();
      return 0;
    } else if (strcmp("-q", argv[i]) == 0 || strcmp("--quit", argv[i]) == 0 ||
               strcmp("quit", argv[i]) == 0) {
      label = calloc(1, sizeof(char));
      strncpy(label, "q", 1 * sizeof(char));
      msg = calloc(1, sizeof(char));
      strncpy(msg, "q", 1 * sizeof(char));
      break;
    } else if ((strcmp("-l", argv[i]) == 0 ||
                strcmp("--label", argv[i]) == 0) &&
               i + 1 < argc) {
      size_t label_len = strlen(argv[i + 1]);
      label = calloc(label_len, sizeof(char));
      memcpy(label, argv[i + 1], label_len);
      ++i;
    } else if ((strcmp("-m", argv[i]) == 0 || strcmp("--msg", argv[i]) == 0) &&
               i + 1 < argc) {
      size_t msg_len = strlen(argv[i + 1]);
      msg = calloc(msg_len, sizeof(char));
      memcpy(msg, argv[i + 1], msg_len);
      ++i;
    } else {
      fprintf(stderr, "invalid argument '%s' provided\n", argv[i]);
      return 1;
    }
  }

  if (strcmp("", label) == 0) {
    label = "beep";
  }
  if (strcmp("", msg) == 0) {
    fprintf(stderr, "empty message not allowed\n");
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

  // int n =
  write(sock_fd, request_buf, buf_len(request_buf));
  // fprintf(stderr, "wrote %d bytes to socket\n", n);
  // buf_print(request_buf);

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