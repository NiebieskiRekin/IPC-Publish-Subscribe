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
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

LoginMessage m_login = {.type = Login};
LoginStatus m_login_status = {.type = Login};
SubscriptionMessage m_subscription = {.type = Subscription};
SubscriptionStatus m_sub_stat = {.type = Subscription};
NewTopicMessage m_new_topic = {.type = NewTopic};
NewTopicStatus m_topic_status = {.type = NewTopic};
Message m_text = {.type = SendMessage};
MessageCount m_count = {.type = MessageReadCount};
BlockUserMessage m_block_user = {.type = BlockUser};
ReadMessage m_read = {.type = ReadMessages};

Client logged_in[MAX_CLIENTS];
int n_logged = 0;
Topic topics[MAX_TOPICS];
int n_topics = 0;
Message messages[N_PRIORITIES][MAX_MESSAGES];
int n_messages[N_PRIORITIES];
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
  for (int j = 0; j < N_PRIORITIES; j++) {
    n_messages[j] = 0;
    for (int i = 0; i < MAX_MESSAGES; i++) {
      messages[j][i].client_id = 0;
      messages[j][i].priority = 0;
      messages[j][i].topic_id = 0;
      messages[j][i].type = SendMessage;
      messages[j][i].message_id = 0;

      for (int k = 0; k < MAX_MESSAGE_LENGTH + 1; k++) {
        messages[j][i].text[k] = '\0';
      }
    }
  }
}

int is_sub_async(SubInfo *s) {
  switch (s->type) {
  case TemporaryAsSoonAsReceived:
  case PermanentAsSoonAsReceived:
    return 1;
  default:
    return 0;
  }
}

int is_duplicate_name(char name[MAX_USERNAME_LENGTH + 1]) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (strcmp(name, logged_in[i].name) == 0) {
      return 1;
    }
  }
  return 0;
}

int is_duplicate_topic(char topic[MAX_TOPIC_LENTH + 1]) {
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
    SubInfo *sub = subscriptions + i;
    if (sub->client_id == client_id && sub->topic_id == topic_id) {
      return sub;
    }
  }
  for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
    SubInfo *sub = subscriptions + i;
    if (sub->client_id == 0) {
      return sub;
    }
  }
  return NULL;
}

int client_logged(int client_id) {
  return (client_id > 0 && client_id < MAX_CLIENTS &&
          logged_in[client_id - 1].client_id != 0);
}

int topic_exists(int topic_id) {
  return (topic_id > 0 && topic_id < MAX_TOPICS &&
          topics[topic_id - 1].topic_id != 0);
}

int handle_block_user(void) {
  printf("Blokowanie użytkownika.");

  // check if both users exist
  if (!(client_logged(m_block_user.client_id))) {
    printf("Blokujący użytkownik nie istnieje.\n");
    return -1;
  }
  if (!client_logged(m_block_user.block_id)) {
    printf("Blokowany użytkownik nie istnieje.\n");
    m_block_user.block_id = 0;
    msgsnd(logged_in[m_block_user.client_id - 1].queue, &m_block_user,
           sizeof(m_block_user) - sizeof(long), 0);
    return -1;
  }

  // match topic id or apply for all their topics
  if (m_block_user.topic_id == 0) {
    // GLOBAL BLOCK
    // find all subscriptions of the blocking user
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
      // apply for all their topics
      if (subscriptions[i].client_id == m_block_user.client_id) {
        // add blocked user id to the array on the first empty slot
        int j = 0;
        while (subscriptions[i].blocked_ids[j] != 0) {
          if (j >= MAX_BLOCKED_USERS) {
            m_block_user.block_id = 0;
            msgsnd(logged_in[m_block_user.client_id - 1].queue, &m_block_user,
                   sizeof(m_block_user) - sizeof(long), 0);
            printf("Przepełnienie listy zablokowanych użytkowników.\n");
            return -1;
          }
          j++;
        }
        subscriptions[i].blocked_ids[j] = m_block_user.block_id;
      }
    }
  } else {
    // per Topic block
    // find the subscription in question for the blocking user
    if (!topic_exists(m_block_user.topic_id)) {
      printf("Temat nie istnieje.\n");
      return -1;
    }

    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
      if (subscriptions[i].client_id == m_block_user.client_id &&
          subscriptions[i].topic_id == m_block_user.topic_id) {
        // add blocked user id to the array on the first empty slot
        int j = 0;
        while (subscriptions[i].blocked_ids[j] != 0) {
          if (j >= MAX_BLOCKED_USERS) {
            m_block_user.block_id = 0;
            msgsnd(logged_in[m_block_user.client_id - 1].queue, &m_block_user,
                   sizeof(m_block_user) - sizeof(long), 0);
            printf("Przepełnienie listy zablokowanych użytkowników.\n");
            return -1;
          }
          j++;
        }
        subscriptions[i].blocked_ids[j] = m_block_user.block_id;
        break;
      }
    }
  }

  printf("Użytkownik %d zablokował %d.", m_block_user.client_id,
         m_block_user.block_id);
  msgsnd(logged_in[m_block_user.client_id - 1].queue, &m_block_user,
         sizeof(m_block_user) - sizeof(long), 0);
  return 0;
}

int author_blocked(Message *msg, SubInfo *sub) {
  for (int i = 0; i < MAX_BLOCKED_USERS; i++) {
    if (sub->blocked_ids[i] == msg->client_id) {
      return 1;
    }
  }
  return 0;
}

int handle_receive_text(void) {
  printf("Nowa wiadomość.\n");
  if (m_text.client_id == 0 || !client_logged(m_text.client_id)) {
    printf("Nieznany klient. ID: %d\n", m_text.client_id);
    return -1;
  }

  if (!topic_exists(m_text.topic_id)) {
    printf("Nieznany temat. ID: %d\n", m_text.topic_id);
    return -1;
  }

  int p = m_text.priority;
  messages[p][n_messages[p]].client_id = m_text.client_id;
  messages[p][n_messages[p]].priority = m_text.priority;
  messages[p][n_messages[p]].topic_id = m_text.topic_id;
  strcpy(messages[p][n_messages[p]].text, m_text.text);
  n_messages[p]++;

  // Send to all subscriptions of the topic that are asynchronous
  for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
    if (subscriptions[i].topic_id == m_text.topic_id &&
        subscriptions[i].client_id != m_text.client_id &&
        is_sub_async(subscriptions + i) &&
        !author_blocked(&m_text, subscriptions + i)) {
      m_text.type = AsyncMessage;
      msgsnd(logged_in[(subscriptions + i)->client_id - 1].queue, &m_text,
             sizeof(m_text) - sizeof(long), 0);
    }
  }

  return 0;
}

int handle_send_text(void) {
  printf("Prośba o przesłanie wiadomości.\n");
  if (m_read.client_id == 0 || !client_logged(m_read.client_id)) {
    printf("Nieznany klient. ID: %d\n", m_read.client_id);
    return -1;
  }

  SubInfo *client_sub[MAX_SUBSCRIPTIONS];
  int n_sub = 0;

  for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
    if (subscriptions[i].client_id == m_read.client_id &&
        !is_sub_async(subscriptions + i)) {
      client_sub[n_sub] = subscriptions + i;
      n_sub++;
    }
  }

  Message *msg_buf[MAX_MESSAGES];
  int n_buf = 0;

  for (int p = m_read.priority; p < N_PRIORITIES; p++) {
    for (int i = m_read.last_read[p]; i < MAX_MESSAGES; i++) {
      for (int j = 0; j < n_sub; j++) {
        if (messages[p][i].topic_id == client_sub[j]->topic_id &&
            client_sub[j]->client_id != messages[p][i].client_id &&
            !author_blocked(&messages[p][i], client_sub[j])) {
          if (client_sub[j]->type == TemporaryAtRequest) {
            if (client_sub[j]->duration == 0) {
              break;
            } else {
              client_sub[j]->duration--;
            }
          }
          msg_buf[n_buf++] = &messages[p][i];
        }
      }
    }
  }
  m_count.count = n_buf;
  msgsnd(logged_in[m_read.client_id - 1].queue, &m_count,
         sizeof(m_count) - sizeof(long), 0);

  for (int i = 0; i < n_buf; i++) {
    m_text.client_id = msg_buf[i]->client_id;
    m_text.topic_id = msg_buf[i]->topic_id;
    m_text.priority = msg_buf[i]->priority;
    strcpy(m_text.text, msg_buf[i]->text);
    msgsnd(logged_in[m_read.client_id - 1].queue, &m_text,
           sizeof(m_text) - sizeof(long), 0);
  }
  return 0;
}

int handle_subscription(void) {
  printf("Działanie na subskrybcji\n");
  if (m_subscription.client_id == 0 ||
      !client_logged(m_subscription.client_id)) {
    printf("Nieznany klient. ID: %d\n", m_subscription.client_id);
    return -1;
  }

  int client = m_subscription.client_id - 1;
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
    printf("Zbyt wiele subskrybcji w systemie.\n");
    m_sub_stat.sub = Oversubscribed;
    m_sub_stat.duration = -1;
    msgsnd(logged_in[client].queue, &m_sub_stat,
           sizeof(m_sub_stat) - sizeof(long), 0);
    return -1;
  }

  switch (m_subscription.sub) {
  case TemporaryAtRequest:
  case TemporaryAsSoonAsReceived:
    sub_info->duration = MAX(sub_info->duration + m_subscription.duration, 0);
    sub_info->client_id = m_subscription.client_id;
    break;
  case Unsubscribed:
    sub_info->client_id = 0;
    for (int i = 0; i < MAX_BLOCKED_USERS; i++) {
      sub_info->blocked_ids[i] = 0;
    }
    sub_info->duration = 0;
    break;
  case PermanentAtRequest:
  case PermanentAsSoonAsReceived:
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
  printf("Subskrybcja %d, długość: %d, ", sub_info->type, sub_info->duration);
  printf("klient ID: %d, Temat ID: %d\n", sub_info->client_id,
         m_sub_stat.topic_id);

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
      handle_block_user();
    }

  } // end while(1)
}
