#include "headerje.h"
#include "header.h"

int main(int argc, char** argv)
{
  chdir("..");
  chdir("./project3");
  printf("JOBEXECUTOR EXECUTED!!\n");
  // check and read arguments
  char* fn;
  int w;
  int check = check_arguments(argc, argv, &fn, &w); // fn holds filename, w holds number of workers
  if(check == -1) printf("Error\n");
  fn = argv[1];
  w = 2;
  // get the folder paths
  int counter;
  char** paths = get_paths(fn,&counter);

	mkdir("log", 0777);

  // make fifos and start workers
  make_fifos(w,paths,counter,argv[4]);

  exit(0);
}

// convert string to integer
int sti(char* str)
{
	int dec = 0, j, len;
	len = strlen(str);
	for(int i = 7; i < len-2; i++){
		dec = dec * 10 + ( str[i] - '0' );
	}
	return dec;
}

// convert integer to string
void its(int number, char* str)
{
  char s[3];

  if(number < 10){
    sprintf(s,"%d",number);
    memcpy(str+9,s,1);
  }
  else if(number < 100){
    sprintf(s,"%d",number);
    memcpy(str+8,s,2);
  }
  else if(number < 1000){
    sprintf(s,"%d",number);
    memcpy(str+7,s,3);
  }
}

int check_arguments(int argc, char** argv, char** fn, int* wo)
{
  int i = 1;
  char* filename;
  int w;
  int flagw = 0, flagn = 0;  // flag workers, flag name

  if(argc == 1){
    printf("Nor file or workers given\n");
    return -1;    // error
  }

  for(i = 1; i < argc; i++){
    if(strcmp(argv[i],"-w") == 0){
      if(argv[i+1] != NULL) w = atoi(argv[i+1]);
      flagw = 1;
    }
    if(strcmp(argv[i],"-d") == 0){
      if(argv[i+1] != NULL) filename = argv[i+1];
      flagn = 1;
    }
  }
  if(flagw == 0) w = 5; // default workers
  if(flagn == 0){ printf("No filename given\n"); return -1; }

  printf("filename = %s\n", filename);
  printf("workers = %d\n", w);

  // pass by reference vars
  *fn = filename;
  *wo = w;
}

char** get_paths(char* filename, int* counter)
{
  // open file
  FILE *fptr;
  fptr = fopen("./path.txt", "r");
  if(fptr == NULL) return NULL;
  ////////////

  // count lines
  int nlcounter = 0;
  char c;
  while((c = getc(fptr)) != EOF){
    if(c == '\n')
      nlcounter++;
  }
  *counter = nlcounter;
  printf("nlcounter = %d\n", nlcounter);
  //////////////

  // go to beginning of file and allocate space
  rewind(fptr);
  char** paths;
  paths = malloc(nlcounter*sizeof(char*));

  // take lines one by one
  size_t len = 0;
  char* buffer;
  ssize_t nread;
  int i = 0;
  while((nread = getline(&buffer, &len, fptr)) != -1){
    paths[i] = malloc(len);
    strcpy(paths[i],buffer);
    paths[i][strlen(paths[i])-1] = '\0';    // take away the '\n' char at the end
    i++;
    len = 0;
  }

  return paths;
  ////////////////////////
}

void make_fifos(int w, char** paths, int counter, char* token)
{
  // make fifos (named pipes)
  // start workers
  // give them supervising paths

	// make poll struct
	struct pollfd* wfds = malloc(w*sizeof(struct pollfd));
	struct pollfd* jfds = malloc(w*sizeof(struct pollfd));

  int pos = 0;  // position in paths
  int load = 0; // how many can this worker serve

  char namew[] = "worker_000_w";   // worker writes       initialization
  char namej[] = "worker_000_j";   // jobExecuter writes  initialization
  int temp = 0;
  for(int i = 0; i < w; i++){
    if(mkfifo(namew,0666) == -1) perror("Can't make fifo");
    //printf("%s successfully created!\n", namew);
    if(mkfifo(namej,0666) == -1) perror("Can't make fifo");
    //printf("%s successfully created!\n", namej);

    // create worker and assign its pipe
    int pid = fork();

    pos += load;
    load = (counter-pos)/(w-i);

    if(pid < 0){
      perror("Unable to fork");
      exit(-1);
    }
    else if(pid == 0){
      // separate what path this worker has

      worker(namew,namej,pos,load,paths);
      exit(0);
    }

    // open every needed pipe for job jobExecuter to read and write results/commands
    jfds[i].fd = open(namej, O_WRONLY);
    jfds[i].events = POLLOUT;

    wfds[i].fd = open(namew, O_RDONLY);
    wfds[i].events = POLLIN;

    if(i != w-1){
      temp = sti(namew);
      temp++;
      its(temp,namew);
      temp = sti(namej);
      temp++;
      its(temp,namej);
    }
  }

  // server - jobExecuter

	//SELECTION
  char* buffer;
  size_t size;
	char** results = malloc(w*sizeof(char*));
	for(int i = 0; i < w; i++){
		results[i] = malloc(BUFSZ);
	}

  size = 0;
  buffer = token;

  printf("-------------------------------------------------------------------------------\n");

	// send command to every worker
  for(int i = 0; i < w; i++){
		while(write(jfds[i].fd,buffer,100) <= 0);
		  //printf("error\n");
		printf("just wrote %s", buffer);
	}

		// for every pipe wait for result

	int flag = 0;
	int pl;

	for(int i = 0; i < w; i++){
		read(wfds[i].fd,results[i],BUFSZ);
		printf("fds %s\n", results[i]);
	}


  int d = 0;
  for(int i = 0; i < w; i++)
    d += strlen(results[i]);

  char* tot = malloc(d);
  for(int i = 0; i < w; i++)
    strcpy(tot,results[i]);

  int search = open("search", O_WRONLY);
  if(write(search,tot,d-1) < 0){
    perror("write");
    exit(-1);
  }
  close(search);
  free(tot);
	for(int i = 0; i < w; i++)
		memset(results[i],0,BUFSZ);
  printf("-------------------------------------------------------------------------------\n");

  remove("worker_000_j");
  remove("worker_000_w");
  remove("worker_001_j");
  remove("worker_001_w");

	exit(0);

}
