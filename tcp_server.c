#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 2048
#define USERNAME "user"
#define PASSWORD "pass123"


void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int read_size;

    
    send(client_socket, "Username: ", strlen("Username: "), 0);
    read_size = recv(client_socket, buffer, BUFFER_SIZE, 0);
    buffer[read_size - 1] = '\0'; 

    if (strcmp(buffer, USERNAME) != 0) {
        send(client_socket, "Authentication failed.\n", strlen("Authentication failed.\n"), 0);
        close(client_socket);
        return;
    }

    send(client_socket, "Password: ", strlen("Password: "), 0);
    read_size = recv(client_socket, buffer, BUFFER_SIZE, 0);
    buffer[read_size - 1] = '\0'; 

    if (strcmp(buffer, PASSWORD) != 0) {
        send(client_socket, "Authentication failed.\n", strlen("Authentication failed.\n"), 0);
        close(client_socket);
        return;
    }
    
    
    send(client_socket, "Welcome to Remote UniShell!\n---END---\n$ ", strlen("Welcome to Remote UniShell!\n---END---\n$ "), 0);


    
    while ((read_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size - 1] = '\0'; 

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Client disconnected.\n");
            break;
        }

        printf("Executing command for client: '%s'\n", buffer);

        
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            continue;
        }

        if (pid == 0) {
            
            dup2(client_socket, STDOUT_FILENO);
            dup2(client_socket, STDERR_FILENO);

            
            execlp("/bin/sh", "sh", "-c", buffer, NULL);
            
            
            perror("execlp failed");
            exit(1);
        } else { 
            waitpid(pid, NULL, 0); 
            send(client_socket, "\n---END---\n$ ", strlen("\n---END---\n$ "), 0);
        }
    }

    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
            perror("accept");
            continue; 
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        
        if (fork() == 0) {
            close(server_fd); 
            handle_client(client_socket);
            exit(0);
        }
        close(client_socket);
    }

    return 0;
}