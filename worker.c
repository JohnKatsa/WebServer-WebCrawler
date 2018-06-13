#include "headerje.h"

int total_chars = 0;
int total_words = 0;
int total_lines = 0;

int worker(char* namew, char* namej, int pos, int load, char** p)
{
  printf("worker successfully called\n");
  printf("load = %d, pos = %d\n", load, pos);

  // create log file
  char* logfile = malloc(sizeof(strlen(namew)-2)+strlen("log/"));
  strcpy(logfile,"log/");
  strncat(logfile,namew,strlen(namew)-2);
  int fd = open(logfile, O_RDWR | O_CREAT, 0644);

  // make trie while opening each directory and every of its file
  trie* t = trie_init();

  DIR *dp = NULL;
  struct dirent *dptr = NULL;
  FILE *input_file;
  char c,cwd[512]; //created new character array
  int thesis;


  map* files = malloc(sizeof(map));      // array of files
  files[0].lines = malloc(sizeof(char*));
  files[0].total = malloc(sizeof(int));
  int nfiles = 0;                        // counter of files

  for(int j = 0; j < pos; j++)
    p++;

  for(int i = 0; i < load; i++){  // for every folder
    printf("Started processing %s folder\n", p[i]);
    thesis = strlen(p[i]);        // hold position of folder to iterate files

    if (NULL == (dp = opendir(p[i]))){
      printf("Cannot open the given directory %s", p[i]);
      continue;
    }
    else{
      while((dptr = readdir(dp))!=NULL){  // for every file in folder
        printf("In file %s\n", dptr->d_name);
        if(!strcmp (dptr->d_name, ".")){
          continue;
        }
        if(!strcmp (dptr->d_name, "..")){
          continue;
        }
        if(dptr->d_name[0]=='.')  //Files never begin with '.'
          continue;

        // reinitialization
        p[i][thesis] = '\0';

        strcat(p[i],"/");
        strcat(p[i],dptr->d_name);

        input_file = fopen(p[i], "r");  // file to read content

        if(input_file == NULL){
          perror("Cannot open file or no file exists\n");
          fclose(input_file);
          continue;
        }

        // open file and save its content in array

        size_t len = 0;
        char* buffer = NULL;
        ssize_t nread = 0;
        int ik = 0;
        files[nfiles].numoflines = 0;
        while((nread = getline(&buffer, &len, input_file)) != -1){
          // assignments here

          files[nfiles].lines[ik] = malloc(len);    //
          strcpy(files[nfiles].lines[ik], buffer);  // save line
          files[nfiles].total[ik] = len;            // save total amount of chars in line

          //tokenize to insert in trie
          char* token = NULL;

          token = strtok(buffer," \t\n");  //token keeps every word
          while(token != NULL){

            // add to statistics
            total_chars += strlen(token);
            total_words++;

            t = trie_insert(t,token,nfiles,ik); // 2 ints are file_id and number of line in file
            token = strtok(NULL," \t\n");
          }


          ik++;
          len = 0;
          buffer = NULL;

          files[nfiles].lines = realloc(files[nfiles].lines, (ik+1)*sizeof(char*));
          files[nfiles].total = realloc(files[nfiles].total, (ik+1)*sizeof(int));
          files[nfiles].numoflines++;

          // add to statistics
          total_lines++;
        }

        // set file name
        files[nfiles].filename = malloc(10000);
        strcpy(files[nfiles].filename, p[i]);

        // continue iteration
        nfiles++;
        files = realloc(files,(nfiles+1)*sizeof(map));  // because nfiles is an index not a counter, starts from 0
        files[nfiles].lines = malloc(sizeof(char*));
        files[nfiles].total = malloc(sizeof(int));

      }
      closedir(dp);
    }
  }

  int fdj = open(namej, O_RDONLY);  // here reads worker
  int fdw = open(namew, O_WRONLY);  // here writes worker


  char* command;// command
  command = malloc(10*BUFSZ);

    // read command
    memset(command,0,BUFSZ);
    read(fdj,command,BUFSZ);
    printf("just read %s", command);

    // execute command
    // SEARCH
    if(strncmp(command,"SEARCH",6) == 0){
      int counter_of_q = get_counter_of_q(command);

      char** words = get_queries(command, counter_of_q);

      char* final = malloc(10*BUFSZ);
      for(int i = 0; i < counter_of_q; i++){
        char* temp = trie_search(t,words[i],files,-1,logfile);
        if(temp != NULL)
          strcat(final,temp);
      }

      if(final[0] != '\0'){
        write(fdw,final,BUFSZ);
        //printf("return final %s\n", final);
      }
      else{
        strcpy(final,"Not found");
        write(fdw,final,BUFSZ);
      }
      free(final);
    }

    else{
      printf("failed\n");
      exit(0);
    }

    printf("ended query\n");

}

int get_counter_of_q(char* buf)
{
  // find counter of q
  int offset = 0;
  int counter_of_q = 0;
  while(buf[offset] != '\n'){
    if(buf[offset] == ' '){
      counter_of_q++;
    }
    offset++;
  }

  return counter_of_q;
}

char** get_queries(char* buf2, int counter_of_q)   // doesnt make new space... it manipulates buf
{

  char** words = malloc(sizeof(char*)*counter_of_q);
  char* buf = malloc(sizeof(char)*BUFSZ);
  memcpy(buf,buf2,BUFSZ);

  int i = 0;
  char* token = strtok(buf," \t");  //token = search... we dont want it

  words[i] = strtok(NULL, " \t\n");

  while(words[i] != NULL){
    i++;
    words[i] = strtok(NULL," \t\n");
  }

  return words;
}
