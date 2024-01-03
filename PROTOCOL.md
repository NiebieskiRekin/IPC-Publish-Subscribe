# IPC Publish-Subscribe
## Protokół projektu | Programowanie Systemowe i Współbieżne

### Autorzy:
- Tomasz Pawłowski 155965
- Jakub Kamieniarz 155845

## Opis funkcjonalności


```c
enum Action{
  Login,
  Subscribe,
  NewTopic,
  Message
};

enum SubscriptionType{
  Permanent,
  Temporary
}

```

### Logowanie
- do kolejki komunikatów serwera klient powinien wysłać komunikat o treści
```c
struct msgbuf{
  long type = 0;
  Action act = Login;
  pid_t id = getpid();
  char name[] = "..."
};
```
- pole *name* powinno być unikalne dla każdego klienta
- uruchomienie programu serwera podaje klucz kolejki komunikatów na ekranie
- serwer po otrzymaniu wiadomości dodaje klienta do listy zalogowanych

### Rejestracja odbiorcy
- do kolejki komunikatów serwera klient wysyła komunikat:
  - dla subskrypcji przejściowej
  ```c
  struct msgbuf{
    long type = 0;
    Action act = Subscribe;
    char name[] = "...";
    SubscriptionType sub = Temporary;
    u_int topic_id = ...; 
    int duration = ...;
  };
  ```
  - dla subskrypcji trwałej
  ```c
  struct msgbuf{
    long type = 0;
    Action act = Subscribe;
    char name[] = "...";
    SubscriptionType sub = Permanent;
    u_int topic_id = ...; 
  };
  ```
- Serwer przechowuje informację o zasubskrybowanych wiadomościach w jakimś odpowiedniku hashmapy
- Jeśli dany klient posiada już subskrypcję tematu: (obecne -> wysłane)
  - temp -> temp: przedłużenie o *duration*
  - temp -> perm: zamiana na *Permanent*
  - perm -> perm: brak zmian
  - perm -> temp: zamiana na *Temporary* o długości *duration*
- Aby natychmiast odsubskrybować temat należy wysłać komunikat:
  ```c
  struct msgbuf{
    long type = 0;
    Action act = Subscribe;
    char name[] = "...";
    SubscriptionType sub = Temporary;
    u_int topic_id = ...;
    int duration = 0; 
  };
  ```

### Rejestracja typu wiadomości (tematu)
- do kolejki komunikatów serwera klient wysyła komunikat:
  ```c
  struct msgbuf{
    long type = 0;
    Action act = NewTopic;
    char name[] = "...";
    char topic_name[] = "...";
  };
  ```
- następnie 



