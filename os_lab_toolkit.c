#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>

#define MAX_LINE 1024
#define MAX_ARGS 100
#define BUFFER_SIZE 4096

/* ======================== SIGNAL HANDLER ======================== */

void sigint_handler(int sig) {
    printf("\n(Ctrl+C) Type 'exit' to quit.\n$ ");
    fflush(stdout);
}

/* ======================== PARSING ======================== */

int parse_command(char *line, char **args, int *background) {
    *background = 0;

    line[strcspn(line, "\n")] = 0;

    if (strlen(line) == 0) return 0;

    if (line[strlen(line) - 1] == '&') {
        *background = 1;
        line[strlen(line) - 1] = '\0';
    }

    int i = 0;
    char *token = strtok(line, " \t");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;
    return i;
}

/* ======================== REDIRECTION ======================== */

void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            args[i] = NULL;
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        else if (strcmp(args[i], "<") == 0) {
            args[i] = NULL;
            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
    }
}

/* ======================== LOCAL EXECUTION ======================== */

void execute_local(char **args, int background) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return; }

    if (pid == 0) {
        handle_redirection(args);
        if (execvp(args[0], args) < 0) {
            perror("execvp");
            exit(1);
        }
    }
    else {
        if (!background) waitpid(pid, NULL, 0);
        else printf("[Background PID: %d]\n", pid);
    }
}

/* ======================== PIPES ======================== */

void execute_piped(char *line) {
    char *cmd1 = strtok(line, "|");
    char *cmd2 = strtok(NULL, "|");

    if (!cmd2) {
        char *args[MAX_ARGS];
        int bg;
        parse_command(cmd1, args, &bg);
        execute_local(args, bg);
        return;
    }

    int fd[2];
    pipe(fd);

    pid_t p1 = fork();
    if (p1 == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        char *args1[MAX_ARGS];
        int bg;
        parse_command(cmd1, args1, &bg);
        handle_redirection(args1);
        execvp(args1[0], args1);
        perror("execvp");
        exit(1);
    }

    pid_t p2 = fork();
    if (p2 == 0) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        char *args2[MAX_ARGS];
        int bg;
        parse_command(cmd2, args2, &bg);
        handle_redirection(args2);
        execvp(args2[0], args2);
        perror("execvp");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
}

/* ======================== REMOTE MODE ======================== */

int connect_server(const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return -1; }

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    printf("✔ Connected to remote: %s:%d\n", ip, port);
    return sock;
}

void remote_exec(int sock, char *cmd) {
    send(sock, cmd, strlen(cmd), 0);

    char buffer[BUFFER_SIZE];
    ssize_t n;

    while ((n = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);

        if (strstr(buffer, "--END-OF-OUTPUT--"))
            break;
    }
}

/* ======================== MAIN SHELL ======================== */

int main() {
    signal(SIGINT, sigint_handler);

    int remote = -1;
    char line[MAX_LINE];
    char *args[MAX_ARGS];
    int background;

    printf("\n========== UniShell Hybrid Shell ==========\n");
    printf("Commands:\n");
    printf("  connect <IP> <PORT>  → remote mode\n");
    printf("  disconnect           → local mode\n");
    printf("  exit                 → quit\n\n");

    while (1) {
        printf(remote == -1 ? "Local$ " : "Remote$ ");
        fflush(stdout);

        if (!fgets(line, MAX_LINE, stdin)) break;

        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;

        if (!strcmp(line, "exit")) {
            if (remote != -1) close(remote);
            break;
        }

        if (!strncmp(line, "connect", 7)) {
            char ip[64];
            int port;
            if (sscanf(line, "connect %s %d", ip, &port) == 2) {
                if (remote != -1) {
                    printf("Already connected.\n");
                } else {
                    remote = connect_server(ip, port);
                }
            } else {
                printf("Usage: connect <IP> <PORT>\n");
            }
            continue;
        }

        if (!strcmp(line, "disconnect")) {
            if (remote != -1) {
                close(remote);
                remote = -1;
                printf("✔ Back to local mode.\n");
            } else printf("Not connected.\n");
            continue;
        }

        if (remote != -1) {
            remote_exec(remote, line);
            continue;
        }

        if (strchr(line, '|'))
            execute_piped(line);
        else {
            parse_command(line, args, &background);
            if (args[0]) execute_local(args, background);
        }
    }

    return 0;
}
