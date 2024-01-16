// ┌────────────────────────┐
// │IPC Publish-Subscribe   │
// ├────────────────────────┤
// │Message types           │
// ├────────────────────────┤
// │Tomasz Pawłowski 155965 │
// │Jakub Kamieniarz 155845 │
// └────────────────────────┘
#ifndef MESSAGE_TYPES
#define MESSAGE_TYPES

#include <sys/ipc.h>
#include <sys/msg.h>
#define MAX_CLIENTS 16
#define MAX_TOPICS 128
#define MAX_MESSAGES 1024
#define MAX_SUBSCRIPTIONS 128
#define MAX_USERNAME_LENGTH 128
#define MAX_TOPIC_LENTH 128
#define MAX_BLOCKED_USERS 16
#define MAX_MESSAGE_LENGTH 2048

enum MessageType {
  Login = 1,
  Subscription = 2,
  NewTopic = 3,
  SendMessage = 4,
  BlockUser = 5,
  ReadMessages = 6,
};

enum SubscriptionType {
  Unsubscribed = 0,
  Permanent = 1,
  Temporary = 2,
  OversubscribedTopic = 3,
  UnknownTopic = 4,
};

typedef struct {
  char name[MAX_USERNAME_LENGTH]; // Nazwa klienta
  int client_id;                  // ID klienta
  int queue;                      // Identyfikator kolejki odbiorczej klienta
} Client;

typedef struct {
  int client_id;              // Identyfikator użytkownika
  int topic_id;               // Identyfikator tematu
  enum SubscriptionType type; // Informacje o subskrybcjach danych klientów
  int duration; // Pozostały czas trwania subskrybcji przejściowej
  int blocked_ids[MAX_BLOCKED_USERS]; // Zablokowani użytkownicy
} SubInfo;                            // Struktura informacji o subskrybentach

typedef struct {
  int topic_id;                     // Identyfikator tematu
  char topic_name[MAX_TOPIC_LENTH]; // Nazwa tematu
} Topic;

typedef struct {
  long type;                      // Typ komunikatu
  char name[MAX_USERNAME_LENGTH]; // Nazwa klienta
  key_t queue_key;                // Identyfikator kolejki odbiorczej klienta
} LoginMessage;

typedef struct {
  long type;                      // Typ komunikatu
  char name[MAX_USERNAME_LENGTH]; // Nazwa klienta
  int status;                     // Stan logowania: 0 - błąd
  int id;                         // ID klienta
} LoginStatus;

typedef struct {
  long type;                 // Typ komunikatu
  int client_id;             // ID klienta
  int topic_id;              // Identyfikator tematu
  enum SubscriptionType sub; // Rodzaj subskrybcji
  int duration;              // Długość trwania subskrybcji
} SubscriptionMessage;

typedef SubscriptionMessage SubscriptionStatus;

typedef struct {
  long type;                        // Typ komunikatu
  int client_id;                    // ID klienta
  char topic_name[MAX_TOPIC_LENTH]; // Nazwa tematu
} NewTopicMessage;

typedef struct {
  long type;                        // Typ komunikatu
  int topic_id;                     // Identyfikator tematu = 0
  char topic_name[MAX_TOPIC_LENTH]; // Nazwa tematu
} NewTopicStatus;

typedef struct {
  long type;                     // Typ komunikatu
  int topic_id;                  // Identyfikator tematu
  int client_id;                 // ID klienta
  char text[MAX_MESSAGE_LENGTH]; // Treść wiadomości
  int priority;                  // Priorytet wiadomości
} Message;

typedef struct {
  long type;     // Typ komunikatu
  int client_id; // ID blokującego klienta
  int block_id;  // ID blokowanego klienta
  int topic_id;  // Identyfikator tematu
} BlockUserMessage;

typedef struct {
  long type;
  int client_id;
  int priority;
  int last_read;
} ReadMessage;

#endif // MESSAGE_TYPES
