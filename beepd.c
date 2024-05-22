#include <pthread.h>
#include <stdio.h>

#include "beep.h"
#include "draw.h"
#include "socks.h"

int main(void) {
  int sockfd = server_setup();
  if (sockfd == -1) {
    return 1;
  }

  Beeps arr = init_array();

  size_t threads_count = 1;
  pthread_t threads[threads_count];

  pthread_create(&threads[0], NULL, draw, (void *)(&arr));

  printf("sockfd: %d\n", sockfd);
  fprintf(stderr, "[DEBUG]: beeps %p\n", arr);
  for (;;) {
    uint8_t *buf = server_handle(sockfd);

    fprintf(stderr, "beepd: %s\n", buf);

    Beep *beep = unmarshal_beep(buf);
    if (strcmp(beep->msg, "q") == 0) {
      break;
    }
    arr = push_to_array(arr, beep);
    print_array(arr);
  }

  for (int i = 0; i < threads_count; ++i) {
    pthread_join(threads[i], NULL);
  }

  //   Beep *beep2 = unmarshal_beep(buf);
  //   strncpy(beep2->label, "hap", 3);
  //   strncpy(beep2->msg, "tereeee", 7);
  //
  //   Beep *beep3 = unmarshal_beep(buf);
  //   strncpy(beep3->label, "lal", 3);
  //   strncpy(beep3->msg, "hahahah", 7);
  //
  //   Beep *beep4 = unmarshal_beep(buf);
  //   strncpy(beep4->label, "gag", 3);
  //   strncpy(beep4->msg, "gisldks", 7);
  //
  //   Beep *beep5 = unmarshal_beep(buf);
  //   strncpy(beep5->label, "aaa", 3);
  //   strncpy(beep5->msg, "aaaaaaa", 7);
  //
  //   free_buf(buf);
  //
  //   // print_beep(beep);
  //
  //
  //   arr = push_to_array(arr, beep1);
  //   arr = push_to_array(arr, beep2);
  //   arr = push_to_array(arr, beep3);
  //   arr = push_to_array(arr, beep4);
  //   arr = push_to_array(arr, beep5);
  //   print_array(arr);
  //
  //   //
  //     Beep *bp = find_from_array(arr, beep2->label);
  //     print_beep(bp);
  //     rm_from_array(arr, beep2);
  //   //
  //
  //   draw(arr);
  //
  // exit:
  //   print_array(arr);
  //   free_array(arr);
  //
  //   //
  //   BeepQueue *queue = NULL;
  //   queue = queue_push(queue, beep);
  //   queue = queue_push(queue, beep);
  //   queue = queue_push(queue, beep);
  //   print_queue(queue);
  //   print_beep(queue_head(queue));
  //   queue = queue_pop(queue);
  //   print_queue(queue);
  //   //

  free_array(arr);
  close(sockfd);
}