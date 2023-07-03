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


int main(int argc, char *argv[]) {
  // arg1=ip_address arg2=port
  ip_address = argv[1];
  port = argv[2];
  client_id = argv[3];

  char recv_payload[MAX_MSG_BUF_LEN], recv_vcd[2];
  char send_buf[MAX_MSG_BUF_LEN], send_cid[2], send_payload[MAX_MSG_BUF_LEN];
  int recv_len, send_len;

  short cid = htons(atoi(client_id));

	// print cid
	  printf("DEBUG:cid=%d\n", cid);
	// transfer cid into 2 bytes
	
  memcpy(send_buf, &cid, sizeof(cid));
//   printf("DEBUG:cid=%d\n", atoi(client_id));
  printf("DEBUG: SEND_BUF:%sSEND_BUF_END\n", send_buf);
  memcpy(send_buf + 2, send_payload, strlen(send_payload));
  send_len = strlen(send_buf);
  
  printf("DEBUG: SEND_BUF:%sSEND_BUF_END\n", send_buf);

  return 0;
}
