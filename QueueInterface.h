typedef struct help ItemType;

typedef struct QueueNodeTag {
   ItemType Item;
   struct QueueNodeTag *Link;
} QueueNode;

typedef struct {
  QueueNode *Front;
  QueueNode *Rear;
} Queue;

void InitializeQueue(Queue *Q);
int Empty(Queue *);
int Full(Queue *);
void Insert(ItemType R, Queue *Q);
void Remove(Queue *Q, ItemType *F);
