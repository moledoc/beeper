#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SOCK_CONST
#define SOCK_IMPLEMENTATION
#include "sock.h"

#define BEEP_IMPLEMENTATION
#include "beep.h"

typedef struct {
  char *name;
  char *help;
  char *quit;
  char *label;
  char *msg;
  int expected_status_code;
} Case;

const char *expected_help_str = "TODO:\n";

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

int is_equal(beep_buf expected, beep_buf actual) {
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

void *get_actual(void *_) {
  beep_buf actual = NULL;
  int sock_fd;

  sock_fd = fd();
  if (sock_fd == -1) {
    fprintf(stderr, "%s\n", strerror(errno));
    goto exit;
  }
  unlink(__sock_path);
  struct sockaddr_un *addr = get_addr(__sock_path);
  if (bind_fd(sock_fd, addr) == -1) {
    fprintf(stderr, "%s\n", strerror(errno));
    goto exit;
  }
  if (listen(sock_fd, 1) == -1) {
    fprintf(stderr, "%s\n", strerror(errno));
    goto exit;
  }

  socklen_t addr_len = sizeof(*addr);
  {
    int conn;
    if ((conn = accept(sock_fd, (struct sockaddr *)addr, &addr_len)) == -1) {
      fprintf(stderr, "%s\n", strerror(errno));
      goto exit;
    }
    actual = calloc(1024, sizeof(char));
    read(conn, actual, sizeof(actual));
    close(conn);
  }

exit:
  close(sock_fd);
  return (void *)actual;
}

int main(int argc, char **argv) {
  if (precond() == -1) {
    fprintf(stderr, "beep.c testing precondition failed\n");
    fprintf(stderr, "%s\n", strerror(errno));
    return 1;
  }

  Case help_short = {.name = "help_short",
                     .help = "-h",
                     .quit = "",
                     .label = "",
                     .msg = "",
                     .expected_status_code = 0};
  Case help_long = {.name = "help_long",
                    .help = "--help",
                    .quit = "",
                    .label = "",
                    .msg = "",
                    .expected_status_code = 0};
  Case help_help = {.name = "help_help",
                    .help = "help",
                    .quit = "",
                    .label = "",
                    .msg = "",
                    .expected_status_code = 0};

  Case quit_short = {.name = "quit_short",
                     .help = "",
                     .quit = "-q",
                     .label = "",
                     .msg = "",
                     .expected_status_code = 0};
  Case quit_long = {.name = "quit_long",
                    .help = "",
                    .quit = "--quit",
                    .label = "",
                    .msg = "",
                    .expected_status_code = 0};
  Case quit_quit = {.name = "quit_quit",
                    .help = "",
                    .quit = "quit",
                    .label = "",
                    .msg = "",
                    .expected_status_code = 0};

  Case both_empty = {.name = "both_empty",
                     .help = "",
                     .quit = "",
                     .label = "",
                     .msg = "",
                     .expected_status_code = 1};
  Case label_empty = {.name = "label_empty",
                      .help = "",
                      .quit = "",
                      .label = "",
                      .msg = "\"not empty\"",
                      .expected_status_code = 1};
  Case msg_empty = {.name = "msg_empty",
                    .help = "",
                    .quit = "",
                    .label = "\"not empty\"",
                    .msg = "",
                    .expected_status_code = 1};

  Case both_empty_str = {.name = "both_empty_str",
                         .help = "",
                         .quit = "",
                         .label = "\"\"",
                         .msg = "\"\"",
                         .expected_status_code = 1};
  Case label_empty_str = {.name = "label_empty_str",
                          .help = "",
                          .quit = "",
                          .label = "\"\"",
                          .msg = "\"not empty\"",
                          .expected_status_code = 1};
  Case msg_empty_str = {.name = "msg_empty_str",
                        .help = "",
                        .quit = "",
                        .label = "\"not empty\"",
                        .msg = "\"\"",
                        .expected_status_code = 1};

  Case both_short = {.name = "both_short",
                     .help = "",
                     .quit = "",
                     .label = "\"short\"",
                     .msg = "\"short message\"",
                     .expected_status_code = 0};
  Case both_long = {
      .name = "both_long",
      .help = "",
      .quit = "",
      .label = "\"very very very very very very very very very very very very "
               "very very very very very very very very very very very very "
               "very very very very very very very very very very very long\"",
      .msg =
          "\"very very very very very very very very very very very very very "
          "very very very very very very very very very very very very very "
          "very very very very very very very very very long\"",
      .expected_status_code = 0};

  int count = 14;
  Case cases[count];
  cases[0] = help_short;
  cases[1] = help_long;
  cases[2] = help_help;

  cases[3] = quit_short;
  cases[4] = quit_long;
  cases[5] = quit_quit;

  cases[6] = both_empty;
  cases[7] = label_empty;
  cases[8] = msg_empty;

  cases[9] = both_empty_str;
  cases[10] = label_empty_str;
  cases[11] = msg_empty_str;

  cases[12] = both_short;
  cases[13] = both_long;

  char *state = NULL;
  for (int i = 0; i < 3; ++i) {
    char cmd[1024];
    memset(cmd, '\0', sizeof(cmd));

    if (strcmp("", cases[i].help) != 0) {
      char *outfile = "/tmp/beep.c.test";
      FILE *fptr = fopen(outfile, "w");
      fclose(fptr);

      fptr = fopen(outfile, "r+");
      if (fptr == NULL) {
        fprintf(stderr, "opening file for writing failed: %s\n",
                strerror(errno));
        exit(1);
      }
      int read_fd = fileno(fptr);
      int write_fd = dup(read_fd);

      char *recomp_cmd = calloc(128, sizeof(char));
      if (recomp_cmd == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        fclose(fptr);
        return errno;
      }
      snprintf(recomp_cmd, 128, "cc -o beep beep.c -DTESTING=%d", write_fd);
      if (system(recomp_cmd) == -1) {
        fprintf(stderr, "failed to execute %d-th case: %s\n", i,
                strerror(errno));
        fclose(fptr);
        return 1;
      }
      free(recomp_cmd);

      snprintf(cmd, sizeof(cmd), "./beep %s", cases[i].help);
      int status_code = system(cmd);
      if (status_code == -1) {
        fprintf(stderr, "failed to execute %d-th case: %s\n", i,
                strerror(errno));
        fclose(fptr);
        return 1;
      }

      char *actual = NULL;
      size_t actual_size = 1024;
      actual = calloc(actual_size, sizeof(char));
      lseek(read_fd, 0, SEEK_SET);
      read(read_fd, actual, actual_size);
      fclose(fptr);

      bool b = status_code == cases[i].expected_status_code &&
               strcmp(expected_help_str, actual) == 0;
      state = b ? "PASS" : "FAIL";
      if (!b) {
        fprintf(stderr,
                "stderr difference:"
                "\n\texpected\n\t\t- '%s'"
                "\n\tactual\n\t\t- '%s'"
                "\n",
                expected_help_str, actual);
      }
      if (actual != NULL) {
        free(actual);
      }
    } else {
      pthread_t thread;
      pthread_create(&thread, NULL, get_actual, NULL);

      if (strcmp("", cases[i].quit) != 0) {
        snprintf(cmd, sizeof(cmd), "./beep %s", cases[i].quit);
      } else {
        snprintf(cmd, sizeof(cmd), "./beep -l %s -m %s", cases[i].label,
                 cases[i].msg);
      }
      int status_code = system(cmd);
      if (status_code == -1) {
        fprintf(stderr, "failed to execute %d-th case: %s\n", i,
                strerror(errno));
        return 1;
      }

      beep_buf actual = NULL;
      pthread_join(thread, (void **)&actual);
      beep_buf expected = mk_expect(cases[i]);
      bool b = status_code == cases[i].expected_status_code &&
               is_equal(expected, actual);
      state = b ? "PASS" : "FAIL";
      fprintf(stderr, "-- %s: %s\n", state, cases[i].name);
      if (actual != NULL) {
        free(actual);
      }
    }
    fprintf(stderr, "-- %s: %s\n", state, cases[i].name);
  }
}