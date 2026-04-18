# Ogólne omówienie zadań 13-16

## Zadanie 13 (pamięć współdzielona + semafory)
- `prog1.c` to **producent**: czyta tekst z klawiatury i zapisuje go do pamięci współdzielonej.
- `prog2.c` to **konsument**: czeka na dane i wyświetla je w swoim terminalu.
- Synchronizacja jest zrobiona semaforami System V:
  - `SEM_EMPTY` mówi, że bufor jest pusty i producent może pisać.
  - `SEM_FULL` mówi, że bufor jest pełny i konsument może czytać.

## Zadanie 14 (dwa semafory, blokada i zakleszczenie)
- Jeden program uruchamiany w dwóch instancjach.
- Każda instancja może zajmować i zwalniać dwa semafory (`1/q` oraz `2/w`).
- Program pozwala pokazać:
  - **blokowanie**: druga instancja czeka na zasób zajęty przez pierwszą;
  - **zakleszczenie**: każda instancja trzyma inny semafor i czeka na drugi.
- Po odblokowaniu z oczekiwania program wypisuje imię i nazwisko studenta i kończy działanie.

## Zadanie 15 (gniazda domeny UNIX)
- `server.c` tworzy gniazdo `AF_UNIX` typu `SOCK_STREAM` pod ścieżką `/tmp/zad15_unix_socket`.
- `client.c` łączy się do serwera i wysyła tekst (imię i nazwisko).
- Serwer odbiera tekst, zamienia go na wielkie litery i odsyła do klienta.

## Zadanie 16 (klient-serwer UDP)
- `server.c` uruchamia gniazdo UDP (`AF_INET`, `SOCK_DGRAM`) na porcie `5001` (czyli > 4000).
- `client.c` wysyła datagram z imieniem i nazwiskiem do serwera.
- Serwer odsyła tekst wielkimi literami.
- Serwer działa w pętli nieskończonej.
- Aby pokazać działanie trzykrotnie, uruchom klienta trzy razy pod rząd.

## Najważniejsze funkcje użyte w kodzie

### Wejście/wyjście i napisy
- `printf(...)` — wypisuje tekst na standardowe wyjście.
- `fprintf(stderr, ...)` — wypisuje komunikaty błędów na standardowe wyjście błędów.
- `perror("nazwa")` — wypisuje ostatni błąd systemowy (`errno`) z prefiksem.
- `fflush(stdout)` — natychmiast opróżnia bufor wyjścia; dzięki temu komunikat od razu pojawia się na ekranie.
- `fgets(buf, size, stdin)` — czyta jedną linię z wejścia (np. klawiatury).
- `strcmp(a, b)` — porównuje dwa napisy.
- `strcspn(text, "\n")` — znajduje pozycję pierwszego znaku nowej linii.
- `strncpy(dst, src, n)` — kopiuje do `n` znaków.
- `strlen(text)` — zwraca długość napisu bez `\0`.
- `memset(ptr, 0, size)` — zeruje pamięć.
- `toupper(c)` — zamienia znak na wielką literę.

### IPC System V (zad. 13 i 14)
- `shmget(key, size, flags)` — tworzy/otwiera segment pamięci współdzielonej.
- `shmat(shmid, ..., ...)` — dołącza segment pamięci do procesu.
- `shmdt(ptr)` — odłącza segment pamięci od procesu.
- `shmctl(shmid, IPC_RMID, ...)` — usuwa segment pamięci z systemu.
- `semget(key, nsems, flags)` — tworzy/otwiera zestaw semaforów.
- `semop(semid, ops, nops)` — wykonuje operacje P/V (zajęcie/zwolnienie) na semaforach.
- `semctl(semid, semnum, cmd, arg)` — ustawia/usuwa semafory (np. `SETVAL`, `IPC_RMID`).

### Gniazda UNIX i sieciowe (zad. 15 i 16)
- `socket(domain, type, protocol)` — tworzy gniazdo.
- `bind(fd, addr, len)` — przypina gniazdo do adresu.
- `listen(fd, backlog)` — przełącza gniazdo serwera TCP/UNIX stream w tryb nasłuchu.
- `accept(fd, ...)` — przyjmuje połączenie klienta.
- `connect(fd, addr, len)` — łączy klienta z serwerem.
- `send(...)` / `recv(...)` — wysyła/odbiera dane na połączeniu strumieniowym.
- `sendto(...)` / `recvfrom(...)` — wysyła/odbiera datagramy UDP.
- `close(fd)` — zamyka deskryptor gniazda.
- `unlink(path)` — usuwa plik gniazda UNIX z systemu plików.
- `htons(...)`, `htonl(...)` — zamieniają kolejność bajtów hosta na sieciową.
- `inet_pton(...)` — konwertuje tekstowy adres IP do postaci binarnej.
- `inet_ntop(...)` — konwertuje binarny adres IP do postaci tekstowej.

## Jak uruchamiać (Linux)

### Zadanie 13
1. Terminal A: skompiluj `prog1.c`, uruchom `prog1`.
2. Terminal B: skompiluj `prog2.c`, uruchom `prog2`.
3. Wpisuj tekst w `prog1`; `prog2` będzie go odbierał.

### Zadanie 14
1. Uruchom dwie instancje `prog` w dwóch terminalach.
2. Używaj klawiszy `1`, `q`, `2`, `w`, `x` zgodnie z instrukcją w programie.

### Zadanie 15
1. Uruchom `server`.
2. Uruchom `client`.
3. Klient wyświetli odpowiedź serwera wielkimi literami.

### Zadanie 16
1. Uruchom `server`.
2. Uruchom `client` trzy razy.
3. Każde uruchomienie klienta pokaże odpowiedź wielkimi literami.
