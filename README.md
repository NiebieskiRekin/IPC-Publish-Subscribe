# IPC-Publish-Subscribe
Projekt zaliczeniowy - Programowanie Systemowe i Współbieżne

> Celem projektu jest implementacja systemu przekazywania (rozgłaszania) wiadomości do wszystkich procesów, które zasubskrybowały dany typ wiadomości (zarejestrowały się na ich odbiór). Aplikacja wykorzystuje mechanizm kolejek komunikatów. Projekt zawiera w sobie 2 podprogramy: klienta i serwera. Każdy klient może wysyłać i otrzymywać wiadomości do/od pozostałych użytkowników systemu. W wymianie wiadomości pomiędzy klientami zawsze pośredniczy serwer. Otrzymywane wiadomości wyświetlane są na ekranie.

## Kompilacja
Wymagany jest kompilator C17 lub nowszy, np. (GCC 8.1.0+, LLVM Clang 7.0.0+)

- Z użyciem `cmake >= 3.20`
  ```
  git clone https://github.com/NiebieskiRekin/IPC-Publish-Subscribe
  cd IPC-Publish-Subscribe
  cmake -S . -B build
  cd build
  cmake --build . --target klient serwer
  ```

- Ręcznie np. GCC
  ```
  gcc -Wall -Wextra -Wpedantic inf155965_155845_k.c -o build/klient
  gcc -Wall -Wextra -Wpedantic inf155965_155845_s.c -o build/serwer
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