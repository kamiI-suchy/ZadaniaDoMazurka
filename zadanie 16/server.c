#include <arpa/inet.h> // Dolaczamy konwersje adresow i portow sieciowych (htonl, htons, inet_ntop).
#include <ctype.h> // Dolaczamy funkcje klasyfikacji znakow, aby uzywac toupper.
#include <errno.h> // Dolaczamy stale bledow systemowych do diagnostyki wywolan.
#include <netinet/in.h> // Dolaczamy definicje struktur IPv4 (sockaddr_in).
#include <stdio.h> // Dolaczamy standardowe I/O do komunikatow i logow serwera.
#include <stdlib.h> // Dolaczamy funkcje narzedziowe i kody EXIT_*.
#include <string.h> // Dolaczamy operacje na pamieci i napisach (memset).
#include <sys/socket.h> // Dolaczamy API gniazd POSIX do socket, bind, recvfrom, sendto.
#include <unistd.h> // Dolaczamy funkcje POSIX do close.
#define SERVER_PORT 5001 // Definiujemy port UDP powyzej 4000 zgodnie z trescia zadania.
#define BUF_SIZE 256 // Definiujemy rozmiar bufora danych dla serwera UDP.
static void to_uppercase(char *text) { // Definiujemy funkcje zamieniajaca napis na wielkie litery w miejscu.
    for (size_t i = 0; text[i] != '\0'; ++i) { // Iterujemy po kolejnych znakach napisu az do terminatora.
        text[i] = (char)toupper((unsigned char)text[i]); // Zamieniamy biezacy znak na odpowiednik wielkiej litery.
    } // Konczymy petle po wszystkich znakach.
} // Konczymy definicje funkcji to_uppercase.
int main(void) { // Definiujemy glowna funkcje serwera UDP.
    int fd = socket(AF_INET, SOCK_DGRAM, 0); // Tworzymy gniazdo IPv4 typu datagramowego (UDP).
    if (fd == -1) { // Sprawdzamy, czy utworzenie gniazda nie zakonczylo sie bledem.
        perror("socket"); // Wypisujemy szczegoly bledu tworzenia gniazda.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu socket.
    struct sockaddr_in server_addr; // Tworzymy strukture adresu lokalnego serwera.
    memset(&server_addr, 0, sizeof(server_addr)); // Zerujemy cala strukture przed konfiguracja.
    server_addr.sin_family = AF_INET; // Ustawiamy rodzine adresu na IPv4.
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Ustawiamy nasluch na wszystkich interfejsach sieciowych.
    server_addr.sin_port = htons(SERVER_PORT); // Ustawiamy port serwera w kolejnosci bajtow sieciowych.
    if (bind(fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) { // Podpinamy gniazdo UDP do lokalnego adresu i portu.
        perror("bind"); // Wypisujemy szczegoly bledu bind.
        close(fd); // Zamykamy gniazdo po nieudanym bind.
        return EXIT_FAILURE; // Konczymy program kodem bledu.
    } // Konczymy obsluge bledu bind.
    printf("Serwer UDP (zad16) nasluchuje na porcie %d\n", SERVER_PORT); // Informujemy, na jakim porcie dziala serwer.
    fflush(stdout); // Oprozniamy bufor wyjscia, aby komunikat byl natychmiast widoczny.
    while (1) { // Uruchamiamy petle nieskonczona serwera zgodnie z trescia zadania.
        char buffer[BUF_SIZE]; // Tworzymy bufor na odebrany datagram klienta.
        struct sockaddr_in client_addr; // Tworzymy strukture adresu nadawcy datagramu.
        socklen_t client_len = sizeof(client_addr); // Ustawiamy dlugosc struktury adresowej klienta.
        ssize_t received = recvfrom(fd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_len); // Odbieramy datagram wraz z adresem nadawcy.
        if (received == -1) { // Sprawdzamy, czy odbior datagramu zakonczyl sie bledem.
            if (errno == EINTR) { // Sprawdzamy, czy recvfrom zostal przerwany sygnalem.
                continue; // Wracamy do petli i ponawiamy oczekiwanie na datagram.
            } // Konczymy obsluge EINTR.
            perror("recvfrom"); // Wypisujemy szczegoly innego bledu odbioru datagramu.
            continue; // Kontynuujemy dzialanie serwera mimo blednego pojedynczego odbioru.
        } // Konczymy obsluge wyniku recvfrom.
        buffer[received] = '\0'; // Domykamy odebrany tekst znakiem NUL.
        to_uppercase(buffer); // Konwertujemy tekst na wielkie litery przed odpowiedzia.
        if (sendto(fd, buffer, (size_t)received, 0, (const struct sockaddr *)&client_addr, client_len) == -1) { // Odsylamy zmieniony datagram do nadawcy.
            perror("sendto"); // Wypisujemy szczegoly bledu odeslania odpowiedzi.
            continue; // Wracamy do kolejnego obrotu petli serwera.
        } // Konczymy obsluge wyniku sendto.
        char ip_buf[INET_ADDRSTRLEN]; // Tworzymy bufor tekstowy na adres IPv4 klienta.
        const char *ip_text = inet_ntop(AF_INET, &client_addr.sin_addr, ip_buf, sizeof(ip_buf)); // Zamieniamy binarny adres klienta na postac tekstowa.
        if (ip_text == NULL) { // Sprawdzamy, czy konwersja adresu IP nie zakonczyla sie bledem.
            ip_text = "(nieznany_adres)"; // Ustawiamy bezpieczny napis zastepczy, gdy konwersja sie nie powiedzie.
        } // Konczymy obsluge bledu konwersji adresu.
        printf("Obsłużono klienta %s:%d, tekst: %s\n", ip_text, ntohs(client_addr.sin_port), buffer); // Logujemy adres klienta i przetworzony tekst.
        fflush(stdout); // Oprozniamy bufor wyjscia, aby log byl natychmiast widoczny.
    } // Konczymy petle serwera (w praktyce dziala bez konca).
    close(fd); // Zamykamy gniazdo serwera (kod formalny, zwykle nieosiagalny).
    return EXIT_SUCCESS; // Zwracamy kod sukcesu (teoretycznie po zakonczeniu petli).
} // Konczymy funkcje main serwera UDP.
