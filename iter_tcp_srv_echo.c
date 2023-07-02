#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>

#define MAX_MSG_LEN 120
#define MAX_MSG_BUF_LEN (MAX_MSG_LEN + 100)
#define DEBUG 0

char *ip_address, *port, *veri_code;

//install SIGINT
int sigint_flag = 0;
void sigint_handler(int sig)
{
	printf("[srv] SIGINT is coming!\n");
	sigint_flag = 1;
}

void server_process(int conn_fd)
{
	char recv_buf[MAX_MSG_BUF_LEN], send_buf[MAX_MSG_BUF_LEN];
	int recv_len, send_len;
	while(1)
	{
		memset(recv_buf, 0, sizeof(recv_buf));
		memset(send_buf, 0, sizeof(send_buf));

		// read data from client
		if(read(conn_fd, recv_buf, sizeof(recv_buf)) == 0)
		{
			break;
		} 

		// output to stdout
		printf("[ECH_RQT]%s", recv_buf);

		// send data to client with veri_code added in the beginning with the format : (veri_code)
		strcpy(send_buf, "(");
		strcat(send_buf, veri_code);
		strcat(send_buf, ")");
		strcat(send_buf, recv_buf);
		if(write(conn_fd, send_buf, strlen(send_buf)) < 0)
		{
			perror("send error");
		}
	}
}

int main(int argc, char *argv[])
{
    //arg1=ip_address arg2=port arg3=veri_code(int len=5)
	ip_address = argv[1];
	port = argv[2];
	veri_code = argv[3];

	#if DEBUG
	printf("ip_address=%s\n", ip_address);
	printf("port=%s\n", port);
	printf("veri_code=%s\n", veri_code);
	#endif

	//install SIGINT
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = sigint_handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);

	//Create listen socket
	int listen_fd = socket(PF_INET, SOCK_STREAM, 0); // PF_INET:IPv4, SOCK_STREAM:TCP
	struct sockaddr_in srv_address;
	memset(&srv_address, 0, sizeof(srv_address));
	srv_address.sin_family = AF_INET; // AF_INET:IPv4
	if(inet_pton(AF_INET, ip_address, &srv_address.sin_addr.s_addr) < 0) // ip_address
	{
		perror("inet_pton error");
	}
	srv_address.sin_port = htons(atoi(port)); // port
	if(bind(listen_fd, (struct sockaddr *)&srv_address, sizeof(srv_address)) < 0) // bind ip_address
	{
		perror("bind error");
	}

	// listen
	listen(listen_fd, 5); // 5:backlog
	printf("[srv] server[%s:%s][%s] is initializing!\n", ip_address, port, veri_code);

	// Create conn_fd
	int conn_fd;
	struct sockaddr_in cli_address;
	socklen_t cli_address_len = sizeof(cli_address);

	printf("[srv] Server has initialized!\n");

	// Major Cycle
	while(!sigint_flag)
	{
		// Minor Cycle
		if((conn_fd = accept(listen_fd, (struct sockaddr *)&cli_address, &cli_address_len)) < 0)
		{
			if(errno == EINTR)
				continue;
			else
			{
				perror("accept error");
				break;
			}
		}

		// connected
		printf("[srv] client[%s:%d] is accepted!\n", inet_ntoa(cli_address.sin_addr), ntohs(cli_address.sin_port));
		server_process(conn_fd);

		// close conn_fd
		if(close(conn_fd) < 0)
		{
			perror("close error");
		}
		printf("[srv] client[%s:%d] is closed!\n", inet_ntoa(cli_address.sin_addr), ntohs(cli_address.sin_port));
	}

	//close
	if(close(listen_fd) < 0)
	{
		perror("close error");
	}
	printf("[srv] listen_fd is closed!\n");
	printf("[srv] server is to return!");
    return 0;
}
