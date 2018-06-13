#include "header.h"
#include "headerje.h"

// threads
pthread_t* tid;

int not; // num of threads global

// helping for threads to make directory
int crawler_fd;

// queue
Queue* urls;
Queue* urlscheck;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int canexit = 0;

int iscrawling = 0;
pthread_mutex_t crawllock = PTHREAD_MUTEX_INITIALIZER;

// statistics
pthread_mutex_t lock_pages = PTHREAD_MUTEX_INITIALIZER;
int number_of_pages = 0;

pthread_mutex_t lock_bytes = PTHREAD_MUTEX_INITIALIZER;
int number_of_bytes = 0;

int main(int argc, char** argv)
{
  if(mkfifo("search",0666) == -1) perror("Can't make fifo");
  int opt;
  int server_port, command_port, num_of_threads;
  char *host_or_ip, *save_dir, *starting_URL;

  // for stats -> time
  struct timeb start, end;
  int diff;
  ftime(&start);

  urls = malloc(sizeof(Queue));
  InitializeQueue(urls);
  urlscheck = malloc(sizeof(Queue));
  InitializeQueue(urlscheck);

  while((opt = getopt(argc,argv,"h:p:c:t:d:")) != -1){
    switch (opt)
    {
      case 'h':
        printf("h: %s\n", optarg);
        host_or_ip = optarg;
        break;
      case 'p':
        printf("p: %s\n", optarg);
        server_port = atoi(optarg);   // port that the server listens to
        break;
      case 'c':
        printf("c: %s\n", optarg);
        command_port = atoi(optarg);
        break;
      case 't':
        printf("t: %s\n", optarg);
        num_of_threads = atoi(optarg);
        not = num_of_threads;
        break;
      case 'd':
        printf("d: %s\n", optarg);
        save_dir = malloc(strlen(optarg)+1);
        strcpy(save_dir,optarg);
        break;
    }
  }
  starting_URL = argv[11];
  printf("host_or_ip = %s\nserver_port = %d\ncommand_port = %d\nnum_of_threads = %d\nsave_dir = %s\nstarting_URL = %s\n", host_or_ip, server_port, command_port, num_of_threads, save_dir, starting_URL);

  struct crawl* arg = malloc(sizeof(struct crawl));
  arg->host_or_ip = host_or_ip;
  arg->server_port = server_port;
  arg->save_dir = save_dir;

  chdir(save_dir);

  // thread pool
  tid = malloc(num_of_threads*sizeof(pthread_t));
  for(int i = 0; i < num_of_threads; i++)
    if(!pthread_create(&(tid[i]), NULL, make_directory, (void*) arg))
      perror("thread create");

  pthread_mutex_lock(&crawllock);
  iscrawling = 1;
  pthread_mutex_unlock(&crawllock);

  // connected to server, put starting_URL in queue
  ItemType item;
  item.url = starting_URL;
  pthread_mutex_lock(&lock);
  Insert(item,urls);
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&lock);

  // --------------------------------------------------------------------------- here ends the crawling

  // socket creation for command
  struct sockaddr_in com_addr;
  struct sockaddr_in cli_addr;

  int sockfdc;
  if((sockfdc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    perror("socket command");

  // set buffer to zero
  bzero((char *) &com_addr, sizeof(com_addr));

  com_addr.sin_family = AF_INET;
  com_addr.sin_port = htons(command_port); // convert to internet order
  com_addr.sin_addr.s_addr = INADDR_ANY;   // INADDR_ANY = host IP

  if(bind(sockfdc, (struct sockaddr *) &com_addr,sizeof(com_addr)) < 0)
    perror("bind");

  // listen (for command)
  listen(sockfdc,8192);

  // wait for command
  int clilen = sizeof(cli_addr);  // size of address of client
  int newsockfd;
  while(1){
    if((newsockfd = accept(sockfdc, (struct sockaddr *) &(cli_addr), &clilen)) >= 0){
      while(1){
        char* buffer = malloc(BUFSIZE);
        char* str = malloc(BUFSIZE);
        bzero(str,BUFSIZE);
        bzero(buffer,BUFSIZE);
        if(read(newsockfd,buffer,BUFSIZE-1) < 0){
          perror("ERROR reading from socket");
        }
        //printf("Here is the message: %s\n",buffer);
        char* token = buffer;
        printf("token is %s\n", token);
        if(token == NULL)
          continue;
        if(!strncmp("STATS",token,5)){
          // calculate time elapsed
          ftime(&end);
          int total_seconds_elapsed = end.time - start.time;
          char* time_str = malloc(100);
          int hours_elapsed = total_seconds_elapsed / 3600;
          int minutes_elapsed = (total_seconds_elapsed % 3600) / 60;
          int seconds_elapsed = total_seconds_elapsed % 60;
          int mil_elapsed = end.millitm - start.millitm;
          sprintf(time_str, "%02i:%02i:%02i.%02i", hours_elapsed, minutes_elapsed, seconds_elapsed,mil_elapsed);
          printf("%02i:%02i:%02i", hours_elapsed, minutes_elapsed, seconds_elapsed);

          sprintf(str,"Server up for %s, served %d pages, %d bytes\n",time_str, number_of_pages,number_of_bytes);
          if(write(newsockfd,str,BUFSIZE) < 0)
            perror("ERROR writing to socket");
        }
        else if(!strncmp("SEARCH",token,6)){
          if(iscrawling == 1){
            write(newsockfd,"crawling still in progress...\n",30);
          }
          else{
            int pid = fork();
            if(pid < 0){
              perror("fork");
              exit(-1);
            }
            else if(pid == 0){
              char wd[1024];
              if (getcwd(wd, sizeof(wd)) != NULL)
                printf("Current working dir: %s\n", wd);
              printf("0\n");
              execl("../jobExecutor","-d","path.txt","-w","3",token,NULL);
            }
            else{
              int search = open("../search", O_RDONLY);
              char* tot = malloc(10000);
              if(read(search,tot,9999) <= 0);
              if(write(newsockfd,tot,9999) <= 0)
                perror("ERROR writing to socket");
              free(tot);
              close(search);
              remove("search");
            }
          }
        }
        else if(!strncmp("SHUTDOWN",token,8)){
          close(newsockfd);
          exit(0);
        }
        free(buffer);
        free(str);
      }
    }
  }

  return 0;
}

void* make_directory(void* arg)
{
  // socket creation
  struct crawl* ar = arg;

  int sockfd;
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    perror("socket command");

  // get info about the server
  struct hostent *server;             // server info
  server = gethostbyname(ar->host_or_ip);
  if(server == NULL){
    printf("No such host\n");
    perror("ERROR, no such host");
    return NULL;
  }

  // bind
  struct sockaddr_in serv_addr;
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
  serv_addr.sin_port = htons(ar->server_port);

  ItemType item;

  while(1){
    pthread_mutex_lock(&lock);
    while(Empty(urls)){
      canexit++;
      if(canexit == not+(not-1)){
        pthread_mutex_lock(&crawllock);
        iscrawling = 0;
        pthread_mutex_unlock(&crawllock);
      }
      if(canexit >= not){
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
        pthread_exit(NULL);
      }
      pthread_cond_wait(&cond, &lock);
    }
    Remove(urls, &item);

    // put in history queue item only if it doesn't already exist
    if(!Exists(item,urlscheck))
      Insert(item,urlscheck);
    else{
      pthread_cond_signal(&cond);
      pthread_mutex_unlock(&lock);
      continue; // if it already existed ignore and go get another url from queue
    }

    Analyze_Insert(item,sockfd,serv_addr);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
  }
}

int Analyze_Insert(ItemType item, int sockfd, struct sockaddr_in serv_addr)
{
  // prepare request for the server
  //int sockfd;
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    perror("socket command");

  if(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
    perror("ERROR connecting");
    return -1;
  }

  printf("url = %s\n", item.url);
  char* request = malloc(210+strlen(item.url));
  bzero(request,210+strlen(item.url));
  if(sprintf(request,"GET %s HTTP/1.1\r\nUser-­Agent: Mozilla/4.0 (compatible;; MSIE5.01;; Windows NT)\r\nHost: www.tutorialspoint.com\r\nAccept-­Language: en-­us\r\nAccept-­Encoding: gzip, deflate\r\nConnection: Closed\r\n\r\n",item.url) < 0){
    perror("sprintf");
    return -1;
  }

  // make the directory if doesn't exist
  char* temp = malloc(strlen(item.url));
  bzero(temp,strlen(item.url));
  temp[0] = '.';
  int offset = 1; // ignore first "/"
  while(item.url[offset] != '/')
    offset++;
  memcpy(&temp[1],item.url,offset);

  struct stat st = {0};
  if (stat(temp, &st) == -1)  // check if directory exists
    mkdir(temp, 0700);

  free(temp);
  //

  // send request to server
  if(write(sockfd,request,strlen(request)) < 0){
    perror("write to socket");
    return -1;
  }

  // see the length of the file to receive
  char* reply = malloc(158);
  bzero(reply,158);
  if(read(sockfd,reply,157) < 0){
    perror("read failed");
    return -1;
  }
  printf("reply = %s\n", reply);

  // identify size of reply
  char* p = strstr(reply,"Content-Length:");
  p = &p[16];
  int siz = 0;
  sscanf(p,"%d",&siz);
  //printf("siz = %d\n", siz);
  free(reply);

  reply = malloc(siz+1);
  bzero(reply,siz+1);
  if(read(sockfd,reply,siz) < 0){
    perror("read failure");
    return -1;
  }

  // make file and put in queue next
  char cur[23];
  bzero(cur,23);
  cur[0] = '.';
  strcpy(&cur[1],item.url);
  FILE *fp = fopen(cur, "w+");
  printf("cur = %s\n", cur);
  if(fwrite(reply, 1, siz, fp) < 0)
    perror("fwrite");
  else
    fclose(fp);

  free(request);
  char* head = reply;
  char* tail;   //
  char* name;
  int len;

  while((head = strstr(head,"<a href=")) != NULL){
    head += 10;
    if(head[1] == '.')
      head++;
    tail = strstr(head,">");
    len = (long)tail - (long)head;
    name = malloc(len+1);
    bzero(name,len+1);
    strncpy(name,head,len);

    printf("name = %s\n", name);
    item.url = name;
    Insert(item,urls);
  }

  free(reply);

  // update stats
  pthread_mutex_lock(&lock_pages);
  number_of_pages++;
  pthread_mutex_unlock(&lock_pages);
  pthread_mutex_lock(&lock_bytes);
  number_of_bytes+=siz;
  pthread_mutex_unlock(&lock_bytes);

  return 0;
}
