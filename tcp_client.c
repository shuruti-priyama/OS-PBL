#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 2048
#define DELIMITER "---END---"

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Server IP>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to server.\n");

    
    int read_size = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[read_size] = '\0';
    printf("%s", buffer);
    fgets(buffer, BUFFER_SIZE, stdin);
    send(sock, buffer, strlen(buffer), 0);

    read_size = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[read_size] = '\0';
    printf("%s", buffer);
    fgets(buffer, BUFFER_SIZE, stdin);
    send(sock, buffer, strlen(buffer), 0);

    
    while (1) {
        char full_response[BUFFER_SIZE * 5] = {0};
        
        
        while (strstr(full_response, DELIMITER) == NULL) {
            read_size = recv(sock, buffer, BUFFER_SIZE - 1, 0);
            if (read_size <= 0) {
                printf("Server disconnected.\n");
                close(sock);
                return -1;
            }
            buffer[read_size] = '\0';
            strcat(full_response, buffer);
        }

        
        char *delimiter_pos = strstr(full_response, DELIMITER);
        if (delimiter_pos != NULL) {
            
            *delimiter_pos = '\0';
            
            
            printf("%s", full_response);

            
            char *prompt_start = delimiter_pos + strlen(DELIMITER);
            
        
            printf("%s", prompt_start);
        } else {
            
            printf("%s", full_response);
        }
        
        fflush(stdout);

        
        char user_input[BUFFER_SIZE];
        if (fgets(user_input, sizeof(user_input), stdin) == NULL) {
            break; 
        }

        send(sock, user_input, strlen(user_input), 0);

        if (strncmp(user_input, "exit", 4) == 0) {
            break;
        }
    }

    printf("Exiting client.\n");
    close(sock);
    return 0;
}