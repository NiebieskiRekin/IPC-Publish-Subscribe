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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "inf155965_155845_message_types.h"

int main() {

  key_t server_key; // key_t = int
  printf("Podaj klucz kolejki serwera: ");
  // TODO zapytanie o klucz  
  int mid = msgget(server_key, 0600 | IPC_CREAT);

  LoginMessage m;  
  msgrcv(mid, &m2, sizeof m2, Subscription, 0);
  printf("%s\n", m2.name);
  printf("%d\n",m2.topic_id);
  printf("%d\n",m2.duration);

  msgctl(mid, IPC_RMID, NULL);
}
