# IPC-Publish-Subscribe
Projekt zaliczeniowy - Programowanie Systemowe i Współbieżne

> Celem projektu jest implementacja systemu przekazywania (rozgłaszania) wiadomości do wszystkich procesów, które zasubskrybowały dany typ wiadomości (zarejestrowały się na ich odbiór). Aplikacja wykorzystuje mechanizm kolejek komunikatów. Projekt zawiera w sobie 2 podprogramy: klienta i serwera. Każdy klient może wysyłać i otrzymywać wiadomości do/od pozostałych użytkowników systemu. W wymianie wiadomości pomiędzy klientami zawsze pośredniczy serwer. Otrzymywane wiadomości wyświetlane są na ekranie.

## Kompilacja
```
git clone https://github.com/NiebieskiRekin/IPC-Publish-Subscribe
cd IPC-Publish-Subscribe
cmake -S . -B build
cd build
cmake --build . --target klient serwer
```

## Uruchomienie
W osobnych terminalach:
- program serwera
  ```
  ./build/serwer
  ```

- program klienta
  ```
  ./build/klient
  ```

Opis działania programu znajduje się w protokole.

## Debugowanie w VSCode itp.
Do programu dołączona jest również konfiguracja debuggera do IDE bazujących na kodzie Visual Studio Code oraz instrukcja korzystania znajdująca się w katalogu `/docs`.