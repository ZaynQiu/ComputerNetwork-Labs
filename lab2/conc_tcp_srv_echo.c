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
#include <sys/wait.h>

#define MAX_MSG_LEN 120
#define MAX_MSG_BUF_LEN (MAX_MSG_LEN + 100)
#define BACKLOG 5
#define DEBUG 1

char *ip_address, *port, *veri_code;

//install SIGINT
int sigint_flag = 0;
void sigint_handler(int sig)
{
	printf("[srv] SIGINT is coming!\n");
	sigint_flag = 1;
}

//install SIGCHLD
void sigchld_handler(int sig)
{
	int status;
	pid_t pid;
	while((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		printf("[srv](%d)[chd](%d) Child has terminated!\n", getpid(), pid);
	}
}

//install SIGPIPE
void sigpipe_handler(int sig)
{
	printf("[srv](%d) SIGPIPE is coming!\n", getpid());
}


void server_process(int conn_fd)
{
	char recv_payload[MAX_MSG_BUF_LEN], recv_cid[2];
	char send_buf[MAX_MSG_BUF_LEN], send_vcd[2], send_payload[MAX_MSG_BUF_LEN];
	int recv_len, send_len;
	while(1)
	{
		memset(recv_payload, 0, sizeof(recv_payload));
		memset(recv_cid, 0, sizeof(recv_cid));
		memset(send_buf, 0, sizeof(send_buf));
		memset(recv_cid, 0, sizeof(recv_cid));

		#if DEBUG
		printf("Server before read data from client.\n");
		//print the client info
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		getpeername(conn_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		printf("[srv_pid](%d)[client_pip](%s)[cpn](%d)\n", getpid(), inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		#endif

		// read data from client
		int read_status = read(conn_fd, recv_cid, sizeof(recv_cid));
		#if DEBUG
		printf ("read_status = %d\n", read_status);
		#endif
		
		if(read_status == 0) // client closed
		{
			break;
		}
		else if(read_status < 0) // read error
		{
			perror("read error");
			break;
		}
		read(conn_fd, recv_payload, sizeof(recv_payload));
		unsigned short client_id = (unsigned short)(recv_cid[1] << 8) + (unsigned short)recv_cid[0];
		client_id = ntohs(client_id);

		#if DEBUG
		printf("received CID:%d\n", client_id);
		#endif

		// output to stdout
		printf("[chd](%d)[cid](%d)[ECH_RQT] %s", getpid(), client_id, recv_payload);
		
		// add veri_code to send_buf in the first 2 bytes
		unsigned short vcd = htons(atoi(veri_code));
		#if DEBUG
		printf("sending VCD:%d\n", vcd);
		#endif
		memcpy(send_buf, &vcd, 2);
		memcpy(send_buf+2, recv_payload, strlen(recv_payload));
		send_len = strlen(send_buf);
		// send data payload to client
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
	struct sigaction act_sigint;
	act_sigint.sa_flags = 0;
	act_sigint.sa_handler = sigint_handler;
	sigemptyset(&act_sigint.sa_mask);
	sigaction(SIGINT, &act_sigint, NULL);

	//install SIGCHLD
	struct sigaction act_sigchld;
	act_sigchld.sa_flags = 0;
	act_sigchld.sa_handler = sigchld_handler;
	sigemptyset(&act_sigchld.sa_mask);
	sigaction(SIGCHLD, &act_sigchld, NULL);

	//install SIGPIPE
	struct sigaction act_sigpipe;
	act_sigpipe.sa_flags = 0;
	// act_sigpipe.sa_handler = SIG_IGN;
	act_sigpipe.sa_handler = sigpipe_handler;
	sigemptyset(&act_sigpipe.sa_mask);
	sigaction(SIGPIPE, &act_sigpipe, NULL);

	//Create listen socket
	int listen_fd = socket(PF_INET, SOCK_STREAM, 0); // PF_INET:IPv4, SOCK_STREAM:TCP
	struct sockaddr_in srv_address;
	// memset(&srv_address, 0, sizeof(srv_address));
	bzero(&srv_address, sizeof(srv_address));
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
	listen(listen_fd, BACKLOG);
	printf("[srv](%d)[srv_sa](%s:%s)[vcd](%s) Server has initialized!\n", getpid(), ip_address, port, veri_code);
	
	// Create conn_fd
	int conn_fd;
	struct sockaddr_in cli_address;
	socklen_t cli_address_len;
	bzero(&cli_address, sizeof(cli_address));

	// Major Cycle
	while(!sigint_flag)
	{	
		// Minor Cycle
		cli_address_len = sizeof(cli_address);
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
		#if DEBUG
		//print the client ip and port
		printf("Client IP: %s\tport: %d\n", inet_ntoa(cli_address.sin_addr), ntohs(cli_address.sin_port));
		#endif

		// connected
		printf("[srv](%d)[cli_sa](%s:%d) Client is accepted!\n", getpid(), inet_ntoa(cli_address.sin_addr), ntohs(cli_address.sin_port));

		// fork a child process to handle the client
		pid_t pid_chld;
		if((pid_chld = fork()) < 0)
		{
			perror("fork error");
			break;
		}
		else if(pid_chld == 0) // child process
		{
			printf("[chd](%d)[ppid](%d) Child process is created!\n", getpid(), getppid());

			// close listen_fd
			if(close(listen_fd) < 0)
			{
				perror("close error");
			}

			// handle the client
			server_process(conn_fd);
			
			// client is closed in server_process()
			printf("[chd](%d)[ppid](%d)[cli_sa](%s:%d) Client is closed!\n", getpid(), getppid(), inet_ntoa(cli_address.sin_addr), ntohs(cli_address.sin_port));

			// close conn_fd
			if(close(conn_fd) < 0)
			{
				perror("close error");
			}
			printf("[chd](%d)[ppid](%d) connfd is closed!\n", getpid(), getppid());
			printf("[chd](%d)[ppid](%d) Child process is to return!\n", getpid(), getppid());

			return 0;
		}
		else if(pid_chld > 0) // parent process
		{
			// close conn_fd
			if(close(conn_fd) < 0)
			{
				perror("close error");
			}

			continue;
		}
	}

	//close
	// if(close(listen_fd) < 0)
	// {
	// 	perror("close error");
	// }
	// printf("[srv] listen_fd is closed!\n");
	// printf("[srv] server is to return!");
    return 0;
}
