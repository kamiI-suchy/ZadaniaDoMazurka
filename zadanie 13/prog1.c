/*
 * Zadanie 13 – prog1 (producent)
 *
 * Odczytuje tekst z klawiatury i przesyla go do prog2
 * za pomoca pamieci wspoldzielonej (System V IPC).
 * Synchronizacja przesylania realizowana jest przez
 * dwa semafory:
 *   SEM_EMPTY (indeks 0) – bufor jest pusty (poczatkowo 1)
 *   SEM_FULL  (indeks 1) – bufor zawiera dane (poczatkowo 0)
 *
 * Uzycie: uruchom prog1 przed prog2.
 *         Wpisz tekst i zatwierdz Enterem.
 *         Wpisz "quit" aby zakonczyc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY  0x1301
#define SEM_KEY  0x1302
#define BUF_SIZE 256

#define SEM_EMPTY 0
#define SEM_FULL  1

union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};

static void sem_op(int semid, int semnum, int delta)
{
    struct sembuf op;
    op.sem_num = (unsigned short)semnum;
    op.sem_op  = (short)delta;
    op.sem_flg = 0;
    while (semop(semid, &op, 1) == -1) {
        if (errno == EINTR)
            continue;
        perror("semop");
        exit(1);
    }
}

int main(void)
{
    int shmid = shmget(SHM_KEY, BUF_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    char *shm = shmat(shmid, NULL, 0);
    if (shm == (char *)-1) {
        perror("shmat");
        return 1;
    }

    int semid = semget(SEM_KEY, 2, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        shmdt(shm);
        return 1;
    }

    /* Inicjalizacja semaforow */
    union semun arg;
    arg.val = 1;
    semctl(semid, SEM_EMPTY, SETVAL, arg);
    arg.val = 0;
    semctl(semid, SEM_FULL, SETVAL, arg);

    char buf[BUF_SIZE];
    printf("Producent uruchomiony. Wpisz tekst (lub 'quit' aby zakonczyc):\n");

    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            break;
        buf[strcspn(buf, "\n")] = '\0';
        if (strcmp(buf, "quit") == 0)
            break;

        sem_op(semid, SEM_EMPTY, -1);   /* czekaj na pusty bufor */
        strncpy(shm, buf, BUF_SIZE - 1);
        shm[BUF_SIZE - 1] = '\0';
        sem_op(semid, SEM_FULL, +1);    /* sygnalizuj: dane gotowe */

        printf("Wyslano: %s\n", buf);
    }

    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    printf("Producent zakonczony.\n");
    return 0;
}
