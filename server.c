#include<sys/socket.h>
#include<sys/epoll.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>


#define PORT 8888
#define MAXFDS 5000
#define EVENTSIZE 100

#define BUFFER "HTTP/1.1 200 OK\r\nContent-length: 5\r\nConnection: close\r\nContent-Type : text/html\r\n\nHello"

int epfd;

void *server_epoll(void *p);

void setnonblocking(int fd)
{
	int opts;
	opts = fcntl(fd,F_GETFL);
	if(opts < 0 )
	{
		fprintf(stderr,"fcntl failed\n");
		return;
	}
	
	opts = opts | O_NONBLOCK;
	if(fcntl(fd,F_SETFL,opts) < 0 )
	{
		fprintf(stderr,"fcntl failed\n");
		return;
	}

	return ;
}


int main(int argc, char* argv[])
{
	int fd, cfd,opt = 1;

	struct epoll_event event;
	struct sockaddr_in sin,cin;
	socklen_t sin_len = sizeof(struct sockaddr_in);

	pthread_t tid;
	pthread_attr_t attr;

	epfd = epoll_create(MAXFDS);
	if(( fd = socket(AF_INET,SOCK_STREAM,0)) <= 0)
	{
		fprintf(stderr,"socket failed\n");
		return -1;
	}

	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(const void*) &opt,sizeof(opt));

	memset(&sin,'\0',sizeof(struct sockaddr_in));

	sin.sin_family = AF_INET;
	sin.sin_port = htons((short)(PORT));
	sin.sin_addr.s_addr = INADDR_ANY;
	if(bind(fd,(struct sockaddr*) &sin,sizeof(sin)) != 0)
	{
		fprintf(stderr,"bind failed\n");
		return -1;
	}

	if(listen(fd,2048) != 0)
	{
		fprintf(stderr,"listen failed\n");
		return -1;
	}

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if(pthread_create(&tid,&attr,server_epoll,NULL) != 0)
	{
		fprintf(stderr,"pthread_create failed\n");
		return -1;
	}


	while((cfd = accept(fd,(struct sockaddr * ) &cin,&sin_len)) > 0)
	{
		setnonblocking(cfd);
		event.data.fd = cfd;
		event.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP | EPOLLPRI;
		epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&event);
		printf("connect from %s\n",inet_ntoa(cin.sin_addr));
	}

	if(fd > 0)
		close(fd);
	return 0;
}

void *server_epoll(void *p)
{
	int i ,ret, cfd, nfds;
	struct epoll_event event, events[EVENTSIZE];
	char buffer[512];
	
	while(1)
	{

		nfds = epoll_wait(epfd,events,EVENTSIZE,-1);

		for(i=0;i<nfds;i++)
		{
			if(events[i].events & EPOLLIN)
			{
				cfd = events[i].data.fd;
				ret = recv(cfd,buffer,sizeof(buffer),0);
				
				printf("revevice from %d :%s\n",cfd,buffer);
				
				event.data.fd = cfd;
				event.events =  EPOLLET;
			//	event.events = EPOLLOUT | EPOLLET;

				epoll_ctl(epfd,EPOLL_CTL_MOD,cfd,&event);
			}
			else if (events[i].events & EPOLLOUT)
			{
				cfd = events[i].data.fd;
				ret = send(cfd,BUFFER,atoi(buffer),0);
				printf("send to %d:\n",cfd);
				
				event.data.fd = cfd;
				event.events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd,EPOLL_CTL_MOD,cfd,&event);
				//close(cfd);
			}
			else {
				cfd = events[i].data.fd;
				event.data.fd = cfd;
				epoll_ctl(epfd,EPOLL_CTL_DEL,cfd,&event);
				close(cfd);
			}

		}
	
	}

	return NULL;

}

