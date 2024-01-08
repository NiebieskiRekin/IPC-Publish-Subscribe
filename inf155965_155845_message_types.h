// ┌────────────────────────┐
// │IPC Publish-Subscribe   │
// ├────────────────────────┤
// │Message types           │
// ├────────────────────────┤
// │Tomasz Pawłowski 155965 │
// │Jakub Kamieniarz 155845 │
// └────────────────────────┘

enum MessageType {
  Login = 1,
  Subscription = 2,
  NewTopic = 3,
  SendMessage = 4,
  BlockUser = 5,
  AvailableTopics = 6,
  TopicsNumber = 7,
  TopicsRequest = 8,
};

enum SubscriptionType {
  Permanent = 0,
  Temporary = 1,
  Unsubscribed = 2,
  OversubscribedTopic = 3
};

// Message messages[1024]; // Wiadomości przechowywane przez serwer

typedef struct {
  char name[128]; // Nazwa klienta
  int msgid;      // Identyfikator kolejki odbiorczej klienta
} Client;

// Client Logged_in[16]; // Tablica zalogowanych klientów

typedef struct {
  int topic_id;         // Identyfikator tematu
  char topic_name[128]; // Nazwa tematu
} Topic;

typedef struct {
  int topic_id;                      // Identyfikator tematu
  char topic_name[128];              // Nazwa tematu
  unsigned int number_of_subscibers; // Liczba klientów subskrybujących temat
  struct {
    Client *subscriber;    // Tablica klientów subskrybujących temat
    SubscriptionType type; // Informacje o subskrybcjach danych klientów
    int duration;          // Pozostały czas trwania subskrybcji przejściowej
  } subscriber_info[16];
} Topic_;

// Topic_ topics[128];

typedef struct {
  long type;      // Typ komunikatu
  char name[128]; // Nazwa klienta
  int msgid;      // Identyfikator kolejki odbiorczej klienta
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

typedef struct {
  long type;      // Typ komunikatu
  char name[128]; // Nazwa klienta
} TopicsRequestMessage;

typedef struct {
  long type;                      // Typ komunikatu
  unsigned long number_of_topics; // Liczba dostępnych tematów
} TopicsNumberMessage;

typedef struct {
  long type;      // Typ komunikatu
  Topic topics[]; // Tablica Tematów
} AvailableTopicsMessage;
