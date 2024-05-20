#include <stdio.h>

#include "beep.h"
#include "socks.h"

int main(void) {
  int sockfd = server_setup();
  if (sockfd == -1) {
    return 1;
  }

  printf("sockfd: %d\n", sockfd);
  uint8_t *buf = server_handle(sockfd);

  fprintf(stderr, "beepd: %s\n", buf);

  Beep *beep1 = unmarshal_beep(buf);

  Beep *beep2 = unmarshal_beep(buf);
  strncpy(beep2->label, "hap", 3);

  Beep *beep3 = unmarshal_beep(buf);
  strncpy(beep3->label, "lal", 3);

  Beep *beep4 = unmarshal_beep(buf);
  strncpy(beep4->label, "gag", 3);

  Beep *beep5 = unmarshal_beep(buf);
  strncpy(beep5->label, "aaa", 3);

  free_buf(buf);

  // print_beep(beep);

  Beeps arr = init_array();
  arr = push_to_array(arr, beep1);
  arr = push_to_array(arr, beep2);
  arr = push_to_array(arr, beep3);
  arr = push_to_array(arr, beep4);
  arr = push_to_array(arr, beep5);
  print_array(arr);

  Beep *bp = find_from_array(arr, beep2->label);
  print_beep(bp);
  rm_from_array(arr, beep2);

exit:
  print_array(arr);
  free_array(arr);

  /*
  BeepQueue *queue = NULL;
  queue = queue_push(queue, beep);
  queue = queue_push(queue, beep);
  queue = queue_push(queue, beep);
  print_queue(queue);
  print_beep(queue_head(queue));
  queue = queue_pop(queue);
  print_queue(queue);
  */

  close(sockfd);
}