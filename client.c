#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 34554
#define BUFFER_SIZE 1024

int main(int argc, char * argv[]) {
    if (argc < 2)
    {
        printf("Expected arguments: file path on client side," 
                "IP address of server," 
                "file path on server side.\n");
        exit(EXIT_FAILURE);
    }

    char * client_path = argv[1];
    char * ip_address = argv[2];
    char server_path[BUFFER_SIZE] = {0};
    strcpy(server_path, argv[3]);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        printf("Cannot open socket.\n");
        exit(EXIT_FAILURE);
    }
    
    // socket address configuration
    struct sockaddr_in socket_address = {0};
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, ip_address, &socket_address.sin_addr);

    if (-1 == connect(client_socket, 
                        (struct sockaddr *) &socket_address,        
                        sizeof(socket_address)))
    {
        printf("Cannot connect to the server.\n");  
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    // First message is the path for saving file on server side.
    // All messages including the file name are the same size equal to the BUFFER_SIZE.
    if (send(client_socket, server_path, BUFFER_SIZE, 0) == -1)
    {
        printf("Cannot send file path.\n");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    FILE * f = fopen(client_path, "r");
    if (f == NULL)
    {
        printf("Cannot open file.\n");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Read bytes (size equal BUFFER_SIZE) from file to out_buffer,
    // and then send them to the server.
    char out_buffer[BUFFER_SIZE] = {0};
    while (!feof(f))
    {
        fread(out_buffer, 1, BUFFER_SIZE, f);
        if (send(client_socket, out_buffer, BUFFER_SIZE, 0) == -1)
        {
            printf("Cannot send data.\n");
            fclose(f);
            close(client_socket);
            exit(EXIT_FAILURE);
        }
        memset(out_buffer, 0, BUFFER_SIZE);
    }
    sleep(5);
    fclose(f);
    close(client_socket);
    return 0;
}
