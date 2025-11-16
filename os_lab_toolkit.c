#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_OUTPUT 4096

// Escape JSON characters
void escape_json(const char *src, char *dest) {
    while (*src) {
        if (*src == '\"') {
            *dest++ = '\\';
            *dest++ = '\"';
        }
        else if (*src == '\\') {
            *dest++ = '\\';
            *dest++ = '\\';
        }
        else if (*src == '\n') {
            *dest++ = '\\';
            *dest++ = 'n';
        }
        else if (*src == '\t') {
            *dest++ = '\\';
            *dest++ = 't';
        }
        else {
            *dest++ = *src;   // ✅ FIXED LINE
        }
        src++;
    }
    *dest = '\0';
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("{\"status\":\"error\",\"output\":\"No command provided\"}");
        return 0;
    }

    char cmd[256] = {0};
    for (int i = 1; i < argc; i++) {
        strcat(cmd, argv[i]);
        if (i < argc - 1) strcat(cmd, " ");
    }

    int pipefd[2];
    pipe(pipefd);

    pid_t pid = fork();

    if (pid == 0) {  
        // Child
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        execl("/bin/sh", "sh", "-c", cmd, NULL);
        perror("exec failed");
        exit(1);
    }

    close(pipefd[1]);

    char buffer[MAX_OUTPUT] = {0};
    read(pipefd[0], buffer, MAX_OUTPUT - 1);
    close(pipefd[0]);

    wait(NULL);

    char escaped[MAX_OUTPUT * 2] = {0};
    escape_json(buffer, escaped);

    printf("{\"status\":\"ok\",\"output\":\"%s\"}", escaped);

    return 0;
}
