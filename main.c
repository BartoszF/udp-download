#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>


#define NUM 10

int main(int argc, char* argv[])
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "socket error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
	}

	int flags = fcntl(sockfd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(sockfd, F_SETFL, flags);

	if (argc != 4) {
		fprintf (stderr, "usage: %s port filename size\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	int port = strtol(argv[1], NULL, 10);
	char* file = argv[2];
	int size = strtol(argv[3], NULL, 10);
	char ssize[6];
	sprintf(ssize, "%d", size);
	struct hostent* host = gethostbyname((char *)"aisd.ii.uni.wroc.pl");

	struct sockaddr_in server_address;
	bzero (&server_address, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(port);
	server_address.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
	//inet_pton(AF_INET, "aisd.ii.uni.wroc.pl", &server_address.sin_addr);
	
	printf("Ready!\n");

	int ret = 0;

	char message[32] = "GET 0 ";
	strcat(message, ssize);
	strcat(message, "\n");
	ssize_t message_len = strlen(message);
	int done = 0;

	while(done != 1)
	{
	  if(ret <= 0)
	  {
	  	if (sendto(sockfd, message, message_len, 0, (struct sockaddr*) &server_address, sizeof(server_address)) != message_len) {
		  fprintf(stderr, "sendto error: %s\n", strerror(errno)); 
		  return EXIT_FAILURE;		
	        }
	  	else
	    	  printf("Sent!\n");

		ret = NUM;
	  }
	  else
	  {
	    ret--;
	    sleep(1);
	  }

	  char buf[size + 11];
	  int sin_size = sizeof(server_address);

	  if (recvfrom(sockfd, buf, size + 11, 0, (struct sockaddr*) &server_address, (socklen_t*) &sin_size)==-1)
          {printf("Nope %d\n",size);}
	  else
	  {
            printf("Received packet from %s:%d\n\n",
               inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));
	    //fwrite(buf,1,sizeof(buf),stdout);
	    done = 1;
	    FILE *pFile = NULL;
 	    pFile = fopen(file, "w");

	    fwrite(buf,1,sizeof(buf), pFile);
	    //fprintf(pFile, "%s", buf);
	    fclose(pFile);
	  }
	}

	close (sockfd);
	return EXIT_SUCCESS;
}
	
