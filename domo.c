#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyUSB0" 
#define _POSIX_SOURCE 1

#define FALSE 0
#define TRUE 1


volatile int STOP=FALSE;

int open_arduino();
int create_server();
void post_pachube(char *temp);
int read_from_client(int fd, char *temp);

int create_server()
{
	int fd;
	struct sockaddr_in serv_addr;
	
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0)
	{
		perror("failed creating server socket.\n");
		exit(-1);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(8080);

	bind(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	listen(fd, 10);

	return fd;
}

int open_arduino()
{
	int fd;
	struct termios tio;
	
	fd = open(MODEMDEVICE, O_RDWR | O_NONBLOCK);
	if (fd<0)
	{
		perror(MODEMDEVICE); 
		exit(-1);	
	}

	memset(&tio,0, sizeof(tio));
	tcgetattr(fd, &tio);
	
	//Input flags 
	//Turn off input processing
	//convert break to null byte, no CR to NL translation
	//no NL to CR translation, don´t mark parity errors or breaks
	//no XON/XOFF software flow control
	tio.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
	
	// Output flags - Turn off output processing
	// no CR to NL translation, no NL to CR-NL translation,
	// no NL to CR translation, no column 0 CR suppression,
	// no fill characters, no case mapping,
	// no local output processing
	tio.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR |  OFILL | OLCUC | OPOST);

	//No line processing
	//echo off, canonical mode on 
	//extended input processing off, signal chars off
	tio.c_lflag &= ~(ECHO | ECHONL | IEXTEN | ISIG);
	tio.c_lflag |= ICANON;	

	//clear current char size mask, no parity checking,
	//no output processing, force 8 bit input
	tio.c_cflag &= ~(CSIZE | PARENB);
	tio.c_cflag |= CS8;

	//Config speed
	if (cfsetispeed(&tio, BAUDRATE) < 0 || cfsetospeed(&tio, BAUDRATE) < 0) {
		perror("Could not set speed comport");
		exit(-1);
	}

	//flushes UART data buffer
	tcflush(fd, TCIFLUSH);
	
	//Apply the config
	if (tcsetattr(fd, TCSANOW, &tio) < 0) {
		perror("Could not apply config to comport");
		exit(-1);
	}
		
	return fd;
}

void post_pachube(char *temp)
{
	int sockfd = 0, n = 0;
	char recvBuff[1024];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	const char *hostname = "api.pachube.com";
	char pachubeMsg[1024];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0)
	{
		perror("getaddrinfo failed.\n");
		exit(-1);
	}

	for(p = servinfo; p != NULL; p = p->ai_next){
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1){
			perror("socket");
			continue;
		}
		
		if(connect(sockfd, p->ai_addr,p->ai_addrlen) == -1){
			close(sockfd);
			perror("connect");
			continue;
		}

		break;
	}
	
	if (p == NULL){
		fprintf(stderr, "failed to connect\n");
		exit(-1);
	}

	memset(pachubeMsg, '0', sizeof(pachubeMsg));
	snprintf(pachubeMsg, sizeof(pachubeMsg),
		"PUT /v2/feeds/41769/datastreams/0.csv HTTP/1.1\n"
		"Host: api.pachube.com\nAccept: */* \n"
		"X-PachubeApiKey: NzdCL90dec7H_NKSVbA7mwYhHHkaqZTsNO6VdbUe-gs\n"
		"Content-Length: 4\nExpect: 100-continue\n\n");
	n = write(sockfd, pachubeMsg, strlen(pachubeMsg));
	
	//TODO: Check read
	//memset(recvBuff, '0', sizeof(recvBuff));
	//n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
	//recvBuff[n] = 0;
	
	memset(pachubeMsg, '0', sizeof(pachubeMsg));
	snprintf(pachubeMsg, sizeof(pachubeMsg), "%s\r\n", temp);
	n = write(sockfd, pachubeMsg, strlen(pachubeMsg));

	//TODO: Check read
	//memset(recvBuff, '0', sizeof(recvBuff));
	//n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
	//recvBuff[n] = 0;
	
	close(sockfd);
	freeaddrinfo(servinfo);		
}

int read_from_client(int fd, char *temp)
{
	int n;

	n = read(fd, temp, 255);
	temp[n] = 0;
	
	if (n  < 0){
		perror("read from client");
		exit(-1);
	}
	else if (n == 0){
		return -1;
	}
	else if (strncmp(temp, "quit", 4) == 0){
		return -1;
	}

	fprintf(stderr, "Server: got message: %s", temp);
	return 0;

}


int main(int argc, char *argv[])
{
	int comfd, sockfd, connfd;
	fd_set active_fd_set, read_fd_set;
	char buf[255];
	time_t rawtime;
	struct tm *timeinfo;
	struct sockaddr_in cli_addr;
	socklen_t clilen;
	int i;
	int res, n;

	comfd = open_arduino();
	sockfd = create_server();
	
	FD_ZERO(&active_fd_set);
	FD_SET(sockfd, &active_fd_set);
	FD_SET(comfd, &active_fd_set);

	while (STOP == FALSE){
	
		read_fd_set = active_fd_set;
		
		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0){
			perror("select");	
			exit(-1);
		}
	
		for (i = 0; i < FD_SETSIZE; ++i){
			if(FD_ISSET (i, &read_fd_set)){
				if (i == sockfd){
					clilen = sizeof(cli_addr);	
					connfd = accept(sockfd, 
					(struct sockaddr *) &cli_addr, &clilen);
					
					if (connfd < 0){
						perror ("accept");
						exit(-1);
					}
					fprintf (stderr, 
						"Server: connect from host %s, port %hd.\n",
						inet_ntoa (cli_addr.sin_addr),
						ntohs (cli_addr.sin_port));
					
					memset(buf, '0', sizeof(buf));
					snprintf(buf, sizeof(buf),"Pls, give temp: ");
					int n = write(connfd, buf, strlen(buf));
					if (n < 0){
						perror("write to client");
						exit(-1);
					}
					FD_SET(connfd, &active_fd_set);
				}
				else if (i == comfd){
					time(&rawtime);
					timeinfo = localtime(&rawtime);
					res = read(comfd, buf, 255);
					buf[res] = 0;
					
					if (res == 4){
						fprintf(stderr, "Time and date: %sTemp: %s", 
							asctime(timeinfo), buf);
						post_pachube(buf);
						res = 0;
					}
				}
				else if (i == connfd){
					memset(buf, '0', sizeof(buf));
					res = read_from_client(i, buf);
					if (res == 0){
						n = write(comfd, buf, strlen(buf));
						if (n < 0){
							perror("write to arduino");
							exit(-1);
						}	
					}
					else if(res  < 0) {
						close(i);
						FD_CLR(i, &active_fd_set);
					}
				}

			}
		}		

	}
	return 0;
}	
