#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define min(a, b) ((a < b) ? a : b)
#define num_client 5

void *connection(void *);
int readdata(int sock, void *buf, int buflen);
int readlong(int sock, long *value);
int readfile(int sock, FILE *f);
struct hostent *server;
int portno;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, n;

    struct sockaddr_in serv_addr;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    /* create socket, get sockfd handle */

    // portno = atoi(argv[2]);
    // sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockfd < 0) 
    //     error("ERROR opening socket");
    portno = atoi(argv[2]);
    server = gethostbyname(argv[1]);

    pthread_t my_thread;
    int i;
    for (i=1; i<=num_client; i++) 
    {
        if( pthread_create( &my_thread , NULL ,  connection , (void*) i) < 0)
        {
            perror("could not create thread");
            return 1;
        }
    }
    pthread_exit(NULL);

}

void *connection(void *threadid)
{
    int threadnum = (int)threadid;
    int sockfd;
    struct sockaddr_in serv_addr;
    int n;
    // char buffer[256],buffer[256];
    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    // struct hostent *server = gethostbyname(hostinfo);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    // serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    printf("Connected successfully client:%d\n", threadnum);

    printf("Running thread : %d\n", threadnum);
    char filename [20];
    char temp[5];
    strcpy(filename, "test");
    sprintf(temp,"%d",threadnum);
    strcat(filename,temp);
    strcat(filename,".txt");
    FILE *filehandle = fopen(filename, "wb");
    if (filehandle != NULL)
    {
        int ok = readfile(sockfd, filehandle);
        fclose(filehandle);
        if (ok == 1)
            printf("File received: %s\n", filename);
        
        else
        {
            remove(filename);
            error("File send fail");
        }
    }

    close(sockfd);
    return 0;
}

int readdata(int sock, void *buf, int buflen)
{
    unsigned char *pbuf = (unsigned char *) buf;

    while (buflen > 0)
    {
        int num = recv(sock, pbuf, buflen, 0);
        // if (num == SOCKET_ERROR)
        // {
        //     if (WSAGetLastError() == WSAEWOULDBLOCK)
        //     {
        //         // optional: use select() to check for timeout to fail the read
        //         continue;
        //     }
        //     return 0;
        // }
        if (num <= 0)
            return 0;

        pbuf += num;
        buflen -= num;
    }

    return 1;
}

int readlong(int sock, long *value)
{
    if (!readdata(sock, value, sizeof(value)))
        return 0;
    *value = ntohl(*value);
    return 1;
}

int readfile(int sock, FILE *f)
{
    long filesize;
    
    if (!readlong(sock, &filesize))
        return 0;
    if (filesize > 0)
    {
        char buffer[1024];
        do
        {
            int num = min(filesize, sizeof(buffer));
            if (readdata(sock, buffer, num) == 0)
                return 0;
            int offset = 0;
            do
            {
                size_t written = fwrite(&buffer[offset], 1, num-offset, f);
                if (written < 1)
                    return 0;
                offset += written;
            }
            while (offset < num);
            filesize -= num;
        }
        while (filesize > 0);
    }
    return 1;
}
