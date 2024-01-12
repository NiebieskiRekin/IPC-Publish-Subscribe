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

#include <sys/msg.h>

enum MessageType {
  Login = 1,
  Subscription = 2,
  NewTopic = 3,
  SendMessage = 4,
  BlockUser = 5,
};

enum SubscriptionType {
  Permanent = 0,
  Temporary = 1,
  Unsubscribed = 2,
  OversubscribedTopic = 3
};

// Message messages[1024]; // Wiadomości przechowywane przez serwer

typedef struct {
  char name[128];  // Nazwa klienta
  int client_id;   // ID klienta
  key_t queue_key; // Identyfikator kolejki odbiorczej klienta
} Client;

// Client Logged_in[16]; // Tablica zalogowanych klientów

typedef struct {
  int client_id;              // Identyfikator użytkownika
  enum SubscriptionType type; // Informacje o subskrybcjach danych klientów
  int duration;               // Pozostały czas trwania subskrybcji przejściowej
  int blocked_ids[16];        // Zablokowani użytkownicy
} SubInfo; // Struktura informacji o subskrybentach

typedef struct {
  int topic_id;         // Identyfikator tematu
  char topic_name[128]; // Nazwa tematu
  SubInfo subscriptions[16];
} Topic;


// Topic_ topics[128];

typedef struct {
  long type;      // Typ komunikatu
  char name[128]; // Nazwa klienta
  key_t queue_key;      // Identyfikator kolejki odbiorczej klienta
} LoginMessage;

typedef struct {
  long type;      // Typ komunikatu
  char name[128]; // Nazwa klienta
  int status;     // Stan logowania: 0 - błąd
} LoginStatus;

typedef struct {
  long type;                 // Typ komunikatu
  char name[128];            // Nazwa klienta
  int topic_id;              // Identyfikator tematu
  enum SubscriptionType sub; // Rodzaj subskrybcji
  int duration;              // Długość trwania subskrybcji
} SubscriptionMessage;

typedef SubscriptionMessage SubscriptionStatus;

typedef struct {
  long type;            // Typ komunikatu
  char name[128];       // Nazwa klienta
  char topic_name[128]; // Nazwa tematu
} NewTopicMessage;

typedef struct {
  long type;            // Typ komunikatu
  int topic_id;         // Identyfikator tematu = 0
  char topic_name[128]; // Nazwa tematu
} NewTopicStatus;

typedef struct {
  long type;       // Typ komunikatu
  int topic_id;    // Identyfikator tematu
  char name[128];  // Nazwa klienta
  char text[2048]; // Treść wiadomości
  int priority;    // Priorytet wiadomości
} Message;

typedef struct {
  long type;            // Typ komunikatu
  char name[128];       // Nazwa blokującego klienta
  char block_name[128]; // Nazwa blokowanego klienta
  int topic_id;         // Identyfikator tematu
} BlockUserMessage;

#endif // MESSAGE_TYPES
