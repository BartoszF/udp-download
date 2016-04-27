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

#define TRUE 1
#define FALSE 0

#define NUM 300
#define PARTSIZE 200

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

	if (argc != 4) 
  {
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
  
  int sin_size = sizeof(server_address);
	
	printf("Ready!\n");

	int ret = 0;
  
	int done = FALSE;
  int parts = size / PARTSIZE;
  if ( size - (parts * PARTSIZE) > 0) parts++;
  
  int partsDone[parts];
  for(int i=0;i<parts;i++) partsDone[i] = FALSE;
  
  int part = 0;
  
  char buf[size];

	while(done != 1)
	{
      if(ret <= 0)
      {
        int start = part * PARTSIZE;
		char sstart[6];
		sprintf(sstart, "%d", start);
        int l = PARTSIZE;
		char sl[6];
        
        if( start + l > size ) l = size - start;
		
		sprintf(sl, "%d", l);
        
        char message[32] = "GET ";
        strcat(message, sstart);
        strcat(message, " ");
        strcat(message, sl);
        strcat(message, "\n");
        ssize_t message_len = strlen(message);
  
        if (sendto(sockfd, message, message_len, 0, (struct sockaddr*) &server_address, sizeof(server_address)) != message_len) 
        {
          fprintf(stderr, "sendto error: %s\n", strerror(errno)); 
          return EXIT_FAILURE;		
        }
        //else
          //printf("Sent! %s\n", message);
        
        part++;
        if(part == parts) 
        {
          ret = NUM;
          part=0;
        }
      }
      else
      {
        ret--;
      }
      
      char nbuf[PARTSIZE + 11];
    
      if (recvfrom(sockfd, nbuf, PARTSIZE + 11, 0, (struct sockaddr*) &server_address, (socklen_t*) &sin_size)==-1)
      { 
        //printf("Nope %d\n",size); 
      }
      else
      {
        /*if(server_address.sin_addr != *((struct in_addr *)host->h_addr_list[0]))  //Something like that?
        {
          printf("Wrong address!\n");
          continue;
        }
        else
        {*/
          printf("Received packet from %s:%d\n\n",
            inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));
            
          char* command = strtok(nbuf, "\n");  //Split command
          char* data = strtok(nbuf, "\n");            //And put rest into buffer
          printf("%s\n",command);
          //fwrite(data,1,sizeof(data),stdout);
          
          char* c = strtok(command, " ");
		  char* st = strtok(NULL, " ");
		  char* si = strtok(NULL, " ");
          int sst = strtol(st, NULL, 10);
          int ssi = strtol(si, NULL, 10);
          
          partsDone[sst/PARTSIZE] = TRUE;
		  printf("%s %s %s\n", command, st, si);
		  printf("Part done : %d\nsst : %d\nssi : %d\n", sst/PARTSIZE, sst, ssi);
          for(int i=0;i<ssi;i++)
          {
            buf[sst+i] = data[i];
          }
          
          // Write to file part //
          //done = TRUE;
        //}
      }
      
      int pDone = 0;
      for(int i=0;i<parts;i++)
      {
        if(partsDone[i] == FALSE) break;
		//printf("%d ", i);
        pDone ++;
      }
	  //printf("\n");
      
      if( pDone == parts-1) break;

      
	}
  FILE *pFile = NULL;
  pFile = fopen(file, "w");

  fwrite(buf,1,sizeof(buf), pFile);
  //fprintf(pFile, "%s", buf);
  fclose(pFile);
        
	close (sockfd);
	return EXIT_SUCCESS;
}
	
