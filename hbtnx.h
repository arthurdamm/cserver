#ifndef HBTNX_H
#define HBTNX_H

#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

#define BACKLOG 64
#define REQUEST_BUF 1024 * 10
#define READ_BUF 1024 * 10
#define SMALL_BUF 256

#define RESPONSE_200 "HTTP/1.1 200 OK\r\n\r\n"
#define RESPONSE_400 "HTTP/1.1 400 Bad Request\r\n\r\n"
#define RESPONSE_404 "HTTP/1.1 404 Not Found\r\n\r\n"
#define RESPONSE_500 "HTTP/1.1 500 Internal Server Error\r\n\r\n"


int accept_connections(int server_sd);
int parse_request(int client_sd, char *request);
int send_response(int client_sd, char *response, char *body);

#endif