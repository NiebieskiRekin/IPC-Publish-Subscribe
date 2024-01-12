#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

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



//LOGOWANIE
typedef struct {
    long type; // Typ komunikatu
    char name[128]; // Nazwa klienta
    int msgid; // Identyfikator kolejki odbiorczej klienta
} LoginMessage;

typedef struct {
    char name[128]; // Nazwa klienta
    int msgid; // Identyfikator kolejki odbiorczej klienta
} Client; // Wewnętrzna reprezentacja klienta po stronie serwera


typedef struct {
    long type; // Typ komunikatu
    char name[128]; // Nazwa klienta
    int status; // Stan logowania: 0 - błąd, 1 - ok
} LoginStatus;


