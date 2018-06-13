#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <time.h>

#include <dirent.h>

#include <signal.h>

#define BUFSZ 65000

//  jobExecutor.c

int sti(char*);
void its(int,char*);
int check_arguments(int,char**,char**,int*);
char** get_paths(char*,int*);
void make_fifos(int,char**,int,char*);

//  map.c

typedef struct map {
  char** lines;     // array of lines
  int* total;        // how many chars it has
  int numoflines;   // number of lines
  char* filename;   // name of the file with this content
  char* path;       // path to file
} map;

//  list.c
typedef struct list {
  int total;      // shows how many times this word appears in this file and line
  int file_id;
  int file_line;
  struct list* next;
} list;

list* list_init(int,int);

//  trie.c
typedef struct trie {
  char letter; // contains letter
  struct trie* next; // next dimension letter
  struct trie* dim; // same dimension letter
  int isEnd; // mark ending of word
  list* array;  // [,]
} trie;

trie* trie_init();
trie* trie_insert(trie*,char*,int,int);
char* trie_search(trie*,char*,map*,int,char*);

//   worker.c

int worker(char*,char*,int,int,char**);

char* maxcount(trie*,map*,int,char*,char*);
char* mincount(trie*,map*,int,char*,char*);
char* wc(char*);
int get_counter_of_q(char*);
char** get_queries(char*,int);
