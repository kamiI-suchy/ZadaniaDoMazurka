#include <arpa/inet.h> // Dolaczamy konwersje adresow i portow (inet_pton, htons).
#include <errno.h> // Dolaczamy stale bledow systemowych do diagnostyki wywolan.
#include <netinet/in.h> // Dolaczamy strukture sockaddr_in dla IPv4.
#include <stdio.h> // Dolaczamy standardowe I/O do wyswietlania wyniku klienta.
#include <stdlib.h> // Dolaczamy funkcje narzedziowe i kody EXIT_*.
#include <string.h> // Dolaczamy operacje na pamieci i napisach (memset, strlen).
#include <sys/socket.h> // Dolaczamy API gniazd POSIX do socket, sendto, recvfrom.
#include <unistd.h> // Dolaczamy funkcje POSIX do close.
#define SERVER_IP "127.0.0.1" // Definiujemy domyslny adres IP serwera (zmien na IP plytki przy testach sieciowych).
#define SERVER_PORT 5001 // Definiujemy port serwera UDP zgodny z implementacja serwera.
#define BUF_SIZE 256 // Definiujemy rozmiar bufora na odpowiedz serwera.
int main(void) { // Definiujemy glowna funkcje klienta UDP.
    int fd = socket(AF_INET, SOCK_DGRAM, 0); // Tworzymy gniazdo IPv4 typu datagramowego (UDP).
    if (fd == -1) { // Sprawdzamy, czy utworzenie gniazda nie zakonczylo sie bledem.
        perror("socket"); // Wypisujemy szczegoly bledu tworzenia gniazda.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu socket.
    struct sockaddr_in server_addr; // Tworzymy strukture adresu serwera UDP.
    memset(&server_addr, 0, sizeof(server_addr)); // Zerujemy cala strukture adresu serwera.
    server_addr.sin_family = AF_INET; // Ustawiamy rodzine adresu na IPv4.
    server_addr.sin_port = htons(SERVER_PORT); // Ustawiamy port serwera w kolejnosci bajtow sieciowych.
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) != 1) { // Konwertujemy tekstowy adres IP serwera na postac binarna.
        fprintf(stderr, "Niepoprawny adres IP serwera: %s\n", SERVER_IP); // Wypisujemy komunikat o niepoprawnym adresie.
        close(fd); // Zamykamy gniazdo klienta po bledzie konwersji adresu.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu inet_pton.
    const char *message = "Jan Kowalski"; // Definiujemy tekst imienia i nazwiska wysylany do serwera.
    if (sendto(fd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) { // Wysylamy datagram z tekstem do serwera.
        perror("sendto"); // Wypisujemy szczegoly bledu wysylania datagramu.
        close(fd); // Zamykamy gniazdo klienta po bledzie wysylki.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu sendto.
    char response[BUF_SIZE]; // Tworzymy bufor na odpowiedz od serwera.
    struct sockaddr_in from_addr; // Tworzymy strukture adresu nadawcy odpowiedzi.
    socklen_t from_len = sizeof(from_addr); // Ustawiamy rozmiar struktury adresowej nadawcy.
    ssize_t received = recvfrom(fd, response, sizeof(response) - 1, 0, (struct sockaddr *)&from_addr, &from_len); // Odbieramy odpowiedz serwera.
    if (received == -1) { // Sprawdzamy, czy odbior datagramu zakonczyl sie bledem.
        perror("recvfrom"); // Wypisujemy szczegoly bledu odbioru odpowiedzi.
        close(fd); // Zamykamy gniazdo klienta po bledzie odbioru.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu recvfrom.
    response[received] = '\0'; // Domykamy odebrany tekst znakiem NUL.
    printf("Odpowiedz serwera UDP: %s\n", response); // Wyswietlamy tekst zwrocony przez serwer wielkimi literami.
    fflush(stdout); // Oprozniamy bufor wyjscia, aby wynik byl od razu widoczny.
    close(fd); // Zamykamy gniazdo klienta po zakonczeniu komunikacji.
    return EXIT_SUCCESS; // Zwracamy kod sukcesu po poprawnym przebiegu programu.
} // Konczymy funkcje main klienta UDP.
