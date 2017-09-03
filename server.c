#include <stdio.h>
#include <stdlib.h>//exit()
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
//#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>//read write open close

#define DEFAULT_PORT 8000
#define MAXBUF 4096
#define MAXCLIENT 5

int main(int argc ,char **argv){
	int socket_fd, connect_fd;
	struct sockaddr_in servaddr;
	struct sockaddr_in client_addr[MAXCLIENT];
	char buff[MAXBUF];
	char snd_buff[MAXBUF]="";
	int cur_client_num=0;
	int sin_size = sizeof(struct sockaddr_in);
	int epfd;
	int nfds;
	struct epoll_event ev_server[256];
	struct epoll_event new_ev;
	int i;	

	epfd = epoll_create(1024);//max poll fds is 1024
	if(epfd < 0 ){
		fprintf(stderr, "epfd create error...\n");
		exit(1);
	}
		
	if((socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		fprintf(stderr,"socket create error...\n");
		exit(1);
	}
	//init addr struct
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;//ipv4
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//detect local ip addr automatically
	servaddr.sin_port = htons(DEFAULT_PORT);
	
	//bind
	if( bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1 ){
		fprintf(stderr, "bind error...\n");
		exit(1);
	}

	//listen and accept connect request
	if(listen(socket_fd, MAXCLIENT) == -1){//request queue length is 5
		fprintf(stderr,"listen error...\n");
		exit(1);
	}
	
	new_ev.data.fd = socket_fd;
	new_ev.events = EPOLLIN|EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, socket_fd, &new_ev);
	printf("update epoll wait list for server_socket...\n");	

	printf("waiting for client...\n");
	
	int n;
	while(1){
	/*
		if( (connect_fd = accept(socket_fd, (struct sockaddr*)&(client_addr[cur_client_num]), &sin_size)) == -1 ){
			fprintf(stderr,"accept error...\n");
			continue;
		}
		else{
			cur_client_num = (cur_client_num+1) % MAXCLIENT;
		}
	*/
		nfds = epoll_wait(epfd, ev_server, 20, 500);//epoll wait
		for(i=0; i<nfds; i++){//if something occur
			if(ev_server[i].data.fd == socket_fd){//event fd is server socket_fd , that means a connect request come
				if(	(connect_fd = accept(socket_fd, (struct sockaddr *)NULL, NULL)) == -1 ){
					fprintf(stderr, "accept error...\n");
					continue;
				}
				printf("a client connect done...\n");
				new_ev.data.fd = connect_fd;
				new_ev.events = EPOLLIN|EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connect_fd, &new_ev);//add connect_fd to epoll fd list		
			}
			else if(ev_server[i].events & EPOLLIN){//find a read event
				connect_fd = ev_server[i].data.fd;//got event fd
				if(connect_fd < 0)
					continue;
				if((n = read(connect_fd, buff, MAXBUF)) < 0){//try read data for con_fd
					if(errno == ECONNRESET){
						close(connect_fd);
						ev_server[i].data.fd = -1;
					}
					fprintf(stderr, "read error...\n");
					memset(buff, 0, MAXBUF);
					continue;
				}
				else if(n == 0){
					close(connect_fd);
					ev_server[i].data.fd = -1;
				}
				buff[n]='\0';
				printf("client:%s\n",buff);
				new_ev.data.fd = connect_fd;
				new_ev.events = EPOLLOUT|EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_MOD, connect_fd, &new_ev);//add out event for this con_fd						
			}
			else if(ev_server[i].events & EPOLLOUT){//find a write event
				if(!fork()){//fork a son process to send data
					connect_fd = ev_server[i].data.fd;
					memset(snd_buff, 0, MAXBUF);
					strcpy(snd_buff,"hello client...\0");
					write(connect_fd, snd_buff, strlen(snd_buff));
					exit(0);
				}
				new_ev.data.fd = connect_fd;
				new_ev.events = EPOLLIN|EPOLLET;
				epoll_ctl(epfd,EPOLL_CTL_MOD,connect_fd,&new_ev);//add in event for this con_fd
			}
		}
	}

	close(epfd);
	return 0;
}
