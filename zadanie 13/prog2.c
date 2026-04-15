/*
 * Zadanie 13 – prog2 (konsument)
 *
 * Odbiera tekst od prog1 przez pamiec wspoldzielona
 * (System V IPC) i wyswietla go w terminalu.
 * Pracuje w petli nieskonczonej.
 *
 * Uzycie: uruchom prog2 po uruchomieniu prog1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY  0x1301
#define SEM_KEY  0x1302
#define BUF_SIZE 256

#define SEM_EMPTY 0
#define SEM_FULL  1

static void sem_op(int semid, int semnum, int delta)
{
    struct sembuf op;
    op.sem_num = (unsigned short)semnum;
    op.sem_op  = (short)delta;
    op.sem_flg = 0;
    while (semop(semid, &op, 1) == -1) {
        if (errno == EINTR)
            continue;
        if (errno == EIDRM) {
            /* Producent usunal zasoby IPC – konczymy prace */
            printf("\nProducent zakonczyl prace. Konsument konczy.\n");
            exit(0);
        }
        perror("semop");
        exit(1);
    }
}

int main(void)
{
    int shmid = shmget(SHM_KEY, BUF_SIZE, 0666);
    if (shmid == -1) {
        perror("shmget (uruchom najpierw prog1)");
        return 1;
    }

    const char *shm = shmat(shmid, NULL, SHM_RDONLY);
    if (shm == (const char *)-1) {
        perror("shmat");
        return 1;
    }

    int semid = semget(SEM_KEY, 2, 0666);
    if (semid == -1) {
        perror("semget (uruchom najpierw prog1)");
        shmdt(shm);
        return 1;
    }

    printf("Konsument uruchomiony, oczekuje na dane...\n");
    fflush(stdout);

    while (1) {
        sem_op(semid, SEM_FULL, -1);    /* czekaj na dane */
        printf("Odebrano: %s\n", shm);
        fflush(stdout);
        sem_op(semid, SEM_EMPTY, +1);   /* sygnalizuj: bufor pusty */
    }

    shmdt(shm);
    return 0;
}
