#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define SOCK_CONST
#define SOCK_IMPLEMENTATION
#include "sock.h"

#define BEEP_IMPLEMENTATION
#include "beep.h"

typedef struct {
  char *label;
  char *msg;
} Case;

int precond() { return system("cc -o beep beep.c"); }

beep_buf mk_expect(Case c) { // allocs memory
  char *label = c.label;
  char *msg = c.msg;
  assert(metadata_size == 3 && "extend expected result construction");

  size_t label_len = strlen(label);
  size_t msg_len = strlen(msg);

  beep_buf buf = (beep_buf)calloc(metadata_size + label_len + 1 + msg_len + 1,
                                  sizeof(char));

  buf[version_major_idx] = __version_major;
  buf[version_minor_idx] = __version_minor;

  buf[msg_start_idx] = label_len;
  strncpy(buf + msg_start_idx + 1, label, label_len);

  buf[msg_start_idx + 1 + label_len] = msg_len;
  strncpy(buf + msg_start_idx + 1 + label_len + 1, msg, msg_len);

  return buf;
}

void base_error_msg(beep_buf expected, beep_buf actual) {
  char *expected_as_str = buf_to_str(expected);
  char *actual_as_str = buf_to_str(actual);
  fprintf(stderr,
          "Comparing:"
          "\n\t- expected: '%s'"
          "\n\t- actual: '%s'\n",
          expected_as_str, actual_as_str);
  if (expected_as_str != NULL) {
    free(expected_as_str);
  }
  if (actual_as_str != NULL) {
    free(actual_as_str);
  }
  return;
}

int match(beep_buf expected, beep_buf actual) {
  int ret_val = 1;
  size_t expected_len = buf_len(expected);
  size_t actual_len = buf_len(actual);
  if (expected_len != actual_len) {
    base_error_msg(expected, actual);
    fprintf(stderr, "Length difference: expected %d, actual %d\n", expected_len,
            actual_len);
    return 0;
  }
  for (int i = 0; i < expected_len; ++i) {
    if (expected[i] != actual[i]) {
      base_error_msg(expected, actual);
      fprintf(stderr, "Difference in buffer at index %d\n", i);
      return 0;
    }
  }
  return 1;
}

void *get_actuals(void *c) {
  int count = *(int *)c;
  beep_buf *actuals = NULL;
  int sock_fd;

  sock_fd = fd();
  if (sock_fd == -1) {
    goto exit;
  }
  struct sockaddr_un *addr = get_addr(__sock_path);
  if (bind_fd(sock_fd, addr) == -1) {
    goto exit;
  }
  if (listen(sock_fd) == -1) {
    goto exit;
  }

  actuals = calloc(count, sizeof(beep_buf));
  for (int i = 0; i < count; ++i) {
    int conn;
    if ((conn = accept(sockfd, (struct sockaddr *)&addr, &addr_len)) == -1) {
      continue;
    }
  }

exit:
  close(sock_fd);
  return (void *)actuals;
}

int main(int argc, char **argv) {
  if (precond() == -1) {
    fprintf(stderr, "beep.c testing precondition failed\n");
    return 1;
  }

  Case both_empty = {.label = "", .msg = ""};
  Case label_empty = {.label = "", .msg = "not empty"};
  Case msg_empty = {.label = "not empty", .msg = ""};

  Case both_short = {.label = "short", .msg = "short message"};
  Case both_long = {
      .label = "very very very very very very very very very very very very "
               "very very very very very very very very very very very very "
               "very very very very very very very very very very very long",
      .msg = "very very very very very very very very very very very very very "
             "very very very very very very very very very very very very very "
             "very very very very very very very very very long"};

  int count = 5;
  Case cases[count];
  cases[0] = both_empty;
  cases[1] = label_empty;
  cases[2] = msg_empty;
  cases[3] = both_short;
  cases[4] = both_long;

  pthread_t thread;
  pthread_create(&threads, NULL, get_actuals, (void *)(&count));

  for (int i = 0; i < count; ++i) {
    char cmd[1024];
    memset(cmd, '\0', sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "./beep -l %s -m %s", cases[i].label,
             cases[i].msg);
    if (system(cmd) == -1) {
      fprintf(stderr, "failed to execute %d-th case\n", i);
      return 1;
    }
  }

  beep_buf *actuals;
  pthread_join(thread, &actuals);

  // for (int i=0; i<count; ++i) {
  //  beep_buf expected = mk_expect(cases[i])
}