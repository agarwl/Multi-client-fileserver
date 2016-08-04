/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define min(a, b) ((a < b) ? a : b)

int senddata(int sock, void *buf, int buflen);
int sendlong(int sock, long value);
int sendfile(int sock, FILE *f);
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     /* create socket */

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");

    // Used so we can re-bind to our port while a previous connection is still in TIME_WAIT state.
    int yes = 1;

    // For re-binding to it without TIME_WAIT problems 
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        error("setsockopt");
    }

     /* fill in port number to listen on. IP address can be anything (INADDR_ANY) */

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     /* bind socket to this port number on this machine */

     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     
     /* listen for incoming connection requests */

     listen(sockfd,5);
     clilen = sizeof(cli_addr);

     int pid,status,w;
     /* accept a new request, create a newsockfd */
     while(1){
        
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) 
            error("ERROR on accept");

        if((pid = fork()) == -1)
        {
            close(newsockfd);
            continue;
        }
        else if(pid > 0)
        {
            close(newsockfd);
            while (( w= waitpid(-1, &status, WNOHANG) ) > 0)
            {
                printf("Killed zombie %d\n", w);
            }
            continue;
        }
        else if(pid == 0)
        {
            FILE *filehandle = fopen("test.txt", "rb");
            if (filehandle != NULL)
            {
              sendfile(newsockfd, filehandle);
              fclose(filehandle);
            }
            close(newsockfd);
            return 0;
        }
    }


     /* send reply to client */

     // n = write(newsockfd,"I got your message",18);
     // if (n < 0) error("ERROR writing to socket");
     return 0; 
}



int senddata(int sock, void *buf, int buflen)
{
    unsigned char *pbuf = (unsigned char *) buf;

    while (buflen > 0)
    {
        int num = send(sock, pbuf, buflen, 0);
        // if (num == SOCKET_ERROR)
        // {
        //     if (WSAGetLastError() == WSAEWOULDBLOCK)
        //     {
        //         // optional: use select() to check for timeout to fail the send
        //         continue;
        //     }
        //     return 0;
        // }

        pbuf += num;
        buflen -= num;
    }

    return 1;
}

int sendlong(int sock, long value)
{
    value = htonl(value);
    return senddata(sock, &value, sizeof(value));
}

int sendfile(int sock, FILE *f)
{
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    rewind(f);
    if (filesize == EOF)
        return 0;
    if (sendlong(sock, filesize) == 0)
        return 0;
    if (filesize > 0)
    {
        char buffer[1024];
        do
        {
            size_t num = min(filesize, sizeof(buffer));
            num = fread(buffer, 1, num, f);
            if (num < 1)
                return 0;
            if (senddata(sock, buffer, num) == 0)
                return 0;
            filesize -= num;
        }
        while (filesize > 0);
    }
    return 1;
}    