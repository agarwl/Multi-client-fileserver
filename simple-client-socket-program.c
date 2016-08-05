#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include <string.h>

#define min(a, b) ((a < b) ? a : b)
#define NUM_FILES 10000
#define BUF_SIZE 30

void *connection(void *);
int readdata(int sock, void *buf, int buflen);
int readlong(int sock, long *value);
int readfile_and_discard(int sock, FILE *f);
struct hostent *server;
int portno,runtime,sleeptime;
char *mode;


void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, n;

    struct sockaddr_in serv_addr;

   
    if (argc < 7) {
       fprintf(stderr,"usage %s hostname port users time sleep mode\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);
    server = gethostbyname(argv[1]);
    int num_client = atoi(argv[3]);
    runtime = atoi(argv[4]);
    sleeptime = atoi(argv[5]);
    mode = argv[6];

    pthread_t tid[num_client];
    int i;
    for (i=0; i<num_client; i++) 
    {
        if( pthread_create( &tid[i] , NULL ,  connection , (void*) i) < 0)
        {
            error("could not create thread");
        }
    }
    for (i = 0; i < num_client; i++)
       pthread_join(tid[i], NULL);

   return 0;

}

void *connection(void *threadid)
{
    int threadnum = (int)threadid;
    int sockfd;
    struct sockaddr_in serv_addr;
    int n;

    char filename[BUF_SIZE];
    char temp[5];
    int filenum;                    // read the files/foo0.txt file by default if the mode is fixed


    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    time_t start= time(NULL);
    time_t now;
    
    while (1)
    {
        now = time(NULL);
        if ( now - start > runtime)
            break;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) 
            error("ERROR opening socket");

        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
            error("ERROR connecting");

        // printf("Connected successfully client:%d\n", threadnum);

        if(strcmp(mode,"random") == 0)
            filenum = rand() % NUM_FILES;
        else if(strcmp(mode,"fixed") == 0)
            filenum = 0;
       
        bzero(filename,BUF_SIZE); 
        strcpy(filename, "get files/foo");
        sprintf(temp,"%d",filenum);
        strcat(filename,temp);
        strcat(filename,".txt");

        n = write(sockfd,filename,strlen(filename));
        if (n < 0) error("ERROR writing to socket");

        FILE *filehandle = fopen(filename+10, "wb");
        printf("%s\n",filename );
        if (filehandle != NULL)
        {
            int ok = readfile_and_discard(sockfd, filehandle);
            fclose(filehandle);
            if (ok == 1){
                printf("File received: %s\n", filename+10);
            }
            else
            {
                remove(filename);
                error("File send fail");
            }
        }
        sleep(sleeptime);
        close(sockfd);
    }
    
    return 0;
}

int readdata(int sock, void *buf, int buflen)
{
    unsigned char *pbuf = (unsigned char *) buf;

    while (buflen > 0)
    {
        int num = recv(sock, pbuf, buflen, 0);
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

int readfile_and_discard(int sock, FILE *f)
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
