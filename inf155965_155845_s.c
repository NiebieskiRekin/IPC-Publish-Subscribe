// ┌────────────────────────┐
// │IPC Publish-Subscribe   │
// ├────────────────────────┤
// │Serwer                  │
// ├────────────────────────┤
// │Tomasz Pawłowski 155965 │
// │Jakub Kamieniarz 155845 │
// └────────────────────────┘

#include "inf155965_155845_message_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <unistd.h>

LoginMessage m_login = {.type = Login};
LoginStatus m_login_status = {.type = Login};
SubscriptionMessage m_subscription = {.type = Subscription};
SubscriptionStatus m_sub_stat = {.type = Subscription};
NewTopicMessage m_new_topic = {.type = NewTopic};
NewTopicStatus m_topic_status = {.type = NewTopic};
Message m_text = {.type = SendMessage};
BlockUserMessage m_block_user = {.type = BlockUser};
ReadMessage m_read = {.type = ReadMessages};

Client logged_in[MAX_CLIENTS];
int n_logged = 0;
Topic topics[MAX_TOPICS];
int n_topics = 0;
Message messages[MAX_MESSAGES];
int n_messages = 0;
SubInfo subscriptions[MAX_SUBSCRIPTIONS];

key_t server_key;
int server_queue;

void clean_exit(int signo) {
  printf("\nZakończenie działania. Sygnał: %d\n", signo);
  for (int i = 0; i < MAX_CLIENTS; i++) {
    msgctl(logged_in[i].queue, IPC_RMID, NULL);
  }
  msgctl(server_queue, IPC_RMID, NULL);
  // free pointers etc.
  exit(0);
}

void init(void) {
  // Inicjalizacja klucza kolejki
  server_key = getpid(); // should be unique

  // Inicjalizacja klientów
  for (int i = 0; i < MAX_CLIENTS; i++) {
    strcpy(logged_in[i].name, "\0");
    logged_in[i].queue = 0;
    logged_in[i].client_id = 0;
  }

  // Inicjalizacja tematów
  for (int i = 0; i < MAX_TOPICS; i++) {
    topics[i].topic_id = 0;
    strcpy(topics[i].topic_name, "\0");
  }

  // Inicjalizacja subskrybcji
  for (int j = 0; j < MAX_SUBSCRIPTIONS; j++) {
      subscriptions[j].client_id = 0;
      subscriptions[j].duration = 0;
      subscriptions[j].type = Unsubscribed;
      subscriptions[j].topic_id = 0;
      for (int k = 0; k < MAX_BLOCKED_USERS; k++) {
        subscriptions[j].blocked_ids[k] = 0;
      }
    }

  // Inicjalizacja wiadomości tekstowych
  for (int i=0; i<MAX_MESSAGES; i++){
    messages[i].client_id = 0;
    messages[i].priority = 0;
    messages[i].topic_id = 0;
    messages[i].type = SendMessage;

    for (int j=0; j<MAX_MESSAGE_LENGTH-1; j++){
      messages[i].text[j] = ' ';
    }
    messages[i].text[MAX_MESSAGE_LENGTH-1] = '\0';
  }
}

int is_duplicate_name(char name[128]) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (strcmp(name, logged_in[i].name) == 0) {
      return 1;
    }
  }
  return 0;
}

int is_duplicate_topic(char topic[128]) {
  for (int i = 0; i < MAX_TOPICS; i++) {
    if (strcmp(topic, topics[i].topic_name) == 0) {
      return 1;
    }
  }
  return 0;
}

// Zwraca obecną subskrybcję tematu dla danego klienta lub następny wolny slot
SubInfo *get_client_subscription(int client_id, int topic_id) {
  for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
    SubInfo *sub = subscriptions+i;
    if (sub->client_id == client_id && sub->topic_id == topic_id) {
      return sub;
    }
  }
  for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
    SubInfo *sub = subscriptions+i;
    if (sub->client_id == 0) {
      return sub;
    }
  }
  return NULL;
}

int client_logged(int client_id) {
  return (client_id>0 && client_id<MAX_CLIENTS && logged_in[client_id - 1].client_id != 0);
}

int topic_exists(int topic_id) {
  return (topic_id>0 && topic_id<MAX_TOPICS && topics[topic_id - 1].topic_id != 0);
}

int handle_receive_text(void){
  printf("Nowa wiadomość.\n");
  if (m_text.client_id == 0 || !client_logged(m_text.client_id)) {
    printf("Nieznany klient. ID: %d\n", m_text.client_id);
    return -1;
  }
  
  if (!topic_exists(m_text.topic_id)) {
    printf("Nieznany temat. ID: %d\n", m_text.topic_id);
    return -1;
  }

  messages[n_messages].client_id = m_text.client_id;
  messages[n_messages].priority = m_text.priority;
  messages[n_messages].topic_id = m_text.topic_id;
  strcpy(messages[n_messages].text,m_text.text);
  n_messages++;

  return 0;
}

int handle_send_text(void){
  printf("Prośba o przesłanie wiadomości.\n");
  if (m_read.client_id == 0 || !client_logged(m_read.client_id)) {
    printf("Nieznany klient. ID: %d\n", m_read.client_id);
    return -1;
  }
  // TODO
  // if (!topic_exists(m_read.topic_id)) {
  //   printf("Nieznany temat. ID: %d\n", m_text.topic_id);
  //   return -1;
  // }

  // messages[n_messages].client_id = m_text.client_id;
  // messages[n_messages].priority = m_text.priority;
  // messages[n_messages].topic_id = m_text.topic_id;
  // strcpy(messages[n_messages].text,m_text.text);
  // n_messages++;

  // return 0;
}

int handle_subscription(void) {
  printf("Działanie na subskrybcji\n");
  if (m_subscription.client_id == 0 || !client_logged(m_subscription.client_id)) {
    printf("Nieznany klient. ID: %d\n", m_subscription.client_id);
    return -1;
  }

  int client = m_subscription.client_id -1;
  m_sub_stat.client_id = m_subscription.client_id;
  m_sub_stat.topic_id = m_subscription.topic_id;

  if (!topic_exists(m_subscription.topic_id)) {
    printf("Nieznany temat. ID: %d\n", m_subscription.topic_id);
    m_sub_stat.duration = -1;
    m_sub_stat.sub = UnknownTopic;
    msgsnd(logged_in[client].queue, &m_sub_stat,
           sizeof(m_sub_stat) - sizeof(long), 0);
    return -1;
  }

  SubInfo *sub_info = get_client_subscription(m_subscription.client_id,
                                              m_subscription.topic_id);
  if (sub_info == NULL) {
    // Klient nie posiada subskrybcji oraz nie może zasubskrybować
    printf("Temat ID: %d subskrybowany przez zbyt wiele osób", m_subscription.topic_id);
    m_sub_stat.sub = OversubscribedTopic;
    m_sub_stat.duration = -1;
    msgsnd(logged_in[client].queue, &m_sub_stat,
           sizeof(m_sub_stat) - sizeof(long), 0);
    return -1;
  }

  switch (m_subscription.sub){
    case Temporary:
      sub_info->duration = MAX(sub_info->duration+m_subscription.duration,0);
      sub_info->client_id = m_subscription.client_id;
      break;
    case Unsubscribed:
      sub_info->client_id = 0;
      for (int i=0; i<MAX_BLOCKED_USERS; i++){
        sub_info->blocked_ids[i] = 0;
      }
      sub_info->duration = 0;
      break;
    case Permanent:
      sub_info->duration = 0;
      sub_info->client_id = m_subscription.client_id;
      break;
    default:
      return -1;
      break;
  }

  sub_info->type = m_subscription.sub;
  m_sub_stat.sub = m_subscription.sub;
  m_sub_stat.duration = sub_info->duration;
  sub_info->topic_id = m_subscription.topic_id;
  printf("Subskrybcja %d, długość: %d, ",sub_info->type,sub_info->duration);
  printf("klient ID: %d, Temat ID: %d\n",sub_info->client_id,m_sub_stat.topic_id);

  msgsnd(logged_in[client].queue, &m_sub_stat,
         sizeof(m_sub_stat) - sizeof(long), 0);
  return 0;
}

int handle_new_topic(void) {
  printf("Nowy temat\n");

  if (m_new_topic.client_id == 0) {
    printf("Nieznany klient.\n");
    return -1;
  }

  strcpy(m_topic_status.topic_name, m_new_topic.topic_name);

  if (n_topics >= MAX_TOPICS || is_duplicate_topic(m_new_topic.topic_name)) {
    printf("Temat nie może zostać utworzony.\n");
    m_topic_status.topic_id = 0;
    msgsnd(logged_in[m_new_topic.client_id].queue, &m_topic_status,
           sizeof(m_topic_status) - sizeof(long), 0);
    return -1;
  }

  // Dodaj temat do bazy
  topics[n_topics].topic_id = n_topics + 1;
  strcpy(topics[n_topics].topic_name, m_new_topic.topic_name);
  printf("Dodano nowy temat:\n");
  printf("Temat: %s, ID: %d, Autor: %s\n", topics[n_topics].topic_name,
         topics[n_topics].topic_id, logged_in[m_new_topic.client_id - 1].name);

  // Wyślij potwierdzenie utworzenia nowego tematu do klienta
  m_topic_status.topic_id = n_topics + 1;
  msgsnd(logged_in[m_new_topic.client_id - 1].queue, &m_topic_status,
         sizeof(m_topic_status) - sizeof(long), 0);
  n_topics++;
  return 0;
}

int handle_login(void) {
  printf("Nowe zapytanie o logowanie.\n");

  int queue_key = m_login.queue_key;
  int temp = msgget(queue_key, 0600 | IPC_CREAT);

  // Jeśli liczba klientów została przekroczona lub dany użytkownik istnieje
  if (n_logged >= MAX_CLIENTS || is_duplicate_name(m_login.name)) {
    printf("Niepoprawne logowanie\n");
    strcpy(m_login_status.name, m_login.name);
    m_login_status.status = 0;
    m_login_status.id = -1;
    msgsnd(temp, &m_login_status, sizeof(m_login_status) - sizeof(long),
           IPC_NOWAIT);
    msgctl(temp, IPC_RMID, NULL);
    return -1;
  }

  // Zarejestruj dane w bazie, otwórz kolejkę klienta
  strcpy(logged_in[n_logged].name, m_login.name);
  logged_in[n_logged].client_id = n_logged + 1;
  logged_in[n_logged].queue = temp;

  // Wyślij potwierdzenie
  strcpy(m_login_status.name, logged_in[n_logged].name);
  m_login_status.id = logged_in[n_logged].client_id;
  m_login_status.status = 1;

  printf("Zalogowano klienta:\n");
  printf("Name: %s, ID: %d, Queue: 0x%x\n", m_login_status.name,
         m_login_status.id, m_login.queue_key);

  msgsnd(logged_in[n_logged].queue, &m_login_status,
         sizeof(m_login_status) - sizeof(long), 0);
  n_logged++;
  return 0;
}

int main(void) {
  // Zwolnij zasoby przy Ctrl-C
  signal(SIGINT, clean_exit);
  // Zainicjalizuj tablice danych
  init();

  // Globalna kolejka serwera,
  // key kolejki wyświetlany na ekranie przez proces serwera
  server_queue = msgget(server_key, 0600 | IPC_CREAT);
  printf("Klucz kolejki serwera: 0x%x\n", server_key);

  while (1) {

    // Logowanie
    if (msgrcv(server_queue, &m_login, sizeof(m_login) - sizeof(long), Login,
               IPC_NOWAIT) != -1) {
      handle_login();
    }

    // Nowy temat
    else if (msgrcv(server_queue, &m_new_topic,
                    sizeof(m_new_topic) - sizeof(long), NewTopic,
                    IPC_NOWAIT) != -1) {
      handle_new_topic();
    }

    // Subskrybcja tematu
    else if (msgrcv(server_queue, &m_subscription,
                    sizeof(m_subscription) - sizeof(long), Subscription,
                    IPC_NOWAIT) != -1) {
      handle_subscription();
    }

    // Nowa wiadomość tekstowa
    else if (msgrcv(server_queue, &m_text, sizeof(m_text) - sizeof(long),
                    SendMessage, IPC_NOWAIT) != -1) {
      handle_receive_text();
    }

    // Prośba o przesłanie nowych wiadomości
    else if (msgrcv(server_queue, &m_read, sizeof(m_read) - sizeof(long),
                    ReadMessages, IPC_NOWAIT) != -1) {
      handle_send_text();
    }

    // Blokowanie użytkownika
    else if (msgrcv(server_queue, &m_block_user,
                    sizeof(m_block_user) - sizeof(long), BlockUser,
                    IPC_NOWAIT) != -1) {
      // TODO
    }

  } // end while(1)
}
