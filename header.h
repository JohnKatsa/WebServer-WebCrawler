#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include <sys/timeb.h>

#include <sys/wait.h>   /* sockets */
#include <sys/types.h>  /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <netdb.h>      /* gethostbyaddr */
#include <ctype.h>      /* toupper */

#define BUFSIZE 1024

struct help {
  struct sockaddr_in cli_addr;
  int newsockfd;
};

void* reply_request(void*);

// QUEUE

typedef struct help ItemType;

typedef struct QueueNodeTag {
   ItemType Item;
   struct QueueNodeTag *Link;
} QueueNode;

typedef struct {
  QueueNode *Front;
  QueueNode *Rear;
} Queue;

void InitializeQueue(Queue *);
int Empty(Queue *);
int Full(Queue *);
void Insert(ItemType, Queue *);
void Remove(Queue *, ItemType *);
