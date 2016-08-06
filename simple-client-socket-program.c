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
int read_and_discard(int sock);//, FILE *f);
struct hostent *server;
int portno,runtime,sleeptime;
char *mode;

int *num_requests;
double *response_time;

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

    num_requests = malloc(num_client * sizeof(int));
    response_time =  malloc(num_client * sizeof(double));

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

   double total_req=0,throughput,avg_response_time=0;
   for (int i = 0; i < num_client; ++i)
   {
        total_req += num_requests[i];
        avg_response_time += response_time[i];
       /* code */
   }
   throughput = total_req/runtime;
   avg_response_time /= total_req;

   printf("throughput = %f req/s\n",throughput);
   printf("average response time = %f sec\n",avg_response_time);

   free(num_requests);
   free(response_time);

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


    num_requests[threadnum] = 0;
    response_time[threadnum] = 0;

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
        now = time(NULL);

        int ok = read_and_discard(sockfd);
        if (ok == 1){
            response_time[threadnum] += (time(NULL) - now);
            num_requests[threadnum]++;
            printf("File received: %s\n", filename+10);
        }
        else
        {
            error("File send fail");
        }

        sleep(sleeptime);
        close(sockfd);
    }
    
    return 0;
}

int read_and_discard(int sock) 
{
    char buffer[1024];
    size_t bufflen = sizeof(buffer);
    int bytes_read;
    while(( bytes_read = read(sock, buffer, bufflen) ) > 0);
    // {
    //      size_t written = fwrite(buffer, 1, bytes_read, f);
    // }
    if(bytes_read == 0)
        return 1;
    else
        return 0;
}