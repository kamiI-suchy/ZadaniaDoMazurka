#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <setjmp.h>

static sigjmp_buf jump_buffer;

void alarm_handler(int sig)
{
    (void)sig;
    siglongjmp(jump_buffer, 1);
}

int main(void)
{
    char buf[256];

    signal(SIGALRM, alarm_handler);

    while (1) {
        printf("Wprowadź tekst: ");
        fflush(stdout);

        if (sigsetjmp(jump_buffer, 1) == 0) {
            alarm(5);
            if (fgets(buf, sizeof(buf), stdin) != NULL) {
                alarm(0);
                buf[strcspn(buf, "\n")] = '\0';
                printf("Wprowadzono: %s\nzaakceptowano\n", buf);
                fflush(stdout);
            }
        } else {
            printf("\ntimeout\n");
            fflush(stdout);
            clearerr(stdin);
        }
    }

    return 0;
}
