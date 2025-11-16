// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <string.h>
// #include <fcntl.h>
// #include <sys/wait.h>
// #include <signal.h>
// #include <arpa/inet.h>

// #define MAX_LINE 1024
// #define MAX_ARGS 100
// #define BUFFER_SIZE 4096


// void sigint_handler(int sig) {
//     printf("\n(Ctrl+C) Type 'exit' to quit shell.\n$ ");
//     fflush(stdout);
// }

// // Command Parsing
// int parse_command(char *line, char **args, int *background) {
//     *background = 0;
//     line[strcspn(line, "\n")] = 0; 

//     if (strlen(line) == 0) return 0;

    
//     if (line[strlen(line) - 1] == '&') {
//         *background = 1;
//         line[strlen(line) - 1] = '\0';
//     }

//     int i = 0;
//     char *token = strtok(line, " \t");
//     while (token != NULL && i < MAX_ARGS - 1) {
//         args[i++] = token;
//         token = strtok(NULL, " \t");
//     }
//     args[i] = NULL;
//     return i;
// }

// // Handle I/O Redirection 
// void handle_redirection(char **args) {
//     for (int i = 0; args[i] != NULL; i++) {
//         if (strcmp(args[i], ">") == 0) {
//             args[i] = NULL;
//             int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
//             if (fd < 0) { perror("open"); exit(1); }
//             dup2(fd, STDOUT_FILENO);
//             close(fd);
//         } else if (strcmp(args[i], "<") == 0) {
//             args[i] = NULL;
//             int fd = open(args[i + 1], O_RDONLY);
//             if (fd < 0) { perror("open"); exit(1); }
//             dup2(fd, STDIN_FILENO);
//             close(fd);
//         }
//     }
// }

// // Execute Single Command (Local) 
// void execute_command(char **args, int background) {
//     pid_t pid = fork();
//     if (pid < 0) { perror("fork"); return; }
//     else if (pid == 0) {
//         handle_redirection(args);
//         if (execvp(args[0], args) < 0) {
//             perror("execvp failed");
//             exit(1);
//         }
//     } else {
//         if (!background) waitpid(pid, NULL, 0);
//         else printf("[Background PID: %d]\n", pid);
//     }
// }

// // Execute Piped Commands (Local)
// void execute_piped(char *line) {
//     char *cmd1 = strtok(line, "|");
//     char *cmd2 = strtok(NULL, "|");

//     if (!cmd2) { // no pipe
//         char *args[MAX_ARGS];
//         int bg;
//         parse_command(cmd1, args, &bg);
//         execute_command(args, bg);
//         return;
//     }

//     int fd[2];
//     if (pipe(fd) == -1) { perror("pipe"); return; }

//     pid_t pid1 = fork();
//     if (pid1 == 0) {
//         close(fd[0]);
//         dup2(fd[1], STDOUT_FILENO);
//         close(fd[1]);
//         char *args1[MAX_ARGS];
//         int bg; parse_command(cmd1, args1, &bg);
//         handle_redirection(args1);
//         if (execvp(args1[0], args1) < 0) { perror("execvp"); exit(1); }
//     }

//     pid_t pid2 = fork();
//     if (pid2 == 0) {
//         close(fd[1]);
//         dup2(fd[0], STDIN_FILENO);
//         close(fd[0]);
//         char *args2[MAX_ARGS];
//         int bg; parse_command(cmd2, args2, &bg);
//         handle_redirection(args2);
//         if (execvp(args2[0], args2) < 0) { perror("execvp"); exit(1); }
//     }

//     close(fd[0]); close(fd[1]);
//     waitpid(pid1, NULL, 0);
//     waitpid(pid2, NULL, 0);
// }

// // Remote Mode Functions 
// int connect_to_server(const char *ip, int port) {
//     int sock = socket(AF_INET, SOCK_STREAM, 0);
//     if (sock < 0) { perror("socket"); return -1; }

//     struct sockaddr_in serv_addr;
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(port);
//     if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
//         perror("inet_pton");
//         close(sock);
//         return -1;
//     }

//     if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
//         perror("connect");
//         close(sock);
//         return -1;
//     }

//     printf("Connected to UniShell Server (%s:%d)\n", ip, port);
//     return sock;
// }

// void remote_execute(int sock, char *cmd) {
//     if (send(sock, cmd, strlen(cmd), 0) < 0) {
//         perror("send");
//         return;
//     }

//     char buffer[BUFFER_SIZE];
//     ssize_t n;
//     while ((n = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
//         buffer[n] = '\0';
//         printf("%s", buffer);
//         if (strstr(buffer, "--END-OF-OUTPUT--")) break;
//     }
// }


// int main() {
//     char line[MAX_LINE];
//     char *args[MAX_ARGS];
//     int background;
//     int remote_sock = -1; 

//     signal(SIGINT, sigint_handler);

//     printf(" Welcome to UniShell Hybrid Mode!\n");
//     printf("Commands:\n");
//     printf("   connect <IP> <PORT>   → Connect to remote server\n");
//     printf("   disconnect            → Return to local mode\n");
//     printf("   exit / quit           → Exit UniShell\n\n");

//     while (1) {
//         if (remote_sock == -1)
//             printf("Local$ ");
//         else
//             printf("Remote$ ");
//         fflush(stdout);

//         if (!fgets(line, sizeof(line), stdin)) break;
//         line[strcspn(line, "\n")] = 0; 
//         if (strlen(line) == 0) continue;

//         if (strncmp(line, "exit", 4) == 0 || strncmp(line, "quit", 4) == 0) {
//             if (remote_sock != -1) close(remote_sock);
//             break;
//         }

//         // Connect
//         if (strncmp(line, "connect", 7) == 0) {
//             char ip[64];
//             int port;
//             if (sscanf(line, "connect %63s %d", ip, &port) == 2) {
//                 if (remote_sock != -1) {
//                     printf("Already connected. Disconnect first.\n");
//                     continue;
//                 }
//                 remote_sock = connect_to_server(ip, port);
//                 if (remote_sock < 0) {
//                     printf(" Connection failed.\n");
//                     remote_sock = -1;
//                 }
//             } else {
//                 printf("Usage: connect <IP> <PORT>\n");
//             }
//             continue;
//         }

//         //  Disconnect
//         if (strcmp(line, "disconnect") == 0) {
//             if (remote_sock != -1) {
//                 close(remote_sock);
//                 remote_sock = -1;
//                 printf(" Disconnected from server. Back to local mode.\n");
//             } else {
//                 printf("Not connected to any server.\n");
//             }
//             continue;
//         }

//         //  Remote Mode 
//         if (remote_sock != -1) {
//             remote_execute(remote_sock, line);
//             continue;
//         }

//         //  Local Mode 
//         if (strchr(line, '|')) execute_piped(line);
//         else {
//             parse_command(line, args, &background);
//             if (args[0] != NULL) execute_command(args, background);
//         }
//     }

//     return 0;
// }




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
