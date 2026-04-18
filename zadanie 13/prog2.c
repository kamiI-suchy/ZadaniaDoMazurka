#include <errno.h> // Dolaczamy stale bledow systemowych, aby obslugiwac m.in. EINTR i EIDRM.
#include <stdio.h> // Dolaczamy wejscie/wyjscie standardowe, aby korzystac z printf i fflush.
#include <stdlib.h> // Dolaczamy funkcje narzedziowe, aby uzywac exit i kodow EXIT_*.
#include <sys/ipc.h> // Dolaczamy definicje IPC potrzebne do pamieci wspoldzielonej i semaforow.
#include <sys/sem.h> // Dolaczamy API semaforow System V (semget, semop).
#include <sys/shm.h> // Dolaczamy API pamieci wspoldzielonej System V (shmget, shmat, shmdt).
#define SHM_KEY 0x1301 // Definiujemy ten sam klucz pamieci wspoldzielonej co w producencie.
#define SEM_KEY 0x1302 // Definiujemy ten sam klucz zestawu semaforow co w producencie.
#define BUF_SIZE 256 // Definiujemy ten sam rozmiar bufora danych co u producenta.
#define SEM_EMPTY 0 // Definiujemy indeks semafora EMPTY (bufor pusty) wspolny dla obu procesow.
#define SEM_FULL 1 // Definiujemy indeks semafora FULL (bufor z danymi) wspolny dla obu procesow.
static void sem_op_retry_or_finish(int semid, int semnum, int delta) { // Definiujemy pomocnicza operacje semaforowa z obsluga typowych bledow.
    struct sembuf op; // Tworzymy strukture opisujaca operacje semafora do semop.
    op.sem_num = (unsigned short)semnum; // Ustawiamy numer semafora wewnatrz zestawu.
    op.sem_op = (short)delta; // Ustawiamy rodzaj operacji: -1 oczekiwanie, +1 zwolnienie/sygnal.
    op.sem_flg = 0; // Ustawiamy flagi na 0, aby operacja byla blokujaca.
    while (semop(semid, &op, 1) == -1) { // Powtarzamy semop, dopoki zwracany jest blad.
        if (errno == EINTR) { // Sprawdzamy, czy wywolanie zostalo przerwane sygnalem.
            continue; // Wznawiamy operacje po przerwaniu sygnalem.
        } // Konczymy obsluge EINTR.
        if (errno == EIDRM) { // Sprawdzamy, czy zestaw semaforow zostal usuniety przez producenta.
            printf("\nProducent zakonczyl prace i usunal zasoby IPC. Konsument konczy.\n"); // Informujemy uzytkownika, dlaczego konsument wychodzi.
            exit(EXIT_SUCCESS); // Konczymy konsumenta sukcesem po zakonczonej pracy producenta.
        } // Konczymy obsluge usuniecia zasobow IPC.
        perror("semop"); // Wypisujemy szczegoly innego bledu semop.
        exit(EXIT_FAILURE); // Konczymy proces bledu, bo synchronizacja jest niesprawna.
    } // Konczymy petle powtarzania semop.
} // Konczymy definicje funkcji sem_op_retry_or_finish.
int main(void) { // Definiujemy glowna funkcje konsumenta.
    int shmid = shmget(SHM_KEY, BUF_SIZE, 0666); // Otwieramy istniejacy segment pamieci wspoldzielonej utworzony przez producenta.
    if (shmid == -1) { // Sprawdzamy, czy segment pamieci jest dostepny.
        perror("shmget (uruchom najpierw prog1)"); // Wypisujemy podpowiedz i szczegoly bledu.
        return EXIT_FAILURE; // Konczymy program niepowodzeniem.
    } // Konczymy obsluge bledu shmget.
    const char *shm = (const char *)shmat(shmid, NULL, SHM_RDONLY); // Dolaczamy pamiec wspoldzielona tylko do odczytu.
    if (shm == (const char *)-1) { // Sprawdzamy, czy dolaczenie segmentu do przestrzeni procesu powiodlo sie.
        perror("shmat"); // Wypisujemy powod bledu dolaczenia pamieci wspoldzielonej.
        return EXIT_FAILURE; // Konczymy program niepowodzeniem.
    } // Konczymy obsluge bledu shmat.
    int semid = semget(SEM_KEY, 2, 0666); // Otwieramy istniejacy zestaw dwoch semaforow utworzony przez producenta.
    if (semid == -1) { // Sprawdzamy, czy zestaw semaforow jest dostepny.
        perror("semget (uruchom najpierw prog1)"); // Wypisujemy podpowiedz i szczegoly bledu semget.
        shmdt(shm); // Odlaczamy pamiec wspoldzielona, aby nie zostawic zasobu przypietego.
        return EXIT_FAILURE; // Konczymy program niepowodzeniem.
    } // Konczymy obsluge bledu semget.
    printf("Konsument uruchomiony i oczekuje na dane...\n"); // Informujemy, ze konsument pracuje i czeka na wiadomosci.
    fflush(stdout); // Oprozniamy bufor wyjscia, aby komunikat byl natychmiast widoczny.
    while (1) { // Uruchamiamy petle nieskonczona wymagana trescia zadania.
        sem_op_retry_or_finish(semid, SEM_FULL, -1); // Czekamy, az producent zasygnalizuje dostepnosc danych.
        printf("Odebrano: %s\n", shm); // Wyswietlamy tekst odczytany z pamieci wspoldzielonej.
        fflush(stdout); // Oprozniamy bufor wyjscia, aby natychmiast pokazac odebrany tekst.
        sem_op_retry_or_finish(semid, SEM_EMPTY, +1); // Sygnalizujemy producentowi, ze bufor ponownie jest pusty.
    } // Konczymy petle nieskonczona (w praktyce niedostepne bez sygnalu/zakonczenia procesu).
    shmdt(shm); // Odlaczamy pamiec wspoldzielona (kod pozostawiony dla pelnosci, zwykle nieosiagalny).
    return EXIT_SUCCESS; // Zwracamy kod sukcesu (teoretycznie po opuszczeniu petli).
} // Konczymy funkcje main konsumenta.
