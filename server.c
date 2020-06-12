/*
Необходимо разработать клиент-серверное приложение для передачи файлов 
через TCP сокеты. Клиент принимает следующие аргументы: путь к файлу на стороне клиента, 
IP сервера и путь на стороне сервера куда должен быть сохранен файл. 
Сервер работает без аргументов и просто сохраняет все полученные файлы в 
указанные клиентами директории. Обязательно использовать язык C или C++ для серверной части. 
Клиент желательно (но необязательно) написать на любом другом языке. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define SERVER_PORT 34554
#define BUFFER_SIZE 1024
#define BACKLOG_LENGTH 5

int save_to_file(const char * file_name, char * datagram, size_t length)
{
    if (file_name == NULL || datagram == NULL || length == 0)
        return -1;

    FILE * f = fopen(file_name, "a");
    if (f == NULL)
        return -1;
    
    if (fwrite(datagram, 1, length, f) == 0)
    {
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

int main(void) {    
    // main socket of the server
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1 )
    {
        printf("Cannot open socket.\n");
        exit(EXIT_FAILURE);
    }
    
    // server address configuration
    struct sockaddr_in socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr  = inet_addr("127.0.0.1");
    socket_address.sin_port = htons(SERVER_PORT);

    if (-1 == bind(server_socket, 
        (struct sockaddr *) &socket_address, 
        sizeof(socket_address)))
    {
        printf("Cannot bind address for socket.\n");
        close(server_socket);
        exit(EXIT_FAILURE);
    }


    if (listen(server_socket, BACKLOG_LENGTH) == -1)
    {
        printf("Cannot start listening.\n");
        close(server_socket);
        exit(EXIT_FAILURE); 
    }

    int filename_recv = 0;
    char filename[BUFFER_SIZE] = {0};

    while (1)
    {
        socklen_t address_len = sizeof(socket_address);
        int socket_descriptor = accept(server_socket, 
                                        (struct sockaddr *) &socket_address, 
                                        &(address_len));

        if (socket_descriptor == -1)
        {
            printf("Cannot accept a connection.\n");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        char in_buffer[BUFFER_SIZE] = {0};
        int recv_status = 0;

        while ((recv_status = recv(socket_descriptor, in_buffer, BUFFER_SIZE, 0)) > 0)
        {
            if (filename_recv == 0)
            {
                strcpy(filename, in_buffer);
                filename_recv = 1;
            }
            else if (save_to_file(filename, in_buffer, BUFFER_SIZE) == -1)
            {
                printf("Cannot save to file.\n");
                close(server_socket);
                close(socket_descriptor);
                exit(EXIT_FAILURE);
            }
        }

        if (recv_status == -1)
        {
                printf("Cannot read a datagram.\n");
                close(server_socket);
                close(socket_descriptor);
                exit(EXIT_FAILURE);
        } 
        else 
        {
            sleep(1);
            close(socket_descriptor); 
            filename_recv = 0;
        }
    }
    return 0;
}
