#include <ctype.h> // Dolaczamy funkcje klasyfikacji znakow, aby uzyc toupper.
#include <errno.h> // Dolaczamy stale bledow systemowych do diagnostyki wywolan.
#include <stdio.h> // Dolaczamy standardowe I/O do wypisywania informacji i bledow.
#include <stdlib.h> // Dolaczamy funkcje narzedziowe do kodow EXIT_*.
#include <string.h> // Dolaczamy operacje na napisach do strcpy, memset, strlen.
#include <sys/socket.h> // Dolaczamy API gniazd POSIX do socket, bind, listen, accept, recv, send.
#include <sys/un.h> // Dolaczamy struktury gniazd domeny UNIX (sockaddr_un).
#include <unistd.h> // Dolaczamy funkcje POSIX do close i unlink.
#define SOCKET_PATH "/tmp/zad15_unix_socket" // Definiujemy sciezke pliku gniazda domeny UNIX.
#define BUF_SIZE 256 // Definiujemy rozmiar bufora do odbioru i odeslania tekstu.
static void to_uppercase(char *text) { // Definiujemy funkcje zamieniajaca tekst w miejscu na wielkie litery.
    for (size_t i = 0; text[i] != '\0'; ++i) { // Iterujemy po kolejnych znakach az do konca napisu.
        text[i] = (char)toupper((unsigned char)text[i]); // Zamieniamy biezacy znak na odpowiednik wielkiej litery.
    } // Konczymy petle iterujaca po znakach.
} // Konczymy definicje funkcji to_uppercase.
int main(void) { // Definiujemy glowna funkcje serwera zadania 15.
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0); // Tworzymy gniazdo domeny UNIX typu strumieniowego.
    if (server_fd == -1) { // Sprawdzamy, czy utworzenie gniazda nie zakonczylo sie bledem.
        perror("socket"); // Wypisujemy szczegoly bledu tworzenia gniazda.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu socket.
    struct sockaddr_un addr; // Tworzymy strukture adresu domeny UNIX.
    memset(&addr, 0, sizeof(addr)); // Zerujemy cala strukture adresu dla bezpiecznej inicjalizacji.
    addr.sun_family = AF_UNIX; // Ustawiamy rodzine adresu na domene UNIX.
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1); // Kopiujemy sciezke pliku gniazda do struktury adresu.
    unlink(SOCKET_PATH); // Usuwamy stary plik gniazda, jesli pozostal po poprzednim uruchomieniu.
    if (bind(server_fd, (const struct sockaddr *)&addr, sizeof(addr)) == -1) { // Podpinamy gniazdo do sciezki systemu plikow.
        perror("bind"); // Wypisujemy szczegoly bledu bind.
        close(server_fd); // Zamykamy deskryptor gniazda serwera po bledzie.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu bind.
    if (listen(server_fd, 5) == -1) { // Ustawiamy gniazdo serwera w tryb nasluchiwania polaczen.
        perror("listen"); // Wypisujemy szczegoly bledu listen.
        close(server_fd); // Zamykamy gniazdo serwera przy bledzie.
        unlink(SOCKET_PATH); // Usuwamy plik gniazda po bledzie inicjalizacji.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu listen.
    printf("Serwer UNIX (zad15) nasluchuje na %s\n", SOCKET_PATH); // Informujemy, gdzie serwer oczekuje na klientow.
    fflush(stdout); // Oprozniamy bufor wyjscia, aby komunikat byl od razu widoczny.
    while (1) { // Uruchamiamy petle serwera obslugujaca kolejne polaczenia klientow.
        int client_fd = accept(server_fd, NULL, NULL); // Akceptujemy kolejne przychodzace polaczenie klienta.
        if (client_fd == -1) { // Sprawdzamy, czy accept nie zwrocil bledu.
            if (errno == EINTR) { // Sprawdzamy, czy accept przerwal sygnal.
                continue; // Wracamy do ponownego oczekiwania na klienta.
            } // Konczymy obsluge EINTR.
            perror("accept"); // Wypisujemy szczegoly innego bledu accept.
            break; // Wychodzimy z petli serwera po krytycznym bledzie.
        } // Konczymy obsluge wyniku accept.
        char buffer[BUF_SIZE]; // Tworzymy bufor na odebrany tekst od klienta.
        ssize_t received = recv(client_fd, buffer, sizeof(buffer) - 1, 0); // Odbieramy dane z polaczonego gniazda klienta.
        if (received > 0) { // Sprawdzamy, czy odebrano dodatnia liczbe bajtow.
            buffer[received] = '\0'; // Dodatkowo domykamy napis znakiem NUL.
            to_uppercase(buffer); // Zamieniamy odebrany tekst na wielkie litery.
            if (send(client_fd, buffer, strlen(buffer), 0) == -1) { // Odsylamy zmieniony tekst do klienta.
                perror("send"); // Wypisujemy szczegoly bledu wysylki do klienta.
            } // Konczymy obsluge wyniku send.
        } else if (received == -1) { // Sprawdzamy, czy recv zakonczyl sie bledem.
            perror("recv"); // Wypisujemy szczegoly bledu odbioru danych.
        } // Konczymy obsluge recv.
        close(client_fd); // Zamykamy polaczenie z tym klientem po obsludze jednej wiadomosci.
    } // Konczymy petle glownego nasluchu serwera.
    close(server_fd); // Zamykamy glowne gniazdo serwera po wyjsciu z petli.
    unlink(SOCKET_PATH); // Usuwamy plik gniazda domeny UNIX podczas sprzatania.
    return EXIT_SUCCESS; // Zwracamy kod sukcesu po zamknieciu serwera.
} // Konczymy funkcje main serwera.
