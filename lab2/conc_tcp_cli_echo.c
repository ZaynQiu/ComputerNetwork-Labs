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
#define DEBUG 1

char *ip_address, *port, *client_id;

// client process
void client_process(int conn_fd)
{
	// client receive data from stdin and output to stdout, then send data to server which end with '\n\0'
	char recv_payload[MAX_MSG_BUF_LEN], recv_vcd[2];
	char send_buf[MAX_MSG_BUF_LEN], send_cid[2], send_payload[MAX_MSG_BUF_LEN];
	int recv_len, send_len;
	while (1)
	{
		memset(recv_payload, 0, sizeof(recv_payload));
		memset(recv_vcd, 0, sizeof(recv_vcd));
		memset(send_buf, 0, sizeof(send_buf));

		// receive send_data from stdin using fgets
		fgets(send_payload, sizeof(send_payload), stdin);

		// output to stdout
		printf("[cli](%d)[cid](%s)[ECH_RQT] %s", getpid(), client_id, send_payload);

		// judge whether to exit
		if (strcmp(send_payload, "EXIT\n") == 0)
		{
			break;
		}
		
		// add client_id to send_buf in the first 2 bytes
		short cid = htons(atoi(client_id));
		memcpy(send_buf, &cid, 2);
		#if DEBUG
		//output send_buf
		printf("DEBUG:cid=%d\n", atoi(client_id));
		printf("DEBUG: SEND_BUF:%sSEND_BUF_END\n", send_buf);
		#endif
		memcpy(send_buf+2, send_payload, strlen(send_payload));
		send_len = strlen(send_buf);
		// send data to server with '\n\0' in the end
        // send_buf[++send_len] = '\0';

		#if DEBUG
		//output send_buf
		printf("DEBUG: SEND_BUF:%sSEND_BUF_END\n", send_buf);
		#endif

		if (write(conn_fd, send_buf, send_len) < 0)
		{
			perror("send error");
		}

		// receive data from server
		if (read(conn_fd, recv_vcd, 2) < 0)
		{
			perror("recv recv_vcd");
		}
		else
		{
			read(conn_fd, recv_payload, sizeof(recv_payload));
			printf("[cli](%d)[vcd](%s)[ECHO_REP] %s", getpid(), recv_vcd, recv_payload);
		}
	}
}

int main(int argc, char *argv[])
{
	// arg1=ip_address arg2=port
	ip_address = argv[1];
	port = argv[2];
	client_id = argv[3];

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
	printf("[cli](%d)[srv_sa][%s:%s] Server is connected!\n", getpid(), ip_address, port);

	// send/recv
	client_process(conn_fd);

	// close
	if(close(conn_fd) < 0)
	{
		perror("close error");
	}
	printf("[cli](%d) conn_fd is closed!\n", getpid());
	printf("[cli](%d) Client is to return!", getpid());
	return 0;
}
