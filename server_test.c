#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdbool.h>
#include<strings.h>
#include<assert.h>
#include<time.h>




static const char* request= " GET http://localhost/index.html HTTP/1.1\r\nConnection: keep-alive\r\n\r\nxxxxxx";

int setnonblocking(int fd)
{
	int old_option = fcntl(fd,F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	
	fcntl(fd,F_SETFL,new_option);
	return old_option;
}


void addfd(int epoll_fd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLOUT | EPOLLET | EPOLLERR;
	epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&event);
	setnonblocking(fd);
}

bool write_nbytes(int sockfd, const char* buffer, int len)
{
	int bytes_write = 0;
	printf("write out %d bytes to socket %d\n",len,sockfd);
	while(1)
	{
		bytes_write = send(sockfd,buffer,len,0);
		if( bytes_write == -1 )
		{
			return false;
		}
		else if ( bytes_write == 0 )
		{
			return false;
		}
		
		len -= bytes_write;
		buffer = buffer + bytes_write;
		if(len <= 0 )
		{
			return true;
		}

	}
}

bool read_once(int sockfd,char* buffer ,int len)
{
	int bytes_read = 0;
	memset(buffer,'\0',len);
	bytes_read = recv(sockfd,buffer,len,0);
	if(bytes_read == -1)
	{
		return false;
	}
	else if( bytes_read == 0 )
	{
		return false;
	}
	
	printf(" read in %d bytes from socket %d with content: %s\n",bytes_read,sockfd,buffer);
	
	return true;
}

void start_conn(int epoll_fd,int num, const char* ip, int port,char *localip)
{
	int ret = 0;
	struct sockaddr_in address;
	bzero(&address,sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET,ip,&address.sin_addr);
	address.sin_port = htons(port);

	printf("connect ip :%s\n",localip);	
	for( int i = 0; i < num; ++i)
	{
		//sleep(1);
		struct timespec time1;  
		time1.tv_sec=0;
		time1.tv_nsec=100000L;
		nanosleep(&time1,NULL);
		int sockfd = socket(AF_INET,SOCK_STREAM,0);

		struct sockaddr_in clnt_addr;
		bzero(&clnt_addr,sizeof(clnt_addr));
		clnt_addr.sin_family= AF_INET;
		clnt_addr.sin_addr.s_addr = inet_addr(localip);
		if(bind(sockfd,(struct sockaddr *) &clnt_addr,sizeof(clnt_addr)) < 0)
		{
			printf("error on binding");
			return;
		}

		printf("create 1 sock \n");
		if(sockfd < 0 )
		{
			printf("error on create socket\n");
			continue;
		}

		if(connect(sockfd,(struct sockaddr*) &address,sizeof(address)) == 0)
		{
			printf("bulid connection %d\n",i);
			addfd(epoll_fd,sockfd);
		}
	}
}

void close_conn(int epoll_fd,int sockfd)
{
	epoll_ctl(epoll_fd,EPOLL_CTL_DEL,sockfd,0);
	close(sockfd);
}

int main(int argc,char* argv[])
{
	assert(argc == 4);
	
	int epoll_fd = epoll_create(100);
	
	char ip_array[300] = "192.168.6.101,192.168.6.102,192.168.6.103,192.168.6.104,192.168.6.105,192.168.6.106,192.168.6.107,192.168.6.108,192.168.6.109";	
	char *ori_ip = NULL;
	char delims[] =",";
	ori_ip = strtok(ip_array,delims);
	while(ori_ip != NULL)	
	{
		start_conn(epoll_fd,atoi(argv[3]),argv[1],atoi(argv[2]),ori_ip);
		ori_ip = strtok(NULL,delims);
	}

	struct epoll_event events[10000];
	char buffer[2048];

	while(1)
	{
		int fds = epoll_wait(epoll_fd,events,10000,2000);
		for(int i = 0; i<fds;i++)
		{
			int sockfd = events[i].data.fd;
			if(events[i].events & EPOLLIN)
			{
				if(!read_once(sockfd,buffer,2048))
				{
					close_conn(epoll_fd,sockfd);
				}
		

				struct epoll_event event;
				event.events = EPOLLOUT | EPOLLET | EPOLLERR;
				event.data.fd = sockfd;
				epoll_ctl(epoll_fd,EPOLL_CTL_MOD,sockfd,&event);
	
			}
			else if( events[i].events & EPOLLOUT)
			{
				if(!write_nbytes(sockfd,request,strlen(request)))
				{
					close_conn(epoll_fd,sockfd);
				}
				struct epoll_event event;
				event.events = EPOLLIN | EPOLLET | EPOLLERR;
				event.data.fd = sockfd;
				epoll_ctl(epoll_fd,EPOLL_CTL_MOD,sockfd,&event);

			}
			else if (events[i].events & EPOLLERR)
			{
				close_conn(epoll_fd,sockfd);
			}

		}
	
	}

}
								
