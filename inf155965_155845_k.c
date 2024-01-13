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

LoginMessage m_login = {.type = Login};
LoginStatus m_login_status = {.type = Login};
SubscriptionMessage m_subscription = {.type = Subscription};
NewTopicMessage m_new_topic = {.type = NewTopic};
NewTopicStatus m_topic_status = {.type = NewTopic};
Message m_text = {.type=SendMessage};
BlockUserMessage m_block_user = {.type=BlockUser};

int main() {

  key_t server_key; 
  printf("Podaj klucz kolejki serwera: \n");
  scanf("%X", &server_key);
  int server_queue = msgget(server_key, 0600 | IPC_CREAT);

  char username[128];
  printf("Podaj nazwę użytkownika: \n");
  scanf("%s", username);

  key_t user_queue_key = 0x123;

  m_login.queue_key=user_queue_key;
  strcpy(m_login.name,username);
  msgsnd(server_queue, &m_login, sizeof(m_login)-sizeof(long), 0);
  printf("Wysłano wiadomość logowania.\n");

  int client_queue = msgget(user_queue_key, 0600 | IPC_CREAT);
  msgrcv(client_queue, &m_login_status, sizeof(m_login_status)-sizeof(long), Login, 0);
  if (m_login_status.status){
    printf("Zalogowano jako:\n");
    printf("Name: %s, ID: %d, Queue: %X\n", m_login_status.name, m_login_status.id, user_queue_key);
  } else {
    printf("Logowanie nieudane:\n");
    printf("Name: %s, ID: %d, Queue: %X\n", m_login_status.name, m_login_status.id, user_queue_key);
  }

  printf("Any key to exit.\n");
  scanf("%*c"); // wartość ignorowana

  msgctl(client_queue, IPC_RMID, NULL);
  msgctl(server_queue, IPC_RMID, NULL);
}
