// ┌────────────────────────┐
// │IPC Publish-Subscribe   │
// ├────────────────────────┤
// │Klient                  │
// ├────────────────────────┤
// │Tomasz Pawłowski 155965 │
// │Jakub Kamieniarz 155845 │
// └────────────────────────┘

#include "inf155965_155845_message_types.h"
#include <bits/types/idtype_t.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/param.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

LoginMessage m_login = {.type = Login};
LoginStatus m_login_status = {.type = Login, .status = 0};
SubscriptionMessage m_subscription = {.type = Subscription};
SubscriptionStatus m_sub_stat = {.type = Subscription};
NewTopicMessage m_new_topic = {.type = NewTopic};
NewTopicStatus m_topic_status = {.type = NewTopic};
Message m_text = {.type = SendMessage};
MessageCount m_count = {.type = MessageReadCount};
BlockUserMessage m_block_user = {.type = BlockUser};
ReadMessage m_read = {.type = ReadMessages};
static struct sembuf semaphore_buf = {.sem_num = 0, .sem_flg = 0};

void opusc(int semid) {
  semaphore_buf.sem_op = -1;
  if (semop(semid, &semaphore_buf, 1) == -1) {
    perror("Opuszczenie semafora");
    exit(1);
  }
}

void podnies(int semid) {
  semaphore_buf.sem_op = 1;
  if (semop(semid, &semaphore_buf, 1) == -1) {
    perror("Podnoszenie semafora");
    exit(1);
  }
}

int client_queue;
int client_id;
char username[MAX_USERNAME_LENGTH + 1];
int last_read_message[N_PRIORITIES] = {0, 0, 0};
char *line = NULL;
pid_t child;
int sem_UI;

void clean_exit(int signo) {
  msgctl(client_queue, IPC_RMID, NULL);
  free(line);
  if (child > 0) {
    kill(child, 9);
  }
  semctl(sem_UI, 0, IPC_RMID);
  exit(signo);
}

int main(void) {
  signal(SIGINT, clean_exit);
  key_t server_key;
  printf("Podaj klucz kolejki serwera: \n");
  printf("0x");
  size_t len = 0;
  unsigned int temp;
  getline(&line, &len, stdin);
  sscanf(line, " %x ", &temp);
  server_key = (int)temp;
  int server_queue = msgget(server_key, 0600 | IPC_CREAT);

  while (m_login_status.status == 0) {
    printf("Podaj nazwę użytkownika: \n");
    getline(&line, &len, stdin);
    // Trim and copy up to n
    int i = 0;
    while (isspace(line[i])) {
      i++;
    }
    int line_size = strlen(line);
    int j;
    for (j = 0; j < MAX_USERNAME_LENGTH && i < line_size && line[i] != '\n';
         j++) {
      username[j] = line[i];
      i++;
    }
    username[j] = '\0';

    key_t user_queue_key = getpid(); // should be unique
    m_login.queue_key = user_queue_key;
    strcpy(m_login.name, username);
    msgsnd(server_queue, &m_login, sizeof(m_login) - sizeof(long), 0);
    printf("Wysłano wiadomość logowania.\n");

    client_queue = msgget(user_queue_key, 0600 | IPC_CREAT);
    sem_UI = semget(IPC_PRIVATE, 1, 0600 | IPC_CREAT);
    semaphore_buf.sem_op = 1+INTERRUPT_INPUT;
    if (semop(sem_UI, &semaphore_buf, 1) == -1){
      perror("Inicjalizacja semafora nieudana\n");
      exit(1);
    }
    msgrcv(client_queue, &m_login_status, sizeof(m_login_status) - sizeof(long),
           Login, 0);

    if (m_login_status.status) {
      client_id = m_login_status.id;
      printf("Zalogowano jako:\n");
      printf("Name: %s, ID: %d, Queue: 0x%x\n", username, m_login_status.id,
             user_queue_key);
    } else {
      printf("Logowanie nieudane:\n");
      printf("Name: %s, ID: %d, Queue: 0x%x\n", m_login_status.name,
             m_login_status.id, user_queue_key);
    }
  }

  child = fork();
  if (child < 0) {
    printf("Nie można utworzyć procesu potomnego. Kończenie działania...");
    clean_exit(child);
  } else if (child == 0) {
    while (1) {
      if (msgrcv(client_queue, &m_text, sizeof(m_text) - sizeof(long),
                 AsyncMessage, 0) &&
          m_text.client_id != 0) {
        opusc(sem_UI);
        printf("\n---------Nowa wiadomość--------\n"
               "Temat ID: %d, Autor ID: %d, Priorytet: %d\n",
               m_text.topic_id, m_text.client_id, m_text.priority);
        printf("%s\n", m_text.text);
        printf("---------------------------------\n");
        podnies(sem_UI);
      }
    }
  } else {
    char action = ' ';
    do {
      printf("Wybierz akcję:\n");
      printf("[1] Nowy temat\n");
      printf("[2] Subskrybcja tematu\n");
      printf("[3] Nowa wiadomość tekstowa\n");
      printf("[4] Odczytaj wiadomości tekstowe\n");
      printf("[5] Zablokuj użytkownika\n");
      printf("[q] Wyjście\n");

      opusc(sem_UI);
      do {
        printf("> ");
        getline(&line, &len, stdin);
      } while (sscanf(line, " %c ", &action) != 1);
      podnies(sem_UI);

      switch (action) {

      case '1': // Nowy temat
        m_new_topic.client_id = client_id;
        printf("Podaj nazwę tematu: \n");
        opusc(sem_UI);
        do {
          printf("> ");
          getline(&line, &len, stdin);
        } while (line[0] == '\n');
        podnies(sem_UI);

        // Trim and copy up to n
        int i = 0;
        while (isspace(line[i])) {
          i++;
        }
        int line_size = strlen(line);
        int j;
        for (j = 0; j < MAX_TOPIC_LENTH && i < line_size && line[i] != '\n';
             j++) {
          m_new_topic.topic_name[j] = line[i];
          i++;
        }
        m_new_topic.topic_name[j] = '\0';

        msgsnd(server_queue, &m_new_topic, sizeof(m_new_topic) - sizeof(long),
               0);
        msgrcv(client_queue, &m_topic_status,
               sizeof(m_topic_status) - sizeof(long), NewTopic, 0);
        if (m_topic_status.topic_id == 0) {
          printf("Nie udało się utworzyć tematu.\n");
        } else {
          printf("Utworzono temat: %s, ID: %d\n", m_topic_status.topic_name,
                 m_topic_status.topic_id);
        }
        break;

      case '2': // Subskrybcja
        m_subscription.client_id = client_id;
        printf("Podaj ID tematu:\n");
        opusc(sem_UI);
        do {
          printf("> ");
          getline(&line, &len, stdin);
        } while (sscanf(line, " %d ", &m_subscription.topic_id) != 1 &&
                 m_subscription.topic_id <= 0);
        podnies(sem_UI);

        m_subscription.sub = Oversubscribed;
        printf("[0] Odsubskrybuj temat,\n"
               "[1] Zasubskrybuj temat trwale,\n"
               "[2] Zasubskrybuj temat tymczasowo \n");
        opusc(sem_UI);
        do {
          printf("> ");
          getline(&line, &len, stdin);
        } while (sscanf(line, " %d ", (int *)(&m_subscription.sub)) != 1 &&
                 m_subscription.sub > 2);
        podnies(sem_UI);

        switch (m_subscription.sub) {
        case TemporaryAtRequest:
          printf("Podaj długość subskrybcji:\n");
          opusc(sem_UI);
          do {
            printf("> ");
            getline(&line, &len, stdin);
          } while (sscanf(line, " %d ", &m_subscription.duration) != 1 &&
                   m_subscription.duration <= 0);
          podnies(sem_UI);

          printf("Określ sposób przesyłania wiadomości: \n");
          printf("[1] Prześlij, gdy nadeślę zapytanie, \n");
          printf("[2] Prześlij natychmiast po otrzymaniu \n");
          {
            int temp;
            opusc(sem_UI);
            do {
              printf("> ");
              getline(&line, &len, stdin);
            } while (sscanf(line, " %d ", &temp) != 1 &&
                     (temp < 1 || temp > 2));
            podnies(sem_UI);
            switch (temp) {
            case 1:
              m_subscription.sub += 0;
              break;
            case 2:
              m_subscription.sub += 2;
              break;
            }
          }
          break;
        case PermanentAtRequest:
          printf("Określ sposób przesyłania wiadomości: \n");
          printf("[1] Prześlij, gdy nadeślę zapytanie, \n");
          printf("[2] Prześlij natychmiast po otrzymaniu \n");
          {
            int temp;
            opusc(sem_UI);
            do {
              printf("> ");
              getline(&line, &len, stdin);
            } while (sscanf(line, " %d ", &temp) != 1 &&
                     (temp < 1 || temp > 2));
            podnies(sem_UI);
            switch (temp) {
            case 1:
              m_subscription.sub += 0;
              break;
            case 2:
              m_subscription.sub += 2;
              break;
            }
          }
          m_subscription.duration = 0;
          break;
        default:
          break;
        }

        msgsnd(server_queue, &m_subscription,
               sizeof(m_subscription) - sizeof(long), 0);
        msgrcv(client_queue, &m_sub_stat, sizeof(m_sub_stat) - sizeof(long),
               Subscription, 0);
        switch (m_sub_stat.sub) {
        case Unsubscribed:
          printf("Odsubskrybowano temat ID: %d\n", m_sub_stat.topic_id);
          break;
        case (TemporaryAtRequest):
          printf("Zapisano subskrybcję synchroniczną tymczasową tematu o ID: "
                 "%d, na długość %d wiadomości.\n",
                 m_sub_stat.topic_id, m_sub_stat.duration);
          break;
        case (TemporaryAsSoonAsReceived):
          printf("Zapisano subskrybcję asynchroniczną tymczasową tematu o ID: "
                 "%d, na długość %d wiadomości.\n",
                 m_sub_stat.topic_id, m_sub_stat.duration);
          break;
        case PermanentAtRequest:
          printf("Zapisano subskrybcję trwałą synchroniczną tematu o ID: %d.\n",
                 m_sub_stat.topic_id);
          break;
        case PermanentAsSoonAsReceived:
          printf(
              "Zapisano subskrybcję trwałą asynchroniczną tematu o ID: %d.\n",
              m_sub_stat.topic_id);
          break;
        case Oversubscribed:
          printf("W systemie jest zbyt wiele subskrybcji.\n");
          break;
        case UnknownTopic:
          printf("Temat o podanym ID: %d nie istnieje.\n", m_sub_stat.topic_id);
          break;
        default:
          printf("???\n");
        }
        break;

      case '3': // Nowa wiadomość
        m_text.client_id = client_id;
        printf("Podaj ID tematu, na który zostanie wysłana wiadomość:\n");
        opusc(sem_UI);
        do {
          printf("> ");
          getline(&line, &len, stdin);
        } while(sscanf(line, " %d ", &m_text.topic_id) != 1 && m_text.topic_id <= 0);
        podnies(sem_UI);

        printf("Podaj priorytet wiadomości [1-3]:\n");
        opusc(sem_UI);
        do {
          printf("> ");
          getline(&line, &len, stdin);
        } while(sscanf(line, " %d ", &m_text.priority)!=1);
        podnies(sem_UI);
        m_text.priority = MAX(MIN(m_text.priority, N_PRIORITIES), 1);

        printf("Podaj treść wiadomości:\n");
        for (int i = 0; i < MAX_MESSAGE_LENGTH + 1; i++)
          m_text.text[i] = '\0';

        opusc(sem_UI);
        int char_count = 0;
        while (char_count < MAX_MESSAGE_LENGTH && getline(&line, &len, stdin) &&
               line[0] != '\n') {
          strncat(m_text.text, line, MAX_MESSAGE_LENGTH - char_count - 1);
          char_count += strlen(line);
        }
        podnies(sem_UI);

        msgsnd(server_queue, &m_text, sizeof(m_text) - sizeof(long), 0);
        printf("\nWysłano wiadomość.\n");
        break;

      case '4': // Odczytaj wiadomości
        m_read.client_id = client_id;
        printf("Podaj priorytet wiadomości [1-%d]: \n", N_PRIORITIES);
        opusc(sem_UI);
        do {
          printf("> ");
          getline(&line, &len, stdin);
        } while (sscanf(line, " %d ", &m_read.priority) != 1);
        podnies(sem_UI);
        m_read.priority = MAX(MIN(m_read.priority, N_PRIORITIES), 1);

        for (int i = 0; i < N_PRIORITIES; i++) {
          m_read.last_read[i] = last_read_message[i];
        }
        int msg_read_now = 0;
        msgsnd(server_queue, &m_read, sizeof(m_read) - sizeof(long), 0);

        msgrcv(client_queue, &m_count, sizeof(m_count) - sizeof(long),
               MessageReadCount, 0);

        if (m_count.count == 0){
          printf("\n----Brak nowych wiadomości-----\n");
        } else if (m_count.count == 1){
          printf("\n---------Nowa wiadomość--------\n");
        } else {
          printf("\n-------%d nowe wiadomości------\n", m_count.count);
        }

        while (msg_read_now++ < m_count.count &&
               msgrcv(client_queue, &m_text, sizeof(m_text) - sizeof(long),
                      SendMessage, 0) &&
               m_text.client_id != 0) {
          printf("Wiadomość %d/%d, Temat ID: %d, Autor ID: %d, Priorytet: %d\n",
                 msg_read_now, m_count.count, m_text.topic_id, m_text.client_id,
                 m_text.priority);
          printf("%s\n", m_text.text);
          int p = m_text.priority;
          last_read_message[p] =
              MAX(last_read_message[p], m_text.message_id + 1);
        }
        printf("---------------------------------\n");
        break;

      case '5': // Zablokuj użytkownika
        m_block_user.client_id = client_id;

        printf("Podaj ID blokowanego użytkownika:\n");
        opusc(sem_UI);
        do {
          printf("> ");
          getline(&line, &len, stdin);
        } while (sscanf(line, " %d ", &m_block_user.block_id) != 1 && m_block_user.block_id <=0);
        podnies(sem_UI);

        printf("Podaj ID tematu, poprzez który użytkownik nie będzie mógł się komunikować.\n"
               "Ustaw wartość [0] dla wszystkich tematów:\n");
        opusc(sem_UI);
        do {
          getline(&line, &len, stdin);
        } while(sscanf(line, " %d ", &m_block_user.topic_id) != 1 && m_block_user.topic_id <=0);
        podnies(sem_UI);

        msgsnd(server_queue, &m_block_user, sizeof(m_block_user) - sizeof(long),
               0);
        msgrcv(client_queue, &m_block_user, sizeof(m_block_user) - sizeof(long),
               BlockUser, 0);

        if (m_block_user.block_id == 0) {
          printf("Niepowodzenie.\n");
        } else {
          printf("Użytkownik o ID: %d został zablokowany ",
                 m_block_user.block_id);
          if (m_block_user.topic_id == 0) {
            printf("globalnie.\n");
          } else {
            printf("na temat ID: %d.\n", m_block_user.topic_id);
          }
        }
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
    } while (action != 'q');
  }
}
