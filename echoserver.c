/*
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
                /* print bytes to stdin */
                printf("%s\n", buf);
                for (i = 0; i < strlen(buf); i++)
                    printf("%02x ", buf[i]);
                printf("\n");

                /* parse client input */
                if (!strcmp("VERSION\n", buf)) {
                    // write '1'
                    r = write(s0, "1\n", 2);
                }
                else if (!strcmp("DAYTIME\n", buf)) {
                    now = time(NULL);
                    tp = localtime(&now);
                    strcpy(buf, asctime(tp));

                    r = write(s0, buf, strlen(buf));
                }
                else if (!strcmp("RANDOM\n", buf)) {
                    srandom(time(NULL));
                    char rnum[3];
                    rnum[1] = '\n';
                    rnum[2] = '\0';
                    rnum[0] = '0' + ((unsigned int) random() % 10);
                    r = write(s0, rnum, strlen(rnum));
                }
                else {
                    assert(r != -1);
                    printf("Child %d: read %d bytes\n", pid, r);
                    if (r == 0)
                        break;
                    r = write(s0, buf, r);
                }
                assert(r != -1);
                printf("Child %d: wrote %d bytes\n", pid, r);
            }
            printf("Child %d: connection closed\n", pid);
            (void) close(s0);
            return 0;
        }
    }
    return 0;
}
