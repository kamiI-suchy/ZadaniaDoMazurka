#include <errno.h> // Dolaczamy stale bledow systemowych do diagnostyki wywolan gniazd.
#include <stdio.h> // Dolaczamy standardowe I/O do komunikatow i wyswietlenia wyniku.
#include <stdlib.h> // Dolaczamy funkcje narzedziowe i kody EXIT_*.
#include <string.h> // Dolaczamy funkcje napisowe do memset i strlen.
#include <sys/socket.h> // Dolaczamy API gniazd POSIX do socket, connect, send, recv.
#include <sys/un.h> // Dolaczamy strukture sockaddr_un dla gniazd domeny UNIX.
#include <unistd.h> // Dolaczamy funkcje POSIX do close.
#define SOCKET_PATH "/tmp/zad15_unix_socket" // Definiujemy sciezke gniazda serwera domeny UNIX.
#define BUF_SIZE 256 // Definiujemy rozmiar bufora klienta.
int main(void) { // Definiujemy glowna funkcje klienta zadania 15.
    int fd = socket(AF_UNIX, SOCK_STREAM, 0); // Tworzymy gniazdo domeny UNIX typu strumieniowego.
    if (fd == -1) { // Sprawdzamy, czy utworzenie gniazda nie zakonczylo sie bledem.
        perror("socket"); // Wypisujemy szczegoly bledu tworzenia gniazda.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu socket.
    struct sockaddr_un addr; // Tworzymy strukture adresu serwera domeny UNIX.
    memset(&addr, 0, sizeof(addr)); // Zerujemy strukture przed wypelnieniem pol.
    addr.sun_family = AF_UNIX; // Ustawiamy rodzine adresu na domene UNIX.
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1); // Kopiujemy sciezke gniazda serwera do struktury adresu.
    if (connect(fd, (const struct sockaddr *)&addr, sizeof(addr)) == -1) { // Nawiazujemy polaczenie z serwerem.
        perror("connect"); // Wypisujemy szczegoly bledu nawiazywania polaczenia.
        close(fd); // Zamykamy gniazdo klienta po bledzie connect.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu connect.
    const char *message = "Jan Kowalski"; // Definiujemy tekst imienia i nazwiska wysylany do serwera.
    if (send(fd, message, strlen(message), 0) == -1) { // Wysylamy tekst do serwera przez polaczenie strumieniowe.
        perror("send"); // Wypisujemy szczegoly bledu wysylania.
        close(fd); // Zamykamy gniazdo klienta po bledzie wysylki.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu send.
    char response[BUF_SIZE]; // Tworzymy bufor na odpowiedz serwera.
    ssize_t received = recv(fd, response, sizeof(response) - 1, 0); // Odbieramy odpowiedz serwera z tekstem wielkimi literami.
    if (received == -1) { // Sprawdzamy, czy odbior danych zakonczyl sie bledem.
        perror("recv"); // Wypisujemy szczegoly bledu odbioru odpowiedzi.
        close(fd); // Zamykamy gniazdo klienta po bledzie odbioru.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu recv.
    response[received] = '\0'; // Domykamy odebrany ciag znakiem NUL.
    printf("Odpowiedz serwera: %s\n", response); // Wyswietlamy tekst zwrocony przez serwer.
    fflush(stdout); // Oprozniamy bufor wyjscia, aby wynik byl od razu widoczny.
    close(fd); // Zamykamy polaczenie klienta po zakonczeniu komunikacji.
    return EXIT_SUCCESS; // Zwracamy kod sukcesu po poprawnym przebiegu programu.
} // Konczymy funkcje main klienta.
