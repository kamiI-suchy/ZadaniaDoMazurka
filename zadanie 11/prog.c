#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void alarm_handler(int sig)
{
    (void)sig;
    printf("Alarm!\n");
    fflush(stdout);
    alarm(3);
}

int main(void)
{
    signal(SIGALRM, alarm_handler);
    alarm(3);

    while (1) {
        pause();
    }

    return 0;
}
