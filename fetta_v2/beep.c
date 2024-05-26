#include <stdio.h>
#include <string.h>

#include "beep.h"
#include "socks.h"

int main(int argc, char **argv) {
  int sockfd = client_setup();
  if (sockfd == -1) {
    return 1;
  }
  printf("sockfd: %d\n", sockfd);

  uint8_t *msg;
  uint8_t *label;
  if (argc == 3) {
    msg = (uint8_t *)argv[2];
    label = (uint8_t *)argv[1];
  } else {
    msg = (uint8_t *)"default";
    label = (uint8_t *)"default";
  }
  Beep beep = {.version_major = version_major,
               .version_minor = version_minor,
               .label = label, // TODO: CHANGEME:
               .msg = msg};
  beep.label_len = strlen(beep.label);
  beep.msg_len = strlen(beep.msg);

  print_beep(&beep);

  uint8_t *data = marshal_beep(&beep);
  printf("msg: %s\n", data);

  client_handle(sockfd, data);

  free(data);
  close(sockfd);
  return 0;
}