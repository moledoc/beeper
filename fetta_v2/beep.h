#pragma once

#ifndef BEEP_H_
#define BEEP_H_

#include <stdint.h>

typedef struct {
  uint8_t version_major;
  uint8_t version_minor;
  uint8_t label_len;
  unsigned char *label;
  uint8_t msg_len;
  unsigned char *msg;
} Beep;

enum metadata {
  version_minor_idx,
  version_major_idx,
  msg_start_idx,
  metadata_size,
};

void free_beep(Beep *beep);
unsigned char *beep2str(Beep *beep); // allocs memory
void print_beep(Beep *beep);

uint8_t *marshal_beep(Beep *beep);  // allocs memory
Beep *unmarshal_beep(uint8_t *buf); // allocs memory

#endif // BEEP_H_

#ifndef BEEP_VARS
#define BEEP_VARS

uint8_t version_major = '0';
uint8_t version_minor = '1';

#endif // BEEP_VARS

#ifndef BEEP_UTILS
#define BEEP_UTILS

int valid_version_buf(uint8_t *buf);
int valid_version_beep(Beep *beep);

int valid_version_buf(uint8_t *buf) {
  return buf != NULL && buf[version_major_idx] == version_major &&
         buf[version_minor_idx] == version_minor;
}

int valid_version_beep(Beep *beep) {
  return beep != NULL && beep->version_major == version_major &&
         beep->version_minor == version_minor;
}

#endif // SOCK_UTILS

#ifndef BEEP_IMPLEMENTATION
#define BEEP_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>

void free_buf(uint8_t *buf) {
  if (buf != NULL) {
    free(buf);
  }
}

void free_beep(Beep *beep) {
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

unsigned char *beep2str(Beep *beep) { // allocs memory
  if (beep == NULL) {
    return NULL;
  }
  size_t str_len =
      (128 + beep->label_len + beep->msg_len) * sizeof(unsigned char);
  unsigned char *str = malloc(str_len);
  memset(str, '\0', sizeof(str));
  snprintf(str, str_len,
           "{"
           "\n\t.version_major=%d"
           "\n\t.version_minor=%d"
           "\n\t.label=(%d)'%s'"
           "\n\t.msg=(%d)'%s'"
           "\n}",
           beep->version_major - '0', beep->version_minor - '0',
           beep->label_len, beep->label, beep->msg_len, beep->msg);
  return str;
}

void print_beep(Beep *beep) {
  if (beep == NULL) {
    fprintf(stderr, "{NULL}\n");
    return;
  }
  unsigned char *str = beep2str(beep);
  fprintf(stderr, "%p: %s\n", beep, str);
  free(str);
}

uint8_t *marshal_beep(Beep *beep) { // allocs memory
  if (beep == NULL || !valid_version_beep(beep)) {
    return NULL;
  }
  uint8_t *buf =
      calloc(sizeof(Beep) + beep->label_len + beep->msg_len, sizeof(uint8_t));
  buf[version_major_idx] = beep->version_major;
  buf[version_minor_idx] = beep->version_minor;
  buf[msg_start_idx] = beep->label_len + '0';
  strncpy(buf + msg_start_idx + 1, beep->label, beep->label_len);
  buf[msg_start_idx + beep->label_len + 1] = beep->msg_len + '0';
  strncpy(buf + msg_start_idx + 1 + beep->label_len + 1, beep->msg,
          beep->msg_len);
  return buf;
}

Beep *unmarshal_beep(uint8_t *buf) { // allocs memory
  if (buf == NULL || !valid_version_buf(buf)) {
    return NULL;
  }

  Beep *beep = calloc(1, sizeof(Beep));

  beep->version_major = buf[version_major_idx];
  beep->version_minor = buf[version_minor_idx];
  beep->label_len = buf[msg_start_idx] - '0';
  beep->label = calloc(beep->label_len + 1, sizeof(unsigned char));
  strncpy(beep->label, buf + msg_start_idx + 1, beep->label_len);

  beep->msg_len = buf[msg_start_idx + 1 + beep->label_len] - '0';
  beep->msg = calloc(beep->msg_len + 1, sizeof(unsigned char));
  strncpy(beep->msg, buf + msg_start_idx + 1 + beep->label_len + 1,
          beep->msg_len);
  return beep;
}

#endif // BEEP_IMPLEMENTATION

#ifndef BEEP_ARRAY_H_
#define BEEP_ARRAY_H_
#include <stdbool.h>
#include <stdint.h>

typedef Beep **Beeps;

bool graceful_shutdown(bool shutdown);

Beeps init_array(); // allocs memory
void free_array(Beeps beeps);
void print_array(Beeps beeps);

uint16_t beeps_count(Beeps beeps);
Beeps push_to_array(Beeps beeps, Beep *beep);
void rm_from_array(Beeps beeps, Beep *beep);
void mv_to_end(Beeps beeps, Beep *beep);
Beep *find_from_array(Beeps beeps, char *label);
Beep *head_of_array(Beeps beeps);

#endif // BEEP_ARRAY_H_

#ifndef BEEP_ARRAY_IMPLEMENTATION
#define BEEP_ARRAY_IMPLEMENTATION
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint16_t beeps_size = (uint16_t)(5 * sizeof(Beep *));
pthread_mutex_t Beeps_mutex = PTHREAD_MUTEX_INITIALIZER;

bool shutting_down = false;
pthread_mutex_t graceful_mutex = PTHREAD_MUTEX_INITIALIZER;

Beeps init_array() {
  Beeps beeps = malloc(beeps_size);
  memset(beeps, 0, sizeof(beeps));
  return beeps;
}

bool graceful_shutdown(bool shutdown) {
  pthread_mutex_lock(&graceful_mutex);
  if (shutdown) {
    shutting_down = shutdown;
  }
  pthread_mutex_unlock(&graceful_mutex);
  return shutting_down;
}

void free_array(Beeps beeps) {
  pthread_mutex_lock(&Beeps_mutex);
  if (beeps == NULL) {
    goto exit;
  }
  Beeps cur;
  for (cur = beeps; cur != NULL && (*cur) != NULL; ++cur) {
    free_beep(*cur);
  }
  free(beeps);
exit:
  pthread_mutex_unlock(&Beeps_mutex);
  return;
}

void print_array(Beeps beeps) {
  pthread_mutex_lock(&Beeps_mutex);
  fprintf(stderr, "[DEBUG] beeps %p\n", beeps);
  if (beeps == NULL) {
    goto exit;
  }
  for (; (*beeps) != NULL; ++beeps) {
    fprintf(stderr, "-> ");
    print_beep(*beeps);
  }
  fprintf(stderr, "-> ");
  print_beep(NULL);
exit:
  pthread_mutex_unlock(&Beeps_mutex);
  return;
}

uint16_t beeps_count(Beeps beeps) {
  uint16_t count = 0;
  pthread_mutex_lock(&Beeps_mutex);
  if (beeps == NULL || *beeps == NULL) {
    goto exit;
  }
  for (; beeps != NULL && *beeps != NULL; ++beeps, ++count) {
    ;
  }
exit:
  pthread_mutex_unlock(&Beeps_mutex);
  return count;
}

Beeps push_to_array(Beeps beeps, Beep *beep) {
  pthread_mutex_lock(&Beeps_mutex);
  Beeps start = beeps;
  if (beeps == NULL) {
    // MAYBE: TODO: handle better
    goto exit;
  }
  uint16_t i = 0;
  for (; i < beeps_size && beeps != NULL && (*beeps) != NULL; ++beeps, ++i) {
    if ((*beeps) == beep || strcmp(beep->label, (*beeps)->label) == 0) {
      fprintf(stderr, "replacing %p with %p (label: '%s'), msg: '%s'\n", *beeps,
              beep, beep->label, beep->msg);
      break;
    }
  }
  if (i >= beeps_size / sizeof(Beep *) || beeps == NULL) {
    beeps_size *= 2;
    fprintf(stderr, "beeps size %d not enough, doubling; new size: %d\n",
            beeps_size / 2 / sizeof(Beep *), beeps_size / sizeof(Beep *));
    start = realloc(start, beeps_size);
    assert(start != NULL && "realloc failed, panic!");
    memset(start + i, 0, beeps_size - (i * sizeof(Beep *)));
  }
  (*(start + i)) = beep;
exit:
  pthread_mutex_unlock(&Beeps_mutex);
  return start;
}

void rm_from_array(Beeps beeps, Beep *beep) {
  pthread_mutex_lock(&Beeps_mutex);
  if (beeps == NULL || beep == NULL) {
    goto exit;
  }
  int i = 0;
  for (; beeps[i] != NULL; ++i) {
    if (beeps[i] == beep || strcmp(beeps[i]->label, beep->label) == 0) {
      break;
    }
  }
  fprintf(stderr, "removing %p (label: '%s') msg: '%s'\n", beeps[i],
          beeps[i]->label, beeps[i]->msg);
  free_beep(beeps[i]);
  beeps[i] = NULL;
  for (int j = i + 1; beeps[j] != NULL; ++j) {
    beeps[j - 1] = beeps[j];
    beeps[j] = NULL;
  }
exit:
  pthread_mutex_unlock(&Beeps_mutex);
  return;
}

void mv_to_end(Beeps beeps, Beep *beep) {
  pthread_mutex_lock(&Beeps_mutex);
  if (beeps == NULL || beep == NULL) {
    goto exit;
  }
  int i = 0;
  for (; beeps[i] != NULL; ++i) {
    if (beeps[i] == beep || strcmp(beeps[i]->label, beep->label) == 0) {
      break;
    }
  }
  fprintf(stderr, "moving to end %p (label: '%s') msg: '%s'\n", beeps[i],
          beeps[i]->label, beeps[i]->msg);
  Beep *new_last = beeps[i];
  beeps[i] = NULL;
  int j = i + 1;
  for (; beeps[j] != NULL; ++j) {
    beeps[j - 1] = beeps[j];
    beeps[j] = NULL;
  }
  beeps[j - 1] = new_last;
exit:
  pthread_mutex_unlock(&Beeps_mutex);
  return;
}

Beep *find_from_array(Beeps beeps, char *label) {
  pthread_mutex_lock(&Beeps_mutex);
  Beep *ret = NULL;
  if (beeps == NULL) {
    // MAYBE: TODO: handle better
    goto exit;
  }

  for (int i = 0; beeps != NULL && (*beeps) != NULL; ++beeps, ++i) {
    if (strcmp(label, (*beeps)->label) == 0) {
      ret = *beeps;
      goto exit;
    }
  }
exit:
  pthread_mutex_unlock(&Beeps_mutex);
  return ret;
}

Beep *head_of_array(Beeps beeps) {
  pthread_mutex_lock(&Beeps_mutex);
  Beep *ret = beeps == NULL ? NULL : *beeps;
  pthread_mutex_unlock(&Beeps_mutex);
  return ret;
}

#endif // BEEP_ARRAY_IMPLEMENTATION

#ifndef BEEP_QUEUE_H_
#define BEEP_QUEUE_H_

typedef struct BeepQueue {
  struct BeepQueue *next;
  Beep *beep;
} BeepQueue;

void free_entire_queue(BeepQueue *queue);
void print_queue(BeepQueue *queue);

BeepQueue *queue_push(BeepQueue *queue, Beep *beep);
BeepQueue *queue_pop(BeepQueue *queue);
Beep *queue_head(BeepQueue *queue);

#endif // BEEP_QUEUE_H_

#ifndef BEEP_QUEUE_IMPLEMENTATION
#define BEEP_QUEUE_IMPLEMENTATION
#include <stdlib.h>

void free_entire_queue(BeepQueue *queue) {
  for (; queue != NULL;) {
    BeepQueue *me = queue;
    queue = queue->next;
    free_beep(queue->beep);
    free(me);
  }
}

void print_queue(BeepQueue *queue) {
  fprintf(stderr, "QUEUE: ");
  for (; queue != NULL; queue = queue->next) {
    fprintf(stderr, "-> ");
    print_beep(queue->beep);
  }
  fprintf(stderr, "-> ");
  print_beep(NULL);
}

BeepQueue *queue_push(BeepQueue *queue, Beep *beep) {
  BeepQueue *next = malloc(1 * sizeof(BeepQueue));
  next->beep = beep;
  next->next = NULL;
  if (queue == NULL) {
    return next;
  }
  BeepQueue *cur = queue;
  for (;; cur = cur->next) {
    if (cur->beep == beep) {
      fprintf(stderr, "ignoring already queued beep %p\n", beep);
      free(next);
      goto exit;
    }
    if (cur->next == NULL) {
      break;
    }
  }

  cur->next = next;
exit:
  return queue;
}

BeepQueue *queue_pop(BeepQueue *queue) {
  if (queue == NULL) {
    return NULL;
  }
  BeepQueue *me = queue;
  queue = queue->next;
  // free_queue_node(me);
  free_beep(me->beep);
  free(me);
  return queue;
}

Beep *queue_head(BeepQueue *queue) {
  if (queue == NULL) {
    return NULL;
  }
  return queue->beep;
}

#endif // BEEP_QUEUE_IMPLEMENTATION