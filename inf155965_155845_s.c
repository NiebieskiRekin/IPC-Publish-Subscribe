// ┌────────────────────────┐
// │IPC Publish-Subscribe   │
// ├────────────────────────┤
// │Serwer                  │
// ├────────────────────────┤
// │Tomasz Pawłowski 155965 │
// │Jakub Kamieniarz 155845 │
// └────────────────────────┘

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "inf155965_155845_message_types.h"

SubscriptionMessage m2 = {Subscription, "myname", 1, Permanent, 0};

SubscriptionMessage m_sub_perm = {Subscription, "...", 123,Permanent};

int main() {
  // Globalna kolejka serwera, key kolejki wyświetlany na ekranie przez proces serwera
  key_t server_key = 0x123;
  int mid = msgget(server_key, 0600 | IPC_CREAT);

  while (1){
    // oczekuj na wiadomości
  }
  // msgsnd(mid, &m2, sizeof m2, IPC_NOWAIT);
  // msgsnd(mid, &m2, sizeof m2, IPC_NOWAIT);
}

