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
#include <sys/param.h>
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
MessageCount m_count = {.type = MessageReadCount};
BlockUserMessage m_block_user = {.type=BlockUser};
ReadMessage m_read = {.type=ReadMessages};

int client_queue;
int client_id;
char username[128];
int last_read_message = 0;
char* line = NULL;

void clean_exit(int signo){
  msgctl(client_queue, IPC_RMID, NULL);
  free(line);
  exit(signo);
}

int main(void) {
  signal(SIGINT,clean_exit);
  key_t server_key; 
  printf("Podaj klucz kolejki serwera: \n");
  printf("0x");
  size_t len = 0;
  unsigned int temp;
  getline(&line,&len,stdin);
  sscanf(line," %x ", &temp);
  server_key = (int)temp;
  int server_queue = msgget(server_key, 0600 | IPC_CREAT);

  while (m_login_status.status == 0){
    printf("Podaj nazwę użytkownika: \n");
    getline(&line,&len,stdin);
    sscanf(line," %127[^\n] ",username); // odczyt max 127 znaków do nowej linii
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

    getline(&line,&len,stdin);
    sscanf(line," %c ",&action);

    switch (action){

      case '1': // Nowy temat
        m_new_topic.client_id = client_id;
        printf("Podaj nazwę tematu: \n");
        getline(&line,&len,stdin);
        sscanf(line," %127[^\n] ",m_new_topic.topic_name);

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
        getline(&line,&len,stdin);
        sscanf(line," %d ",&m_subscription.topic_id);
        m_subscription.sub = OversubscribedTopic;
        printf("Odsubskrybuj = 0,\nZasubskrybuj trwale = 1,\nZasubskrybuj tymczasowo = 2\n");
        do {
          printf("Podaj rodzaj działania: ");
          getline(&line,&len,stdin);
          sscanf(line, " %d ", (int*)(&m_subscription.sub));
        } while (m_subscription.sub > 2);
        if (m_subscription.sub == Temporary){
          printf("Podaj długość subskrybcji: ");
          getline(&line,&len,stdin);
          sscanf(line, " %d ", &m_subscription.duration);
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
            printf("Zapisano subskrybcję trwałą tematu o ID: %d.\n", m_sub_stat.topic_id);
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
        m_text.client_id = client_id;
        printf("Podaj ID tematu, na który zostanie wysłana wiadomość: ");
        getline(&line,&len,stdin);
        sscanf(line, " %d ",&m_text.topic_id);

        printf("Podaj priorytet wiadomości [0-9]: ");
        getline(&line,&len,stdin);
        sscanf(line, " %d ",&m_text.priority);
        m_text.priority = MAX(MIN(m_text.priority,9),0);

        printf("Podaj treść wiadomości: \n");
        for (int i=0; i<MAX_MESSAGE_LENGTH; i++)
          m_text.text[i] = '\0';

        int char_count = 0;
        while (char_count < MAX_MESSAGE_LENGTH-1 && getline(&line,&len,stdin) && line[0]!='\n'){
          strncat(m_text.text,line,MAX_MESSAGE_LENGTH-char_count-1);
          char_count += strlen(line);
        }

        msgsnd(server_queue,&m_text,sizeof(m_text)-sizeof(long),0);
        printf("\nWysłano wiadomość.\n");
        break;

      case '4': // Odczytaj wiadomości
        m_read.client_id = client_id;
        printf("Podaj priorytet wiadomości [0-9]: ");
        getline(&line,&len,stdin);
        sscanf(line, " %d ",&m_read.priority);
        m_read.priority = MAX(MIN(m_read.priority,9),0);

        m_read.last_read = last_read_message;
        int msg_read_now = 0;
        msgsnd(server_queue,&m_read,sizeof(m_read)-sizeof(long),0);

        msgrcv(client_queue, &m_count, sizeof(m_count)-sizeof(long), MessageReadCount, 0);


        while(msg_read_now++ < m_count.count && msgrcv(client_queue, &m_text, sizeof(m_text)-sizeof(long),SendMessage,0) && m_text.client_id!=0) {
          printf("Wiadomość %d/%d, Temat ID: %d, Autor ID: %d, Priorytet: %d\n", msg_read_now,m_count.count,m_text.topic_id,m_text.client_id,m_text.priority);
          printf("%s\n",m_text.text);
          last_read_message = MAX(last_read_message,m_text.message_id-1);
        }
        break;

      case '5': // Zablokuj użytkownika
        break;
      
      case ' ':
        break;

      case 'q':
        clean_exit(0);
        break;

      default:
        printf("Nieznana akcja.\n");
        break;
    }
  }
  while (action != 'q');
    
}
