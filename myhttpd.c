#include "header.h"

// threads
pthread_t* tid;

// queue
Queue* requests;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// statistics
pthread_mutex_t lock_time = PTHREAD_MUTEX_INITIALIZER;
int running_time = 0;

pthread_mutex_t lock_pages = PTHREAD_MUTEX_INITIALIZER;
int number_of_pages = 0;

pthread_mutex_t lock_bytes = PTHREAD_MUTEX_INITIALIZER;
int number_of_bytes = 0;

char* root_dir;

int main(int argc, char** argv)
{
  int opt;
  int serving_port, command_port, num_of_threads;

  // for stats -> time
  struct timeb start, end;
  int diff;
  ftime(&start);

  requests = malloc(sizeof(Queue));
  InitializeQueue(requests);

  while((opt = getopt(argc,argv,"p:c:t:d:")) != -1){
    switch (opt)
    {
      case 'p':
        printf("p: %s\n", optarg);
        serving_port = atoi(optarg);
        break;
      case 'c':
        printf("c: %s\n", optarg);
        command_port = atoi(optarg);
        break;
      case 't':
        printf("t: %s\n", optarg);
        num_of_threads = atoi(optarg);
        break;
      case 'd':
        printf("d: %s\n", optarg);
        root_dir = optarg;
        break;
    }
  }

  // socket
  /* server */
  int sockfdc, sockfds;         // socket of server
  struct sockaddr_in serv_addr, com_addr; // internet address structure

  /* client */
  int newsockfd;
  struct sockaddr_in cli_addr;  // internet address structure

  if((sockfdc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    perror("socket command");
  printf("Listening command to socket %d\n", sockfdc);
  if((sockfds = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    perror("socket serve");
  printf("Listening serve to socket %d\n", sockfds);

  // set buffer to zero
  bzero((char *) &com_addr, sizeof(com_addr));
  bzero((char *) &serv_addr, sizeof(serv_addr));

  com_addr.sin_family = AF_INET;
  com_addr.sin_port = htons(command_port); // convert to internet order
  com_addr.sin_addr.s_addr = INADDR_ANY;   // INADDR_ANY = host IP

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(serving_port); // convert to internet order
  serv_addr.sin_addr.s_addr = INADDR_ANY;   // INADDR_ANY = host IP

  // bind (for input)
  if(bind(sockfdc, (struct sockaddr *) &com_addr,sizeof(com_addr)) < 0)
    perror("bind");

  // bind (for output)
  if(bind(sockfds, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    perror("bind");

  // listen (for input)
  listen(sockfdc,8192);   // 5 = the number of connections that can
                      // be waiting while the process is handling a particular connection
  // listen (for output)
  listen(sockfds,8192);

  // thread pool
  tid = malloc(num_of_threads*sizeof(pthread_t));
  for(int i = 0; i < num_of_threads; i++)
    if(!pthread_create(&(tid[i]), NULL, reply_request, NULL))
      perror("thread create");

  // accept
  struct pollfd poll_list[2];
  poll_list[0].fd = sockfds;
  poll_list[1].fd = sockfdc;
  poll_list[0].events = POLLIN;
  poll_list[1].events = POLLIN;

  while(1){
    int ret= poll(poll_list,2,-1);
    if(ret < 0){
      perror("poll");
      exit(-1);
    }
    if(poll_list[0].revents & POLLIN){
      int clilen = sizeof(cli_addr);  // size of address of client
      if((newsockfd = accept(sockfds, (struct sockaddr *) &(cli_addr), &clilen)) >= 0){
        ItemType h;
        h.cli_addr = cli_addr;
        h.newsockfd = newsockfd;
        printf("newsockfdmain = %d\n", h.newsockfd);

        pthread_mutex_lock(&lock);
        Insert(h, requests);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
        printf("I've been unlocked.\n");
      }
    }
    else if(poll_list[1].revents & POLLIN){

      printf("SHUTDOWN OR STATS CALLED\n");
      int clilen = sizeof(cli_addr);  // size of address of client
      if((newsockfd = accept(sockfdc, (struct sockaddr *) &(cli_addr), &clilen)) >= 0){
        char* buffer = malloc(BUFSIZE);
        char* str = malloc(BUFSIZE);
        bzero(str,BUFSIZE);
        bzero(buffer,BUFSIZE);
        if(read(newsockfd,buffer,BUFSIZE-1) < 0){
          perror("ERROR reading from socket");
        }
        //printf("Here is the message: %s\n",buffer);
        char* token = buffer;
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

          sprintf(str,"Server up for %s, served %d pages, %d bytes\n",time_str,number_of_pages,number_of_bytes);
          if(write(newsockfd,str,BUFSIZE) < 0)
            perror("ERROR writing to socket");
          close(newsockfd);
        }
        else if(!strncmp("SHUTDOWN",token,8)){
          sprintf(str,"\nSHUTTING SERVER DOWN! Byeeee!\n");
          if(write(newsockfd,str,BUFSIZE) < 0)
            perror("ERROR writing to socket");
          close(newsockfd);
          free(requests);
          exit(0);
        }
        free(buffer);
        free(str);
      }
    }
  }

  close(sockfdc);
  close(sockfds);
  return 0;
}

void* reply_request(void* arg)
{
  ItemType* h = malloc(sizeof(ItemType));
  char* buffer = malloc(BUFSIZE);

  time_t current_time;
  char* c_time_string;
  current_time = time(NULL);
  c_time_string = ctime(&current_time);
  c_time_string[strlen(c_time_string)-1] = '\0';

  while(1){
    pthread_mutex_lock(&lock);
    if(Empty(requests))
      pthread_cond_wait(&cond, &lock);
    Remove(requests, h);   // pop from queue
    printf("newsockfdrem = %d\n", h->newsockfd);
    pthread_mutex_unlock(&lock);

    int newsockfd;  // socket of client

    bzero(buffer,BUFSIZE);
    printf("Established Connection!\n");
    printf("newsockfd = %d\n", h->newsockfd);

    if(read(h->newsockfd,buffer,BUFSIZE-1) < 0){
      perror("ERROR reading from socket");
      return NULL;
    }
    printf("Here is the message: %s\n",buffer);

    char* token = strtok(buffer," ");  //token

    token = strtok(NULL, " ");
    int path_size = strlen(token)+1;
    int root_size = strlen(root_dir)+1;
    char* token2 = malloc(path_size+root_size);
    strcpy(token2,root_dir);
    strcat(token2,token);
    printf("path = %s\n", token2); // token contains path to file

    // no permissions
    if(access(token2, R_OK) != 0){
      if(write(h->newsockfd,"HTTP/1.1 403 Forbidden\r\nDate: Mon, 27 May 2018 12:28:53 GMT\r\nServer: myhttpd/1.0.0 (Ubuntu64)\r\nContent-Length: 124\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n<html>Sorry dude, couldn't find this file.</html>",BUFSIZE-1) < 0)
        perror("ERROR writing to socket");
      continue;
    }

    int fd = open(token2, O_RDONLY);
    free(token2);
    if(fd <= 0){
      if(write(h->newsockfd,"HTTP/1.1 404 Not Found\r\nDate: Mon, 27 May 2018 12:28:53 GMT\r\nServer: myhttpd/1.0.0 (Ubuntu64)\r\nContent-Length: 124\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n<html>Sorry dude, couldn't find this file.</html>",BUFSIZE-1) < 0)
        perror("ERROR writing to socket");
      continue;
    }

    int size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
    char* str = malloc(size+1);
    bzero(str,size);
    if(fd){
      read(fd,str,size);
      close(fd);
    }

    char* buffer2 = malloc(size+1+200);
    sprintf(buffer2,"HTTP/1.1 200 OK\r\nDate: %s GMT\r\nServer: myhttpd/1.0.0 (Ubuntu64)\r\nContent-Length: %d\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n%s",c_time_string,size,str);

    free(str);

    if(write(h->newsockfd,buffer2,strlen(buffer2)) < 0)
      perror("ERROR writing to socket");

    free(buffer2);

    // update stats
    pthread_mutex_lock(&lock_pages);
    number_of_pages++;
    pthread_mutex_unlock(&lock_pages);
    pthread_mutex_lock(&lock_bytes);
    number_of_bytes+=size;
    pthread_mutex_unlock(&lock_bytes);


    close(h->newsockfd);
  }
  pthread_exit(NULL);
}
