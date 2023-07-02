#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_MSG_LEN 120
#define MAX_MSG_BUF_LEN (MAX_MSG_LEN + 100)
#define DEBUG 0

// client process
void client_process(int conn_fd)
{
	// client receive data from stdin and output to stdout, then send data to server which end with '\n\0'
	char recv_buf[MAX_MSG_BUF_LEN], send_buf[MAX_MSG_BUF_LEN];
	int recv_len, send_len;
	while (1)
	{
		memset(recv_buf, 0, sizeof(recv_buf));
		memset(send_buf, 0, sizeof(send_buf));

		// receive send_data from stdin using fgets
		fgets(send_buf, sizeof(send_buf), stdin);
		send_len = strlen(send_buf);

		// output to stdout
		printf("[ECH_RQT]%s", send_buf);

		// judge whether to exit
		if (strcmp(send_buf, "EXIT\n") == 0)
		{
			break;
		}
		
		// send data to server with '\n\0' added in the end
        // send_buf[send_len] = '\n';
        send_buf[++send_len] = '\0';
        // send_len++;
		if (write(conn_fd, send_buf, send_len) < 0)
		{
			perror("send error");
		}

		// receive data from server
		if (read(conn_fd, recv_buf, sizeof(recv_buf)) < 0)
		{
			perror("recv error");
		}
		else
		{
			printf("[ECH_REP]%s", recv_buf);
		}
	}
}

int main(int argc, char *argv[])
{
	// arg1=ip_address arg2=port
	char *ip_address = argv[1];
	char *port = argv[2];

#if DEBUG
	printf("ip_address=%s\n", ip_address);
	printf("port=%s\n", port);
#endif
	
	// Create socket
	struct sockaddr_in srv_address;
	int conn_fd = socket(PF_INET, SOCK_STREAM, 0); // PF_INET:IPv4, SOCK_STREAM:TCP
	memset(&srv_address, 0, sizeof(srv_address));
	srv_address.sin_family = AF_INET; // AF_INET:IPv4
	if (inet_pton(AF_INET, ip_address, &srv_address.sin_addr.s_addr) < 0) // ip_address
	{
		perror("inet_pton error");
	}
	srv_address.sin_port = htons(atoi(port)); // port

	// Connect
	if (connect(conn_fd, (struct sockaddr *)&srv_address, sizeof(srv_address)) < 0) // connect ip_address
	{
		perror("connect error");
	}
	printf("[cli] server[%s:%s] is connected!\n", ip_address, port);

	// send/recv
	client_process(conn_fd);

	// close
	if(close(conn_fd) < 0)
	{
		perror("close error");
	}
	printf("[cli] conn_fd is closed!\n");
	printf("[cli] client is to return!");
	return 0;
}
