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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CLIENTS 16
#define MAX_TOPICS 128
#define MAX_MESSAGES 1024

LoginMessage m_login = {.type = Login};
LoginStatus m_login_status = {.type = Login};
SubscriptionMessage m_subscription = {.type = Subscription};
NewTopicMessage m_new_topic = {.type = NewTopic};
NewTopicStatus m_topic_status = {.type = NewTopic};
Message m_text = {.type=SendMessage};
BlockUserMessage m_block_user = {.type=BlockUser};

Client logged_in[MAX_CLIENTS];
int n_logged = 0;
Topic topics[MAX_TOPICS];
int n_topics = 0;
Message messages[MAX_MESSAGES];
int n_message = 0;

key_t server_key = 0x100;
int server_queue;

void clean_exit() {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    msgctl(logged_in[i].queue, IPC_RMID, NULL);
  }
  msgctl(server_queue, IPC_RMID, NULL);
  // free pointers etc.
  exit(0);
}

void init(){

  // Inicjalizacja klientów
  for (int i = 0; i < MAX_CLIENTS; i++) {
    strcpy(logged_in[i].name,"\0");
    logged_in[i].queue = 0;
    logged_in[i].client_id= 0;
  }

  // Inicjalizacja tematów


  // Inicjalizacja wiadomości tekstowych 

  
  
}

int is_duplicate_name(char name[128]){
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (strcmp(name,logged_in[i].name) == 0){
      return 1;
    }
  }
  return 0;
}

int main() {
  // Zwolnij zasoby przy Ctrl-C
  signal(2, clean_exit);
  // Zainicjalizuj tablice danych 
  init();

  // Globalna kolejka serwera, 
  // key kolejki wyświetlany na ekranie przez proces serwera
  msgget(server_key, 0600 | IPC_CREAT);
  printf("Klucz kolejki serwera: %X\n", server_key);

  while (1) {
    // Serwer otrzymał informację o logowaniu

    if (msgrcv(server_queue, &m_login, sizeof(m_login)-sizeof(long), Login, IPC_NOWAIT)!=-1) {
      printf("Nowe zapytanie o logowanie.\n");
  
      int queue_key = m_login.queue_key;
      int temp = msgget(queue_key, 0600 | IPC_CREAT);

      // Jeśli liczba klientów została przekroczona lub dany użytkownik istnieje
      if (n_logged >= MAX_CLIENTS || is_duplicate_name(m_login.name)){
        printf("Niepoprawne logowanie\n");
        strcpy(m_login_status.name,m_login.name);
        m_login_status.status = 0;
        m_login_status.id = -1;
        msgsnd(temp,&m_login_status,sizeof(m_login_status)-sizeof(long),IPC_NOWAIT);
        msgctl(temp, IPC_RMID, NULL);
        break;
      }

      // Zarejestruj dane w bazie, otwórz kolejkę klienta
      strcpy(logged_in[n_logged].name, m_login.name);
      logged_in[n_logged].client_id = n_logged+1;
      logged_in[n_logged].queue = temp;

      // Wyślij potwierdzenie
      strcpy(m_login_status.name, logged_in[n_logged].name);
      m_login_status.id = logged_in[n_logged].client_id;
      m_login_status.status = 1;

      printf("Zalogowano klienta:\n");
      printf("Name: %s, ID: %d, Queue: %X\n", m_login_status.name, m_login_status.id, m_login.queue_key);

      msgsnd(logged_in[n_logged].queue, &m_login_status, sizeof(m_login_status)-sizeof(long), 0);
      n_logged++;
    }
  }
}
