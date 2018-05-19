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
