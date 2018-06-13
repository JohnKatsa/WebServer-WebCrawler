#include "headerje.h"

list* list_init(int id, int pos)
{
  list* l = malloc(sizeof(list));
  l->total = 1;
  l->file_id = id;
  l->file_line = pos;
  l->next = NULL;
  return l;
}
