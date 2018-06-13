#include "header.h"

void InitializeQueue(Queue *Q)
{
    Q->Front=NULL;
    Q->Rear=NULL;
}

int Empty(Queue *Q)
{
   return(Q->Front==NULL);
}

int Full(Queue *Q)
{
   return(0);
}

void Insert(ItemType R, Queue *Q)
{
   QueueNode *Temp;

   Temp=(QueueNode *)malloc(sizeof(QueueNode));

   if (Temp==NULL){
      printf("System storage is exhausted");
   } else {
      Temp->Item=R;
      Temp->Link=NULL;
      if (Q->Rear==NULL){
         Q->Front=Temp;
         Q->Rear=Temp;
      } else {
         Q->Rear->Link=Temp;
         Q->Rear=Temp;
      }
   }
}


void Remove(Queue *Q, ItemType *F)
{
   QueueNode *Temp;

   if (Q->Front==NULL){
      printf("attempt to remove item from an empty queue");
   } else {
      *F=Q->Front->Item;
      Temp=Q->Front;
      Q->Front=Temp->Link;
      free(Temp);
      if (Q->Front==NULL) Q->Rear=NULL;
   }
}

int Exists(ItemType R, Queue *Q)
{
  // empty queue
  if (Q->Front==NULL){
    printf("attempt to check for item in an empty queue");
  }
  else{

    // if 1 item in queue only
    if(Q->Front == Q->Rear)
      return (!strcmp(Q->Front->Item.url,R.url));

    // if more than 1 items in queue
    QueueNode* Temp = Q->Front;
    while(Temp != Q->Rear){
      if(!strcmp(Temp->Item.url,R.url)) // found again in history queue
        return 1; // it exists
      Temp = Temp->Link;
    }
  }

  return 0;   // doesn't exist
}
