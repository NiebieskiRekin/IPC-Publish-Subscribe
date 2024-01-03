// ┌────────────────────────┐
// │IPC Publish-Subscribe   │
// ├────────────────────────┤
// │Serwer                  │
// ├────────────────────────┤
// │Tomasz Pawłowski 155965 │
// │Jakub Kamieniarz 155845 │
// └────────────────────────┘

#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "inf155965_155845_message_types.h"

AvailableTopicsMessage m1 = {.type = AvailableTopics,
                             .topics = {
                                 {1, "Hello, World!"},
                                 {2, "Merry Chistmas!"},
                             }};

SubscriptionMessage m2 = {Subscription, "myname", 1, Permanent, 0};

SubscriptionMessage m_sub_perm = {Subscription, "...", 123,Permanent};

int main() {
  int mid = msgget(0x123, 0600 | IPC_CREAT);
  msgsnd(mid, &m2, sizeof m2, IPC_NOWAIT);
  msgsnd(mid, &m2, sizeof m2, IPC_NOWAIT);
}

