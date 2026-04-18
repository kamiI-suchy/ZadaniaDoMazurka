#include <errno.h> // Dolaczamy stale bledow systemowych, aby obslugiwac np. EEXIST, EAGAIN, EINTR.
#include <stdio.h> // Dolaczamy wejscie/wyjscie standardowe, aby korzystac z printf, fgets, fflush.
#include <stdlib.h> // Dolaczamy funkcje narzedziowe, aby uzywac exit i EXIT_*.
#include <sys/ipc.h> // Dolaczamy definicje IPC wymagane przez semafory System V.
#include <sys/sem.h> // Dolaczamy API semaforow System V: semget, semop, semctl.
#include <unistd.h> // Dolaczamy funkcje POSIX, aby korzystac z getpid.
#define SEM_KEY 0x1401 // Definiujemy klucz wspolnego zestawu semaforow dla obu instancji programu.
#define SEM1_IDX 0 // Definiujemy indeks pierwszego semafora jednoelementowego.
#define SEM2_IDX 1 // Definiujemy indeks drugiego semafora jednoelementowego.
#define STUDENT_NAME "Jan Kowalski" // Definiujemy tekst imienia i nazwiska wyswietlany po odblokowaniu.
union semun { int val; struct semid_ds *buf; unsigned short *array; }; // Definiujemy union semun wymagany przez semctl.
static int semid = -1; // Przechowujemy globalnie identyfikator zestawu semaforow dla funkcji pomocniczych.
static void sem_wait_idx(int idx) { // Definiujemy funkcje blokujacego zajecia semafora o podanym indeksie.
    struct sembuf op; // Tworzymy strukture opisujaca operacje semafora.
    op.sem_num = (unsigned short)idx; // Wskazujemy numer semafora w zestawie.
    op.sem_op = -1; // Ustawiamy operacje dekrementacji, czyli zajecie zasobu.
    op.sem_flg = 0; // Ustawiamy tryb blokujacy bez dodatkowych flag.
    while (semop(semid, &op, 1) == -1) { // Powtarzamy semop, dopoki wystepuje blad.
        if (errno == EINTR) { // Sprawdzamy, czy operacje przerwal sygnal.
            continue; // Wznawiamy oczekiwanie po przerwaniu sygnalem.
        } // Konczymy obsluge EINTR.
        perror("semop"); // Wypisujemy szczegoly nieoczekiwanego bledu semop.
        exit(EXIT_FAILURE); // Konczymy proces z bledem, bo semafory nie dzialaja poprawnie.
    } // Konczymy petle ponawiania semop.
} // Konczymy definicje funkcji sem_wait_idx.
static int sem_trywait_idx(int idx) { // Definiujemy funkcje nieblokujacego zajecia semafora o podanym indeksie.
    struct sembuf op; // Tworzymy strukture pojedynczej operacji semafora.
    op.sem_num = (unsigned short)idx; // Ustawiamy numer semafora do modyfikacji.
    op.sem_op = -1; // Ustawiamy dekrementacje odpowiadajaca probie zajecia zasobu.
    op.sem_flg = IPC_NOWAIT; // Ustawiamy flage nieblokujaca, aby natychmiast wrocic gdy semafor zajety.
    if (semop(semid, &op, 1) == 0) { // Sprawdzamy, czy zajecie semafora zakonczylo sie sukcesem.
        return 0; // Zwracamy sukces nieblokujacego zajecia zasobu.
    } // Konczymy obsluge sukcesu semop.
    if (errno == EAGAIN) { // Sprawdzamy, czy semafor byl zajety i operacja nie mogla byc wykonana teraz.
        return -1; // Zwracamy kod informujacy, ze semafor jest zajety.
    } // Konczymy obsluge przypadku zajetego semafora.
    perror("semop"); // Wypisujemy szczegoly innego nieoczekiwanego bledu semop.
    exit(EXIT_FAILURE); // Konczymy proces z bledem krytycznym.
} // Konczymy definicje funkcji sem_trywait_idx.
static void sem_post_idx(int idx) { // Definiujemy funkcje zwolnienia semafora o podanym indeksie.
    struct sembuf op; // Tworzymy strukture opisujaca operacje zwolnienia.
    op.sem_num = (unsigned short)idx; // Ustawiamy numer semafora do zwolnienia.
    op.sem_op = +1; // Ustawiamy inkrementacje odpowiadajaca zwolnieniu zasobu.
    op.sem_flg = 0; // Ustawiamy brak dodatkowych flag.
    if (semop(semid, &op, 1) == -1) { // Wykonujemy zwolnienie i sprawdzamy, czy wystapil blad.
        perror("semop"); // Wypisujemy szczegoly bledu zwalniania semafora.
        exit(EXIT_FAILURE); // Konczymy proces z bledem krytycznym.
    } // Konczymy obsluge bledu sem_post.
} // Konczymy definicje funkcji sem_post_idx.
static void acquire_sem(int idx) { // Definiujemy logike zajmowania semafora z wymaganym zachowaniem po odblokowaniu.
    if (sem_trywait_idx(idx) == 0) { // Sprawdzamy, czy udalo sie zajac semafor bez czekania.
        printf("Semafor %d zajety przez ten proces.\n", idx + 1); // Informujemy, ze biezaca instancja trzyma wybrany semafor.
        fflush(stdout); // Oprozniamy bufor wyjscia, aby komunikat byl widoczny od razu.
    } else { // Wchodzimy do galezi, gdy semafor byl zajety przez inna instancje.
        printf("Semafor %d jest zajety. Oczekiwanie na zwolnienie...\n", idx + 1); // Informujemy o przejsciu do oczekiwania blokujacego.
        fflush(stdout); // Oprozniamy bufor wyjscia przed blokujacym semop.
        sem_wait_idx(idx); // Blokujemy sie do czasu, az druga instancja zwolni semafor.
        printf("Semafor %d zwolniony - uzyskano dostep.\n", idx + 1); // Informujemy, ze blokowanie zostalo zdjete.
        printf("%s\n", STUDENT_NAME); // Wypisujemy imie i nazwisko studenta zgodnie z trescia zadania.
        fflush(stdout); // Oprozniamy bufor wyjscia, aby dane byly natychmiast na ekranie.
        exit(EXIT_SUCCESS); // Konczymy program po odblokowaniu, jak wymaga zadanie.
    } // Konczymy galezie logiki zajmowania semafora.
} // Konczymy definicje funkcji acquire_sem.
int main(void) { // Definiujemy glowna funkcje programu sterujacego semaforami.
    semid = semget(SEM_KEY, 2, IPC_CREAT | IPC_EXCL | 0666); // Probujemy utworzyc nowy zestaw dwoch semaforow jako pierwsza instancja.
    if (semid == -1 && errno == EEXIST) { // Sprawdzamy, czy zestaw juz istnieje i trzeba sie dolaczyc.
        semid = semget(SEM_KEY, 2, 0666); // Otwieramy istniejacy zestaw semaforow utworzony przez pierwsza instancje.
        if (semid == -1) { // Sprawdzamy, czy dolaczenie do semaforow powiodlo sie.
            perror("semget"); // Wypisujemy szczegoly bledu dolaczenia do semaforow.
            return EXIT_FAILURE; // Konczymy program niepowodzeniem.
        } // Konczymy obsluge bledu semget przy dolaczaniu.
        printf("Dolaczono do istniejacych semaforow (PID: %d).\n", (int)getpid()); // Informujemy, ze ta instancja dolaczyla do wspolnych semaforow.
    } else if (semid == -1) { // Sprawdzamy, czy wystapil inny blad niz EEXIST podczas tworzenia semaforow.
        perror("semget"); // Wypisujemy szczegoly bledu tworzenia semaforow.
        return EXIT_FAILURE; // Konczymy program niepowodzeniem.
    } else { // Wchodzimy do galezi pierwszej instancji, ktora utworzyla semafory.
        union semun arg; // Tworzymy union potrzebny do ustawiania wartosci semaforow przez semctl.
        arg.val = 1; // Ustawiamy wartosc semafora 1 na 1, czyli zasob na start wolny.
        if (semctl(semid, SEM1_IDX, SETVAL, arg) == -1) { // Inicjalizujemy semafor 1 i sprawdzamy blad.
            perror("semctl SETVAL SEM1"); // Wypisujemy szczegoly bledu inicjalizacji semafora 1.
            semctl(semid, 0, IPC_RMID); // Usuwamy zestaw semaforow po nieudanej inicjalizacji.
            return EXIT_FAILURE; // Konczymy program niepowodzeniem.
        } // Konczymy obsluge bledu inicjalizacji semafora 1.
        if (semctl(semid, SEM2_IDX, SETVAL, arg) == -1) { // Inicjalizujemy semafor 2 i sprawdzamy blad.
            perror("semctl SETVAL SEM2"); // Wypisujemy szczegoly bledu inicjalizacji semafora 2.
            semctl(semid, 0, IPC_RMID); // Usuwamy zestaw semaforow po nieudanej inicjalizacji.
            return EXIT_FAILURE; // Konczymy program niepowodzeniem.
        } // Konczymy obsluge bledu inicjalizacji semafora 2.
        printf("Semafory zainicjalizowane (PID: %d).\n", (int)getpid()); // Informujemy, ze semafory zostaly utworzone i gotowe do uzycia.
    } // Konczymy galezie inicjalizacji/dolaczania do semaforow.
    printf("Sterowanie:\n"); // Wyswietlamy naglowek instrukcji klawiszy sterowania.
    printf("  1 + Enter - zajmij semafor 1\n"); // Wyswietlamy instrukcje zajecia pierwszego semafora.
    printf("  q + Enter - zwolnij semafor 1\n"); // Wyswietlamy instrukcje zwolnienia pierwszego semafora.
    printf("  2 + Enter - zajmij semafor 2\n"); // Wyswietlamy instrukcje zajecia drugiego semafora.
    printf("  w + Enter - zwolnij semafor 2\n"); // Wyswietlamy instrukcje zwolnienia drugiego semafora.
    printf("  x + Enter - wyjscie (usuwa zasoby IPC)\n"); // Wyswietlamy instrukcje wyjscia i sprzatania zasobow IPC.
    fflush(stdout); // Oprozniamy bufor wyjscia, aby instrukcje byly natychmiast widoczne.
    char line[16]; // Tworzymy maly bufor na pojedynczy rozkaz wpisywany z klawiatury.
    while (fgets(line, sizeof(line), stdin) != NULL) { // Odczytujemy kolejne komendy az do konca wejscia.
        switch (line[0]) { // Wybieramy akcje na podstawie pierwszego znaku wpisanej linii.
            case '1': // Obslugujemy polecenie zajecia semafora 1.
                acquire_sem(SEM1_IDX); // Wywolujemy logike zajecia semafora 1 z obsluga blokowania.
                break; // Konczymy obsluge tej komendy.
            case 'q': // Obslugujemy polecenie zwolnienia semafora 1.
                printf("Zwalnianie semafora 1.\n"); // Informujemy o zwalnianiu pierwszego semafora.
                fflush(stdout); // Oprozniamy bufor wyjscia, aby komunikat byl od razu widoczny.
                sem_post_idx(SEM1_IDX); // Zwolniamy semafor 1.
                break; // Konczymy obsluge tej komendy.
            case '2': // Obslugujemy polecenie zajecia semafora 2.
                acquire_sem(SEM2_IDX); // Wywolujemy logike zajecia semafora 2 z obsluga blokowania.
                break; // Konczymy obsluge tej komendy.
            case 'w': // Obslugujemy polecenie zwolnienia semafora 2.
                printf("Zwalnianie semafora 2.\n"); // Informujemy o zwalnianiu drugiego semafora.
                fflush(stdout); // Oprozniamy bufor wyjscia, aby komunikat byl od razu widoczny.
                sem_post_idx(SEM2_IDX); // Zwalniamy semafor 2.
                break; // Konczymy obsluge tej komendy.
            case 'x': // Obslugujemy polecenie wyjscia z programu.
                printf("Wyjscie. Usuwanie zasobow IPC.\n"); // Informujemy, ze czyscimy semafory z systemu.
                semctl(semid, 0, IPC_RMID); // Usuwamy caly zestaw semaforow z systemu IPC.
                return EXIT_SUCCESS; // Konczymy program powodzeniem.
            default: // Obslugujemy nieznane komendy.
                printf("Nieznana komenda: %c\n", line[0]); // Informujemy, ze podano nieobslugiwany klawisz.
                fflush(stdout); // Oprozniamy bufor wyjscia, aby komunikat byl natychmiast widoczny.
                break; // Konczymy obsluge nieznanej komendy.
        } // Konczymy instrukcje switch.
    } // Konczymy petle czytania komend.
    return EXIT_SUCCESS; // Zwracamy kod sukcesu po zakonczeniu pracy petli wejscia.
} // Konczymy funkcje main.
