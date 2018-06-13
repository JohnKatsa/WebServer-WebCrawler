#include "headerje.h"

trie* trie_init()
{
  trie* t = malloc(sizeof(trie));
  t->letter = '\0';  // my identifier
  t->next = NULL;
  t->dim = NULL;
  t->array = NULL;
  t->isEnd = 0;
  return t;
}

trie* trie_insert(trie* t, char* word, int id, int pos) // id = file_id, pos = file_line
{
  int size = strlen(word);              //
  //cout << "strlen = " << size << endl;  //
  //cout << pos << endl;                  //

  char temp = word[0];
  trie* root = t;
  trie* p = t;   // initially point at root
  trie* previous = p; // used for next |
  trie* prev; // used for dim --

  while(temp != '\0'){    // for every letter

    // INITIALLY BLANK
    if(p->letter == '\0'){
      p->letter = temp;
      word++;
      temp = word[0];
      if(p->next == NULL){
        p->next = trie_init();
      }
      previous = p;
      p = p->next;
      continue;
    }

    // MANY DIMENSIONAL NODES
    prev = p;
    int flag = 0;
    while(p->dim != NULL){
      if(p->letter == temp){
        break;
      }
      else if(p->letter < temp){
        prev = p;
        p = p->dim;
      }
      else{
        //previous;
        if(prev != p){
          prev->dim = trie_init();   // create new dim
          prev->dim->letter = temp;     // set letter
          prev->dim->dim = p;
          prev->dim->next = trie_init();   // create null next
          p = prev->dim;
          flag = 1;
          break;
        }
        else{
          prev = trie_init();
          prev->letter = temp;
          prev->dim = p;
          prev->next = trie_init();
          p = prev;

          if(p->dim == root) // if(root == previous)?
            root = p;    // change root to new min
          else
            previous->next = prev;  // point to new min if not root!

          flag = 1;
        }
      }
    }


    // not in because of 1 node dimension, check 3 cases
    if(p->letter == temp){
      //p;
    }
    else if(p->letter < temp){
      p->dim = trie_init();
      p->dim->letter = temp;
      p = p->dim;
      p->next = trie_init();
    }
    else if(p->letter > temp && flag == 0){
      //previous;
      trie* node;
      node = trie_init();
      node->dim = p;
      node->letter = temp;
      if(previous->next == p) previous->next = node;
      if(prev != p) {prev->dim = node; node->dim = p;}  // in case it comes from multidimensional case
      if(node->next == NULL){
        node->next = trie_init();    // make next null node
      }

      //previous->next = node;
      if(root == p){
        root = node;
      }

      word++;
      temp = word[0];
      if(previous != p)
        previous = previous->next; // john-ny john-athan
      else
        previous = node;

      p = node->next;     //
      continue;
    }

    word++;
    temp = word[0];
    if(p->next == NULL){
      p->next = trie_init();
    }
    if(temp != '\0'){
      previous = p;
      p = p->next;
    }
  }

  // ENDING OF WORD. MAKE THE ARRAY OF FREQUENCIES, OR JUST ADD
  if(p->letter == '\0'){
    p = previous;
  }

  p->isEnd = 1;

  if(p->array == NULL){ // WORD FOUND FOR FIRST TIME
    p->array = list_init(id,pos);
  }
  else{ // WORD FOUND IN OTHER IDS
    list* tmp = p->array;

    while(tmp->next != NULL){ // go to last node of list to append
      if(tmp->file_line == pos && tmp->file_id == id){
        tmp->total++;
        return root;
      }
      tmp = tmp->next;
    }
    tmp->next = list_init(id,pos);
  }
  return root;

}

char* trie_search(trie* t, char* word, map* files, int code, char* logfile)
{
  trie* root = t;
  trie* previous = t;
  trie* last = t;		// used to know if word is real and not just a subword
  int offset = 0;
  while(word[offset] != '\0'){
    while(root != NULL){
      if(root->letter == word[offset]){
        break;
      }
      root = root->dim;
    }
    last = root;
    if(root == NULL){
      break;
    }
    offset++;
    previous = root;
    root = root->next;
  }

  // calculate date
  time_t current_time;
  char* c_time_string;
  current_time = time(NULL);
  c_time_string = ctime(&current_time);

  FILE* fptr;

  char* buffer = malloc(1000);

  if(last == NULL){
    if(code == -1){
      fptr = fopen(logfile, "a");

      // save time of query
      c_time_string[strlen(c_time_string)-1] = '\0';   // because it saves \n at the end
      fprintf(fptr, "-- Time of query arrival : %s   ", c_time_string);
      fprintf(fptr,"Query type : Search   String : %s : ", word);
      fprintf(fptr,"pathname1 : No such string found\n");
      fclose(fptr);
      char* answer = malloc(15);
      answer = "Not found";
      return answer;
    }
    else{
      char* answer = malloc(15);
      answer = "Not found";
      return answer;
    }
  }

  list* l = previous->array;
  if(l == NULL){
    if(code == -1){
      fptr = fopen(logfile, "a");

      // save time of query
      c_time_string[strlen(c_time_string)-1] = '\0';   // because it saves \n at the end
      fprintf(fptr, "-- Time of query arrival : %s   ", c_time_string);
      fprintf(fptr,"Query type : Search   String : %s : ", word);
      fprintf(fptr,"pathname1 : No such string found\n");
      fclose(fptr);
      char* answer = malloc(15);
      answer = "Not found";
      return answer;
    }
  }

  if(code != -1){    // max/mincount call
    return (char*)l;
  }
  else{
    fptr = fopen(logfile, "a");
    int i = 1;
    char* buffer1 = malloc(10*BUFSZ);
    char* buffer2 = malloc(BUFSZ);
    c_time_string[strlen(c_time_string)-1] = '\0';
      //printf("word %s found in the below files:\n", word);
      sprintf(buffer1,"word %s found in the below files:\n", word);
      while(l != NULL){
        //printf("file %s in line %d\n", files[l->file_id].filename, l->file_line);
        sprintf(buffer2,"file %s in line %d: %s\n", files[l->file_id].filename, l->file_line, files[l->file_id].lines[l->file_line]);
        fprintf(fptr, "-- Time of query arrival : %s   ", c_time_string);
        fprintf(fptr,"Query type : Search   String : %s : ", word);
        fprintf(fptr,"pathname%d : %s \n", i, files[l->file_id].filename);
        strcat(buffer1,buffer2);
        l = l->next;
        i++;
      }
    fclose(fptr);
    return buffer1;
  }

}
