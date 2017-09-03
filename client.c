#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define DEFAULT_PORT 8000
#define MAXBUF 4096

int main(int argc, char **argv){
	int socket_fd;
	struct  sockaddr_in servaddr;
	char recvbuf[MAXBUF];
	char sndbuf[MAXBUF] = "hello server...";
	int recv_len;

	if((socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		fprintf(stderr,"client sock create error...\n");
		exit(1);
	}

	memset(&servaddr, 0 ,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(DEFAULT_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//connect and send/recv
	if( connect(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
		printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
		exit(1);
	}
	
	printf("connect succeed..\n");
	printf("say hello to server...\n");
	if( send(socket_fd, sndbuf, strlen(sndbuf), 0) < 0){//send
		printf("send msg error:%s (errno:%d)", strerror(errno), errno);
		exit(1);
	}
	if( (recv_len = recv(socket_fd, recvbuf, MAXBUF, 0)) < 0 ){//recv
		printf("recv error:%s (errno:%d)", strerror(errno), errno);
		exit(1);
	}
	else{
		recvbuf[recv_len]='\0';
		printf("recv:%s\n", recvbuf);
	}
	
	close(socket_fd);
	return 0;
}	
