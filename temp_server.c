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

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void handle_sigchld(int sig) {
    pid_t pid;
    int status;
    while (( pid = waitpid(-1, &status, WNOHANG)) > 0) {
        /* NOT REENTRANT! Avoid in practice! */
        // printf("Reaped child %d\n", pid);
    }
    /* No more zombie children to reap. */
}

int main(int argc, char *argv[])
{
    // Signal handling code
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigchld;
    sigaction(SIGCHLD, &sa, NULL);

sigaction(SIGCHLD, &sa, NULL);
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

    int pid,counter=0,status,w;
     /* accept a new request, create a newsockfd */
    while(1)
    {
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
            counter++;
            printf("here2\n");
            continue;
        }
        else if(pid == 0)
        {
            /* read message from client */
            bzero(buffer,256);
            n = read(newsockfd,buffer,255);
            if (n < 0) error("ERROR reading from socket");
            printf("counter%d\n",counter );
            printf("Here is the message: %s\n",buffer);

            /* send reply to client */

            n = write(newsockfd,"I got your message",18);
            if (n < 0) error("ERROR writing to socket");
            close(newsockfd);
            return 0;
        }
    }
     return 0; 
}
