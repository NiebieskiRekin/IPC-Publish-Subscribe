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
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "inf155965_155845_message_types.h"

LoginMessage m_login = {.type = Login};
LoginStatus m_login_status = {.type = Login, .status=0};
SubscriptionMessage m_subscription = {.type = Subscription};
SubscriptionStatus m_sub_stat = {.type = Subscription};
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
        m_subscription.client_id = client_id;
        printf("Podaj ID tematu:\n");
        scanf("%d",&m_subscription.topic_id);
        getchar(); // Usuń znak \n
        m_subscription.sub = OversubscribedTopic;
        printf("Odsubskrybuj = 0,\nZasubskrybuj trwale = 1,\nZasubskrybuj tymczasowo = 2\n");
        do {
          printf("Podaj rodzaj działania: ");
          scanf("%d", (int*)(&m_subscription.sub));
          getchar(); // Usuń znak \n
        } while (m_subscription.sub > 2);
        if (m_subscription.sub == Temporary){
          printf("Podaj długość subskrybcji: ");
          scanf("%d", &m_subscription.duration);
          getchar(); // Usuń znak \n
        } else {
          m_subscription.duration = 0;
        }
        msgsnd(server_queue, &m_subscription, sizeof(m_subscription)-sizeof(long), 0);
        msgrcv(client_queue, &m_sub_stat, sizeof(m_sub_stat)-sizeof(long), Subscription, 0);
        switch (m_sub_stat.sub){
          case Unsubscribed:
            printf("Odsubskrybowano temat ID: %d\n", m_sub_stat.topic_id);
            break;
          case Temporary:
            printf("Zapisano subskrybcję tymczasową tematu o ID: %d, na długość %d wiadomości.\n",m_sub_stat.topic_id,m_sub_stat.duration);
            break;
          case Permanent:
            printf("Zapisano subskrybcję trwałą tematu o IDP %d.\n", m_sub_stat.topic_id);
            break;
          case OversubscribedTopic:
            printf("Temat o ID: %d jest subskrybowany przez zbyt wiele osób.\n", m_sub_stat.topic_id);
            break;
          case UnknownTopic:
            printf("Temat o podanym ID: %d nie istnieje.\n",m_sub_stat.topic_id);
            break;
        }
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
