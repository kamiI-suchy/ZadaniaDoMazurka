/*
 * Zadanie 14 – sterowanie dwoma semaforami z klawiatury
 *
 * Dwie instancje tego samego programu wspoldziela dwa
 * semafory jednoelementowe (System V IPC).
 *
 * Klawiatura (zatwierdzac Enterem):
 *   1 – zajmij semafor 1
 *   q – zwolnij semafor 1
 *   2 – zajmij semafor 2
 *   w – zwolnij semafor 2
 *   x – wyjscie (usuwa zasoby IPC)
 *
 * Demonstracja blokowania:
 *   1. Instancja A wpisuje '1' – zajmuje semafor 1.
 *   2. Instancja B wpisuje '1' – zostaje zablokowana.
 *   3. Instancja A wpisuje 'q' – zwalnia semafor 1.
 *   4. Instancja B odblokowuje sie, wyswietla imie i nazwisko,
 *      a nastepnie konczy prace.
 *
 * Demonstracja zakleszczenia:
 *   1. Instancja A wpisuje '1' – zajmuje semafor 1.
 *   2. Instancja B wpisuje '2' – zajmuje semafor 2.
 *   3. Instancja A wpisuje '2' – blokuje sie (semafor 2 zajety przez B).
 *   4. Instancja B wpisuje '1' – blokuje sie (semafor 1 zajety przez A).
 *   5. Zakleszczenie – obie instancje czekaja w nieskonczonosc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define SEM_KEY      0x1401
#define SEM1_IDX     0
#define SEM2_IDX     1

/* Zmien na swoje imie i nazwisko */
#define STUDENT_NAME "Jan Kowalski"

union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};

static int semid = -1;

/* Blokujace zajecie semafora – zwraca po uzyskaniu dostepu */
static void sem_wait_idx(int idx)
{
    struct sembuf op;
    op.sem_num = (unsigned short)idx;
    op.sem_op  = -1;
    op.sem_flg = 0;
    while (semop(semid, &op, 1) == -1) {
        if (errno == EINTR)
            continue;
        perror("semop");
        exit(1);
    }
}

/* Nieblokujace zajecie semafora – zwraca 0 gdy udane, -1 gdy zajety */
static int sem_trywait_idx(int idx)
{
    struct sembuf op;
    op.sem_num = (unsigned short)idx;
    op.sem_op  = -1;
    op.sem_flg = IPC_NOWAIT;
    if (semop(semid, &op, 1) == 0)
        return 0;
    if (errno == EAGAIN)
        return -1;
    perror("semop");
    exit(1);
}

/* Zwolnienie semafora */
static void sem_post_idx(int idx)
{
    struct sembuf op;
    op.sem_num = (unsigned short)idx;
    op.sem_op  = +1;
    op.sem_flg = 0;
    if (semop(semid, &op, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

/*
 * Probuje zajac semafor o podanym indeksie.
 * Jesli semafor jest wolny – zajmuje go natychmiast.
 * Jesli jest zajety – blokuje sie; po odblokowaniu wyswietla
 * imie i nazwisko studenta, a nastepnie konczy program.
 */
static void acquire_sem(int idx)
{
    if (sem_trywait_idx(idx) == 0) {
        printf("Semafor %d zajety przez ten proces.\n", idx + 1);
        fflush(stdout);
    } else {
        printf("Semafor %d jest zajety. Oczekiwanie na zwolnienie...\n", idx + 1);
        fflush(stdout);
        sem_wait_idx(idx);
        printf("Semafor %d zwolniony – uzyskano dostep.\n", idx + 1);
        printf("%s\n", STUDENT_NAME);
        fflush(stdout);
        exit(0);
    }
}

int main(void)
{
    /* Pierwsza instancja tworzy i inicjalizuje semafory;
       druga dolacza do istniejacych. */
    semid = semget(SEM_KEY, 2, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1 && errno == EEXIST) {
        semid = semget(SEM_KEY, 2, 0666);
        if (semid == -1) {
            perror("semget");
            return 1;
        }
        printf("Dolaczono do istniejacych semaforow (PID: %d).\n",
               (int)getpid());
    } else if (semid == -1) {
        perror("semget");
        return 1;
    } else {
        union semun arg;
        arg.val = 1;
        semctl(semid, SEM1_IDX, SETVAL, arg);
        semctl(semid, SEM2_IDX, SETVAL, arg);
        printf("Semafory zainicjalizowane (PID: %d).\n", (int)getpid());
    }

    printf("Sterowanie:\n");
    printf("  1 + Enter – zajmij semafor 1\n");
    printf("  q + Enter – zwolnij semafor 1\n");
    printf("  2 + Enter – zajmij semafor 2\n");
    printf("  w + Enter – zwolnij semafor 2\n");
    printf("  x + Enter – wyjscie (usuwa zasoby IPC)\n");
    fflush(stdout);

    char line[16];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        switch (line[0]) {
        case '1':
            acquire_sem(SEM1_IDX);
            break;
        case 'q':
            printf("Zwalnianie semafora 1.\n");
            fflush(stdout);
            sem_post_idx(SEM1_IDX);
            break;
        case '2':
            acquire_sem(SEM2_IDX);
            break;
        case 'w':
            printf("Zwalnianie semafora 2.\n");
            fflush(stdout);
            sem_post_idx(SEM2_IDX);
            break;
        case 'x':
            printf("Wyjscie. Usuwanie zasobow IPC.\n");
            semctl(semid, 0, IPC_RMID);
            return 0;
        default:
            break;
        }
    }

    return 0;
}
