#pragma once

#ifdef SOCK_CONST

const int __version_major = 0;
const int __version_minor = 1;

#endif // SOCK_CONST

#ifndef BEEP_H_
#define BEEP_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum beep_metadata {
  version_minor_idx,
  version_major_idx,
  msg_start_idx,
  metadata_size,
};

typedef struct {
  int version_major;
  int version_minor;
  size_t label_len;
  char *label;
  size_t msg_len;
  char *msg;
} Beep;

int valid_version_beep(Beep *beep);

Beep *mk_beep(const char *label, const char *msg); // allocs memory
void beep_free(Beep *beep);                        // free memory

char *beep_to_str(Beep *beep); // allocs memory
void beep_print(Beep *beep);

char *marshal(Beep *beep); // allocs memory

typedef char *beep_buf;

int valid_version_buf(const beep_buf buf);
void buf_print(const beep_buf buf);
size_t buf_len(const beep_buf buf);
char *buf_to_str(const beep_buf buf); // allocs memory
Beep *unmarshal(const beep_buf buf);  // allocs memory

#endif // BEEP_H_

#ifdef BEEP_IMPLEMENTATION

int valid_version_beep(Beep *beep) {
  return beep != NULL && beep->version_major == __version_major &&
         beep->version_minor == __version_minor;
}

Beep *mk_beep(const char *label, const char *msg) {
  assert(metadata_size == 3 && "making beep needs extending");

  Beep *beep = calloc(1, sizeof(Beep));

  beep->version_major = __version_major;
  beep->version_minor = __version_minor;

  beep->label_len = strlen(label);
  beep->label = calloc(beep->label_len, sizeof(char));
  memcpy(beep->label, label, beep->label_len);

  beep->msg_len = strlen(msg);
  beep->msg = calloc(beep->msg_len, sizeof(char));
  memcpy(beep->msg, msg, beep->msg_len);

  return beep;
}

void beep_free(Beep *beep) {
  assert(metadata_size == 3 && "freeing beep needs extending");

  if (beep == NULL) {
    return;
  }
  if (beep->label != NULL) {
    free(beep->label);
  }
  if (beep->msg != NULL) {
    free(beep->msg);
  }
  free(beep);
}

char *beep_to_str(Beep *beep) {
  assert(metadata_size == 3 && "beep to string needs extending");

  if (beep == NULL) {
    return NULL;
  }
  size_t extra_chars =
      128; // account for extra chars, such as '{', '\n', '\t' ....
  size_t str_len = beep->label_len + beep->msg_len + extra_chars;
  char *str = calloc(str_len, sizeof(char));
  snprintf(str, str_len,
           "{"
           "\n\t.version_major=%d"
           "\n\t.version_minor=%d"
           "\n\t.label=(%d)'%s'"
           "\n\t.msg=(%d)'%s'"
           "\n}",
           beep->version_major, beep->version_minor, beep->label_len,
           beep->label, beep->msg_len, beep->msg);
  return str;
}

void beep_print(Beep *beep) {
  if (beep == NULL) {
    fprintf(stderr, "{NULL}\n");
    return;
  }
  char *str = beep_to_str(beep);
  fprintf(stderr, "%p: %s\n", beep, str);
  free(str);
}

char *marshal(Beep *beep) {
  assert(metadata_size == 3 && "marshalling beep needs extending");

  if (beep == NULL || !valid_version_beep(beep)) {
    return NULL;
  }
  char *buf =
      calloc(sizeof(Beep) + beep->label_len + beep->msg_len, sizeof(char));
  buf[version_major_idx] = beep->version_major;
  buf[version_minor_idx] = beep->version_minor;
  buf[msg_start_idx] = beep->label_len + '0';
  strncpy(buf + msg_start_idx + 1, beep->label, beep->label_len);
  buf[msg_start_idx + beep->label_len + 1] = beep->msg_len + '0';
  strncpy(buf + msg_start_idx + 1 + beep->label_len + 1, beep->msg,
          beep->msg_len);
  return buf;
}

Beep *unmarshal(const beep_buf buf) {
  assert(metadata_size == 3 && "unmarshalling beep needs extending");

  if (buf == NULL || !valid_version_buf(buf)) {
    return NULL;
  }

  Beep *beep = calloc(1, sizeof(Beep));

  beep->version_major = buf[version_major_idx];
  beep->version_minor = buf[version_minor_idx];

  beep->label_len = buf[msg_start_idx];
  beep->label = calloc(beep->label_len + 1, sizeof(char));
  strncpy(beep->label, buf + msg_start_idx + 1, beep->label_len);

  beep->msg_len = buf[msg_start_idx + 1 + beep->label_len];
  beep->msg = calloc(beep->msg_len + 1, sizeof(char));
  strncpy(beep->msg, buf + msg_start_idx + 1 + beep->label_len + 1,
          beep->msg_len);
  return beep;
}

int valid_version_buf(const beep_buf buf) {
  return buf != NULL && buf[version_major_idx] == __version_major &&
         buf[version_minor_idx] == __version_minor;
}

void buf_print(beep_buf buf) {
  assert(metadata_size == 3 && "printing buf needs extending");

  char *str = buf_to_str(buf);
  fprintf(stderr, "%s\n", str);
  if (str != NULL) {
    free(str);
  }
}

size_t buf_len(beep_buf buf) {
  assert(metadata_size == 3 && "finding buf len needs extending");

  int version_len = msg_start_idx;
  int label_len = buf[msg_start_idx] + 1; // include the label_len into the var
  int msg_len =
      buf[msg_start_idx + label_len] + 1; // include the msg_len into the var

  return version_len + label_len + msg_len;
}

char *buf_to_str(const beep_buf buf) {
  assert(metadata_size == 3 && "buf to string needs extending");

  int ver_major = buf[version_major_idx];
  int ver_minor = buf[version_minor_idx];

  int label_len = buf[msg_start_idx];
  char label[label_len + 1];
  memset(label, '\0', sizeof(label));
  memcpy(label, buf + msg_start_idx + 1, label_len);

  int msg_len = buf[msg_start_idx + label_len];
  char msg[msg_len + 1];
  memset(msg, '\0', sizeof(msg));
  memcpy(msg, buf + msg_start_idx + label_len + 1, msg_len);

  size_t str_len = msg_start_idx + label_len + msg_len + 2;
  char *str = calloc(str_len, sizeof(char));
  snprintf(str, str_len, "%d%d%d%s%d%s", ver_major, ver_minor, label_len, label,
           msg_len, msg);
  return str;
}

#endif // BEEP_IMPLEMENTATION