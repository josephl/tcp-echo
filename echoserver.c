/*
 * Copyright © 2012 Joseph Lee
 * This program is licensed under the MIT License.
 * CS 494
 * Assignment #2
 *
 * Copyright © 2012 Bart Massey
 * [This program is licensed under the "MIT License"]
 * Please see the file COPYING in the source
 * distribution of this software for license terms.
 */

/*
 * TCP Echo Service
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define L_PORT 11466

void sigchild(int sig) {
    pid_t pid = wait(0);
    assert(pid != -1);
    printf("Server reaped child %d\n", pid);
}

int main() {
    int r;
    struct sockaddr_in addr;
    int s;
    int i;
    time_t now;
    struct tm *tp;
    char buf[512];
    char *echo;

    void (*sr)(int) = signal(SIGCHLD, sigchild);
    assert(sr != SIG_ERR);
    s = socket(AF_INET, SOCK_STREAM, 0);
    assert(s != -1);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(L_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    r = bind(s, (struct sockaddr *)&addr, sizeof(addr));
    assert(r != -1);
    r = listen(s, 99);
    assert(r != -1);
    printf("Starting server...\n");
    while (1) {
        int pid;
        int s0 = accept(s, 0, 0);
        assert(s0 != -1);
        printf("Got connection...\n");
        pid = fork();
        assert(pid != -1);
        if (pid == 0) {
            pid = getpid();
            printf("Child %d\n", pid);
            while(1) {
                bzero((void *)buf, 512);
                r = read(s0, buf, 512);

                /* half-close read socket */
                assert(shutdown(s0, SHUT_RD) == 0);

                /* print bytes to stdin */
                printf("%s\n", buf);
                for (i = 0; i < strlen(buf); i++)
                    printf("%02x ", buf[i]);
                printf("\n");

                /* discard line ending from buffer */
                for (i = strlen(buf) - 1; i > 0; i--) {
                    if (buf[i] == '\n') {
                        buf[i] = '\0';
                        if (i > 0 && buf[i - 1] == '\r')
                            buf[i - 1] = '\0';
                        break;
                    }
                }

                /* parse client input */
                if (!strcmp("VERSION", buf)) {
                    // write '1'
                    r = write(s0, "1\r\n", 3);
                }
                else if (!strcmp("DAYTIME", buf)) {
                    now = time(NULL);
                    tp = localtime(&now);
                    strcpy(buf, asctime(tp));

                    r = write(s0, buf, strlen(buf));
                }
                else if (!strcmp("RANDOM", buf)) {
                    srandom(time(NULL));
                    char rnum[4];
                    rnum[1] = '\r';
                    rnum[2] = '\n';
                    rnum[3] = '\0';
                    rnum[0] = '0' + ((unsigned int) random() % 10);
                    r = write(s0, rnum, strlen(rnum));
                }
                else {      /* echo */
                    echo = strstr(buf, "ECHO ");
                    if (echo) {
                        echo += 5;
                        i = strlen(echo);
                        echo[i] = '\r';
                        echo[i + 1] = '\n';
                        echo[i + 2] = '\0';
                        r = write(s0, echo, strlen(echo));
                        assert(r != -1);
                    }
                    else {
                        r = write(s0, "Unknown command\n", 16);
                    }
                }
                assert(shutdown(s0, SHUT_WR) == 0);
            }
            printf("Child %d: connection closed\n", pid);
            (void) close(s0);
            return 0;
        }
    }
    return 0;
}
