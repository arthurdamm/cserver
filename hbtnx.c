#include "hbtnx.h"

int main(int ac, char** av)
{
	unsigned short port;
	int sockfd;
	struct sockaddr_in server;

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	if (ac < 2)
		return (printf("USAGE: %s <port>\n", av[0]), EXIT_FAILURE);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		return (perror("socket failed"), EXIT_FAILURE);
	port = atoi(av[1]);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
		return (perror("bind failed"), EXIT_FAILURE);
	if (listen(sockfd, BACKLOG) == -1)
		return (perror("listen failed"), EXIT_FAILURE);
	printf("Server listening on port [%d]\n", ntohs(server.sin_port));

	while (89)
		accept_connections(sockfd);

	close(sockfd);
	return (EXIT_SUCCESS);
}

int accept_connections(int server_sd)
{
	struct sockaddr_in client;
	socklen_t client_sz = sizeof(client);
	int client_sd;
	char ip_buf[INET_ADDRSTRLEN + 1] = {0}, request_buf[REQUEST_BUF + 1];
	ssize_t bytes;

	client_sd = accept(server_sd, (struct sockaddr *)&client, &client_sz);
	if (client_sd == -1)
		return (perror("accept failed"), EXIT_FAILURE);
	if (fork())
	{
		close(client_sd);
		return (EXIT_SUCCESS);
	}
	if (!inet_ntop(AF_INET, &client.sin_addr.s_addr, ip_buf, INET_ADDRSTRLEN))
		return (perror("ip name failed"), EXIT_FAILURE);
	printf("Client connected from: [%s]\n", ip_buf);

	bytes = read(client_sd, request_buf, REQUEST_BUF);
	if (bytes > 0)
	{
		request_buf[bytes] = 0;
		parse_request(client_sd, request_buf);
	}

	close(client_sd);
	close(server_sd);
	exit(EXIT_SUCCESS);
}

int parse_request(int client_sd, char *request)
{
	char method[SMALL_BUF + 1] = {0}, path[SMALL_BUF + 1] = {0},
		version[SMALL_BUF + 1] = {0}, file_buf[READ_BUF + 1],
		*_path = path;
	int fd;
	ssize_t bytes;

	printf("RAW REQUEST:[%s]\n", request);
	if (sscanf(request, "%s %s %s\r\n", method, path, version) != 3)
		return (send_response(client_sd, RESPONSE_400, NULL), EXIT_FAILURE);
	printf("METHOD:[%s]\nPATH:[%s]\nVERSION[%s]\n", method, path, version);

	while (*_path == '/')
		_path++;
	if (strstr(_path, "..") || strstr(_path, "~"))
		return (send_response(client_sd, RESPONSE_400, NULL), EXIT_FAILURE);
	if (!*_path)
		_path = "index.html";
	printf("OPENING PATH:[%s]\n", _path);
	fd = open(_path, O_RDONLY);
	if (fd == -1)
		return (send_response(client_sd, RESPONSE_404, NULL), EXIT_FAILURE);

	bytes = read(fd, file_buf, READ_BUF);
	if (bytes > 0)
	{
		file_buf[bytes] = 0;
		return (send_response(client_sd, RESPONSE_200, file_buf), EXIT_SUCCESS);
	}
	return (send_response(client_sd, RESPONSE_500, NULL), EXIT_FAILURE);
}

int send_response(int client_sd, char *response, char *body)
{
	char *data = NULL;
	size_t len;

	body = body ? body : "";
	data = malloc(strlen(response) + strlen(body) + 1);
	if (!data)
	{
		data = strdup(RESPONSE_500);
		len = strlen(data);
	}
	else
		len = sprintf(data, "%s%s", response, body);
	write(client_sd, data, len);
	free(data);
	return (EXIT_SUCCESS);
}

