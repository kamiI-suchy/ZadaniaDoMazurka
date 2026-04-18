#include <errno.h> // Dolaczamy stale bledow systemowych, aby obslugiwac np. EINTR.
#include <stdio.h> // Dolaczamy wejscie/wyjscie standardowe, aby uzywac printf, fgets, fflush.
#include <stdlib.h> // Dolaczamy funkcje narzedziowe, aby uzywac exit i EXIT_*.
#include <string.h> // Dolaczamy operacje na napisach, aby uzywac strcmp, strcspn, strncpy.
#include <sys/ipc.h> // Dolaczamy definicje IPC wymagane przez pamiec wspoldzielona i semafory.
#include <sys/sem.h> // Dolaczamy API semaforow System V (semget, semop, semctl).
#include <sys/shm.h> // Dolaczamy API pamieci wspoldzielonej System V (shmget, shmat, shmdt, shmctl).
#define SHM_KEY 0x1301 // Definiujemy staly klucz pamieci wspoldzielonej dla producenta i konsumenta.
#define SEM_KEY 0x1302 // Definiujemy staly klucz zestawu semaforow wspolnego dla obu procesow.
#define BUF_SIZE 256 // Definiujemy rozmiar bufora tekstowego przesylanego miedzy procesami.
#define SEM_EMPTY 0 // Definiujemy indeks semafora informujacego, ze bufor jest pusty.
#define SEM_FULL 1 // Definiujemy indeks semafora informujacego, ze bufor zawiera dane.
union semun { int val; struct semid_ds *buf; unsigned short *array; }; // Definiujemy union semun wymagany przez semctl w glibc.
static void sem_op_retry(int semid, int semnum, int delta) { // Definiujemy pomocnicza funkcje wykonujaca pojedyncza operacje semaforowa.
    struct sembuf op; // Tworzymy strukture opisujaca jedna operacje na semaforze.
    op.sem_num = (unsigned short)semnum; // Ustawiamy numer semafora wewnatrz zestawu semaforow.
    op.sem_op = (short)delta; // Ustawiamy wartosc zmiany semafora: -1 czekaj/zajmij, +1 zwolnij/sygnalizuj.
    op.sem_flg = 0; // Ustawiamy flagi operacji na 0, czyli zachowanie blokujace bez dodatkowych opcji.
    while (semop(semid, &op, 1) == -1) { // Powtarzamy wywolanie semop, dopoki zwraca blad.
        if (errno == EINTR) { // Sprawdzamy, czy semop zostal przerwany sygnalem.
            continue; // Wznawiamy operacje semaforowa po przerwaniu sygnalem.
        } // Konczymy blok warunku przerwania sygnalem.
        perror("semop"); // Wypisujemy szczegoly nieoczekiwanego bledu operacji semaforowej.
        exit(EXIT_FAILURE); // Konczymy proces kodem bledu, bo synchronizacja nie moze dzialac poprawnie.
    } // Konczymy petle powtarzania semop.
} // Konczymy definicje funkcji sem_op_retry.
int main(void) { // Definiujemy glowna funkcje producenta.
    int shmid = shmget(SHM_KEY, BUF_SIZE, IPC_CREAT | 0666); // Tworzymy lub otwieramy segment pamieci wspoldzielonej o rozmiarze BUF_SIZE.
    if (shmid == -1) { // Sprawdzamy, czy uzyskanie identyfikatora segmentu nie zakonczylo sie bledem.
        perror("shmget"); // Wypisujemy powod bledu tworzenia/otwierania pamieci wspoldzielonej.
        return EXIT_FAILURE; // Konczymy program niepowodzeniem.
    } // Konczymy obsluge bledu shmget.
    char *shm = (char *)shmat(shmid, NULL, 0); // Dolaczamy segment pamieci wspoldzielonej do przestrzeni adresowej procesu.
    if (shm == (char *)-1) { // Sprawdzamy, czy dolaczenie pamieci wspoldzielonej zakonczylo sie bledem.
        perror("shmat"); // Wypisujemy powod bledu dolaczenia segmentu.
        return EXIT_FAILURE; // Konczymy program niepowodzeniem.
    } // Konczymy obsluge bledu shmat.
    int semid = semget(SEM_KEY, 2, IPC_CREAT | 0666); // Tworzymy lub otwieramy zestaw dwoch semaforow potrzebnych do synchronizacji.
    if (semid == -1) { // Sprawdzamy, czy uzyskanie identyfikatora zestawu semaforow nie zakonczylo sie bledem.
        perror("semget"); // Wypisujemy powod bledu tworzenia/otwierania semaforow.
        shmdt(shm); // Odlaczamy pamiec wspoldzielona, aby nie zostawiac zasobu przypietego.
        return EXIT_FAILURE; // Konczymy program niepowodzeniem.
    } // Konczymy obsluge bledu semget.
    union semun arg; // Tworzymy zmienna union przekazywana do semctl przy inicjalizacji wartosci.
    arg.val = 1; // Ustawiamy wartosc poczatkowa semafora EMPTY na 1, czyli bufor na start jest pusty.
    if (semctl(semid, SEM_EMPTY, SETVAL, arg) == -1) { // Inicjalizujemy semafor EMPTY i sprawdzamy blad.
        perror("semctl SETVAL SEM_EMPTY"); // Wypisujemy powod bledu inicjalizacji semafora EMPTY.
        shmdt(shm); // Odlaczamy segment pamieci przy bledzie.
        shmctl(shmid, IPC_RMID, NULL); // Usuwamy segment pamieci, aby nie pozostawic osieroconego zasobu.
        semctl(semid, 0, IPC_RMID); // Usuwamy zestaw semaforow, aby posprzatac po bledzie.
        return EXIT_FAILURE; // Konczymy program niepowodzeniem.
    } // Konczymy obsluge bledu inicjalizacji EMPTY.
    arg.val = 0; // Ustawiamy wartosc poczatkowa semafora FULL na 0, czyli brak danych do odczytu.
    if (semctl(semid, SEM_FULL, SETVAL, arg) == -1) { // Inicjalizujemy semafor FULL i sprawdzamy blad.
        perror("semctl SETVAL SEM_FULL"); // Wypisujemy powod bledu inicjalizacji semafora FULL.
        shmdt(shm); // Odlaczamy segment pamieci przy bledzie.
        shmctl(shmid, IPC_RMID, NULL); // Usuwamy segment pamieci, aby nie pozostawic osieroconego zasobu.
        semctl(semid, 0, IPC_RMID); // Usuwamy zestaw semaforow, aby posprzatac po bledzie.
        return EXIT_FAILURE; // Konczymy program niepowodzeniem.
    } // Konczymy obsluge bledu inicjalizacji FULL.
    char input[BUF_SIZE]; // Tworzymy lokalny bufor na tekst pobierany od uzytkownika z klawiatury.
    printf("Producent uruchomiony. Wpisz tekst (lub 'quit' aby zakonczyc):\n"); // Informujemy uzytkownika, jak korzystac z programu producenta.
    fflush(stdout); // Wymuszamy natychmiastowe wyslanie komunikatu na ekran.
    while (1) { // Startujemy glowna petle producenta dzialajaca do momentu wpisania quit lub EOF.
        printf("> "); // Wyswietlamy prosty prompt oczekujacy na wpisanie tekstu.
        fflush(stdout); // Oprozniamy bufor wyjscia, aby prompt byl widoczny od razu.
        if (fgets(input, sizeof(input), stdin) == NULL) { // Odczytujemy linie z klawiatury i sprawdzamy koniec wejscia/blady.
            break; // Konczymy petle, gdy brak dalszych danych ze standardowego wejscia.
        } // Konczymy obsluge sytuacji bez danych wejciowych.
        input[strcspn(input, "\n")] = '\0'; // Usuwamy koncowy znak nowej linii, aby porownania i wysylka byly czyste.
        if (strcmp(input, "quit") == 0) { // Sprawdzamy, czy uzytkownik wpisal polecenie zakonczenia pracy.
            break; // Konczymy petle producenta po wpisaniu quit.
        } // Konczymy obsluge polecenia quit.
        sem_op_retry(semid, SEM_EMPTY, -1); // Czekamy, az bufor bedzie pusty i zajmujemy go do zapisu danych.
        strncpy(shm, input, BUF_SIZE - 1); // Kopiujemy wpisany tekst do pamieci wspoldzielonej z ograniczeniem dlugosci.
        shm[BUF_SIZE - 1] = '\0'; // Wymuszamy terminator NUL na koncu bufora dla bezpieczenstwa.
        sem_op_retry(semid, SEM_FULL, +1); // Sygnalizujemy konsumentowi, ze w buforze sa gotowe dane.
        printf("Wyslano: %s\n", input); // Potwierdzamy lokalnie, jaki tekst zostal wyslany do konsumenta.
        fflush(stdout); // Oprozniamy bufor wyjscia, aby potwierdzenie bylo od razu widoczne.
    } // Konczymy glowna petle producenta.
    shmdt(shm); // Odlaczamy segment pamieci wspoldzielonej od procesu producenta.
    shmctl(shmid, IPC_RMID, NULL); // Usuwamy segment pamieci wspoldzielonej z systemu IPC.
    semctl(semid, 0, IPC_RMID); // Usuwamy zestaw semaforow z systemu IPC.
    printf("Producent zakonczony.\n"); // Informujemy o poprawnym zakonczeniu pracy producenta.
    return EXIT_SUCCESS; // Zwracamy kod sukcesu do systemu operacyjnego.
} // Konczymy funkcje main producenta.
