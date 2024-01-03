// ┌────────────────────────┐
// │IPC Publish-Subscribe   │
// ├────────────────────────┤
// │Klient                  │
// ├────────────────────────┤
// │Tomasz Pawłowski 155965 │
// │Jakub Kamieniarz 155845 │
// └────────────────────────┘

#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "inf155965_155845_message_types.h"

int main() {
  int mid = msgget(0x123, 0600 | IPC_CREAT);

  SubscriptionMessage m2;
  
  msgrcv(mid, &m2, sizeof m2, Subscription, 0);
  printf("%s\n", m2.name);
  printf("%d\n",m2.topic_id);
  printf("%d\n",m2.duration);
  msgctl(mid, IPC_RMID, NULL);
}
