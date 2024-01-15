// ┌────────────────────────┐
// │IPC Publish-Subscribe   │
// ├────────────────────────┤
// │Klient                  │
// ├────────────────────────┤
// │Tomasz Pawłowski 155965 │
// │Jakub Kamieniarz 155845 │
// └────────────────────────┘

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "inf155965_155845_message_types.h"

LoginMessage m_login = {.type = Login};
LoginStatus m_login_status = {.type = Login, .status=0};
SubscriptionMessage m_subscription = {.type = Subscription};
NewTopicMessage m_new_topic = {.type = NewTopic};
NewTopicStatus m_topic_status = {.type = NewTopic};
Message m_text = {.type=SendMessage};
BlockUserMessage m_block_user = {.type=BlockUser};
ReadMessage m_read = {.type=ReadMessages};

int client_queue;
int client_id;
char username[128];

void clean_exit(void){
  msgctl(client_queue, IPC_RMID, NULL);
  exit(0);
}

int main(void) {

  key_t server_key; 
  printf("Podaj klucz kolejki serwera: \n");
  printf("0x");
  unsigned int temp;
  scanf("%x", &temp);
  getchar(); // Usuń znak \n
  server_key = (int)temp;
  int server_queue = msgget(server_key, 0600 | IPC_CREAT);

  while (m_login_status.status == 0){
    printf("Podaj nazwę użytkownika: \n");
    scanf("%127[^\n]",username); // odczyt max 127 znaków do nowej linii
    getchar(); // Usuń znak \n
    key_t user_queue_key = getpid(); // should be unique
    m_login.queue_key=user_queue_key;
    strcpy(m_login.name,username);
    msgsnd(server_queue, &m_login, sizeof(m_login)-sizeof(long), 0);
    printf("Wysłano wiadomość logowania.\n");

    client_queue = msgget(user_queue_key, 0600 | IPC_CREAT);
    msgrcv(client_queue, &m_login_status, sizeof(m_login_status)-sizeof(long), Login, 0);

    if (m_login_status.status){
      client_id = m_login_status.id;
      printf("Zalogowano jako:\n");
      printf("Name: %s, ID: %d, Queue: 0x%x\n", username, m_login_status.id, user_queue_key);
    } else {
      printf("Logowanie nieudane:\n");
      printf("Name: %s, ID: %d, Queue: 0x%x\n", m_login_status.name, m_login_status.id, user_queue_key);
    }
  }


  char action = ' ';
  do {
    printf("Wybierz akcję:\n");
    printf("[1] Nowy temat\n");
    printf("[2] Subskrybcja tematu\n");
    printf("[3] Nowa wiadomość tekstowa\n");
    printf("[4] Odczytaj wiadomości tekstowe\n");
    printf("[5] Zablokuj użytkownika\n");
    printf("[q] Wyjście\n");

    scanf(" %s",&action);
    getchar(); // Usuń znak \n

    switch (action){

      case '1': // Nowy temat
        m_new_topic.client_id = client_id;
        printf("Podaj nazwę tematu: \n");
        scanf("%127[^\n]",m_new_topic.topic_name);
        getchar(); // Usuń znak \n
        msgsnd(server_queue,&m_new_topic,sizeof(m_new_topic)-sizeof(long),0);
        msgrcv(client_queue,&m_topic_status,sizeof(m_topic_status)-sizeof(long),NewTopic,0);
        if (m_topic_status.topic_id == 0){
          printf("Nie udało się utworzyć tematu.\n");
        } else {
          printf("Utworzono temat: %s, ID: %d\n",m_topic_status.topic_name,m_topic_status.topic_id);
        }
        break;

      case '2': // Subskrybcja
        
        break;

      case '3': // Nowa wiadomość
        break;

      case '4': // Odczytaj wiadomosći
        break;

      case '5': // Zablokuj użytkownika
        break;
      
      case ' ':
        break;

      case 'q':
        clean_exit();
        break;

      default:
        printf("Nieznana akcja.\n");
        break;
    }
  }
  while (action != 'q');
    
}
