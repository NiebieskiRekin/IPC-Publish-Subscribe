# IPC Publish-Subscribe
### Protokół projektu | Programowanie Systemowe i Współbieżne

### Autorzy:
- Tomasz Pawłowski 155965
- Jakub Kamieniarz 155845

## Opis funkcjonalności

### Pomocnicze struktury danych

```c
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

enum SubscriptionType { Permanent = 0, Temporary = 1, Unsubscribed = 2 };

typedef struct {
  int topic_id;         // Identyfikator tematu
  char topic_name[128]; // Nazwa tematu
} Topic;
```

### Logowanie
- do kolejki komunikatów serwera klient powinien wysłać komunikat w formie: 
  ```c
  typedef struct {
    long type;      // Typ komunikatu
    char name[128]; // Nazwa klienta
    int msgid;      // Identyfikator kolejki odbiorczej klienta
  } LoginMessage;
  LoginMessage m_login_user = {Login, "...", 0x123};
  ```
- pole *name* powinno być unikalne dla każdego klienta, ograniczone jest do 127 znaków ASCII
- pole *msgid* powinno zawierać identyfikator kolejki, gdzie będą wysyłane komunikaty klienta
- uruchomienie programu serwera podaje klucz kolejki komunikatów na ekranie do której należy wysłać komunikat
- serwer po otrzymaniu wiadomości weryfikuje podane informacje oraz wysyła potwierdzenie logowania:
  - w przypadku poprawnego logowania dodaje klienta do listy zalogowanych oraz wysyła komunikat formatu: 
    ```c
    typedef struct {
      long type;      // Typ komunikatu
      char name[128]; // Nazwa klienta
      int status;     // Stan logowania: 0 - błąd
    } LoginStatus;
    ```
  - o treści:
    ```c
    LoginStatus m_login_error = {.type = Login, .name = "...", .status = 0};
    ```
  - w przeciwnym przypadku wysyła komunikat powyższego formatu o treści: 
    ```c
    LoginStatus m_login_proper = {.type = Login, .name = "...", .status = 1};
    ```
  
### Rejestracja odbiorcy (Subskrybcja tematu)
- do kolejki komunikatów serwera klient wysyła komunikat w formacie:
  ```c
  typedef struct {
    long type;                 // Typ komunikatu
    char name[128];            // Nazwa klienta
    int topic_id;              // Identyfikator tematu
    enum SubscriptionType sub; // Rodzaj subskrybcji
    int duration;              // Długość trwania subskrybcji
  } SubscriptionMessage;
  ```
  - dla subskrypcji przejściowej (np. temat 123 na 4 wiadomości)
    ```c
    SubscriptionMessage m_sub_temp = {Subscription, "...", 123, Temporary, 4};
    ```
  - dla subskrybcji trwałej (np. temat 123), pole *duration* ignorowane
    ```c
    SubscriptionMessage m_sub_perm = {Subscription, "...", 123, Permanent};
    ```
- Serwer przechowuje informacje o zasubskrybowanych tematach dla każdego klienta w formie __TODO__
- Jeśli dany klient posiada już subskrypcję tematu: (obecna &rarr; wysłana)
  - temp &rarr; temp: przedłużenie o *duration*, bądź skrócenie dla ujemnych wartości
  - temp &rarr; perm: zamiana na *Permanent*
  - perm &rarr; perm: brak zmian
  - perm &rarr; temp: zamiana na *Temporary* o długości *duration*
- Aby odsubskrybować temat (np. 123) należy wysłać komunikat:
  ```c
  SubscriptionMessage m_unsub = {Subscription, "...", 123, Unsubscribed};
  ```
- Serwer po otrzymaniu wiadomości odsyła informację zwrotną, która odzwierciedla obecny stan subskrypcji tematu:
  ```c
  typedef SubscriptionMessage SubscriptionStatus;
  SubscriptionStatus m_sub_stat = {Subscription, "...", 123, Unsubscribed, -1};
  ```


### Rejestracja typu wiadomości (tematu)
- do kolejki komunikatów serwera klient wysyła komunikat:
  ```c
  typedef struct {
      long type;            // Typ komunikatu
      char name[128];       // Nazwa klienta
      char topic_name[128]; // Nazwa tematu
  } NewTopicMessage;
  NewTopicMessage m_topic = {NewTopic, "...", "..."};
  ```
- Nazwa tematu powinna być unikatowa dla każdego tematu, składa się wyłącznie ze znaków ASCII o maksymalnej długości 127 znaków. 
- następnie serwer sprawdza czy dany temat już istnieje
    - do klienta odsyłany jest komunikat o formacie:

      ```c
      typedef struct {
        long type;            // Typ komunikatu
        int topic_id;         // Identyfikator tematu = 0
        char topic_name[128]; // Nazwa tematu
      } NewTopicStatus;
      ```

    - jeśli tak, to:

      ```c
      NewTopicStatus m_top_stat_error = {NewTopic, 0, "..."};
      ```

    - jeśli nie, to do klienta odsyłany jest komunikat (poprzez wysłanie komunikatu do kolejki):

      ```c
      NewTopicStatus m_top_stat_proper = {NewTopic, 123, "..."};
      ```
    
## Rozgłoszenie nowej wiadomości
- Klient wysyła komunikat do serwera:
  ```c
  typedef struct {
    long type;       // Typ komunikatu
    int topic_id;    // Identyfikator tematu
    char name[128];  // Nazwa klienta
    char text[2048]; // Treść wiadomości
    int priority;    // Priorytet wiadomości
  } Message;
  Message m_message = {SendMessage, 123, "...", "Hello, World!", 7};
  ```
- Wiadomości może są zapisywane w bazie __TODO__ 
- Następnie serwer rozsyła wiadomość do odpowiednich subskrybentów tematu o ile autor wiadmości nie został wcześniej zablokowany przez danego subskybenta.

## Odbiór wiadomości w sposób synchroniczny (blokujący)
- Wiadomości kontrolne:
  - status logowania,
  - status subskrybcji,
  - status rejestracji nowego tematu
  - status zablokowania użytkownika 
## Odbiór wiadomości w sposób asynchroniczny
- wiadomości odbierane przez serwer
- wiadomości dotyczące tematu subskrybowanego przez użytkownika
 
## Zablokowanie użytkownika
- format danych:
  ```c
  typedef struct {
    long type;            // Typ komunikatu
    char name[128];       // Nazwa blokującego klienta
    char block_name[128]; // Nazwa blokowanego klienta
    int topic_id;         // Identyfikator tematu
  } BlockUserMessage;
  ```
- na dany temat
  ```c
  BlockUserMessage m_block_user_by_topic = {BlockUser, "...", "...", 123};
  ```
- wszędzie
  ```c
  BlockUserMessage m_block_global = {BlockUser, "...", "...", 0};
  ```

## Lista dostępnych tematów - dodatkowo
- Zapytanie
  ```c
  typedef struct {
    long type;      // Typ komunikatu
    char name[128]; // Nazwa klienta
  } TopicsRequestMessage;
  TopicsRequestMessage m_request_topics = {TopicsRequest, "..."};
  ```
- Odpowiedź 1.
  ```c
  typedef struct {
    long type;                      // Typ komunikatu
    unsigned long number_of_topics; // Liczba dostępnych tematów
  } TopicsNumberMessage;
  TopicsNumberMessage m_n_topics = {TopicsNumber, 2};
  ```
- Odpowiedź 2.
  ```c
  typedef struct {
    long type;      // Typ komunikatu
    Topic topics[]; // Tablica Tematów
  } AvailableTopicsMessage;

  AvailableTopicsMessage m_available_topics = {
      .type = AvailableTopics,
      .topics = {{1, "Hello, World!"}, {2, "Merry Chistmas!"}}};
  ```
