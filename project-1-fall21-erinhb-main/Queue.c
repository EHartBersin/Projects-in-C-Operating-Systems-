#include <stdio.h>
#include <stdlib.h>

typedef struct Event{

	int jobID;
	int time;
	char * type;

}Event;

struct Event createEvent(int j, int ti, char* ty){

	Event* e = (Event*)malloc(sizeof(Event));
	Event event = *e;
	event.jobID = j;
	event.time = ti;
	event.type = ty;
	return event;

}

typedef struct Node{

	struct Event event;
	struct Node* next;

}Node;

struct Node* createNode(Event event){

	Node* temp = (Node*)malloc(sizeof(Node));
	temp->event = event;
	temp->next = NULL;
	return temp;

}

typedef struct Queue{

	struct Node *head;
	struct Node *tail;
	int size;

}Queue;

struct Queue* createQueue(){

	struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
	q->head = NULL;
	q->tail = NULL;
	return q;

}

void enQueue(struct Queue* q, struct Event event){

	Node* temp = createNode(event);
	
	if(q->tail == NULL){
		q->head = q->tail = temp;
		q->size++;
		return;
	}

	q->tail->next = temp;
	q->tail = temp;
	q->size++;

}

void deQueue(struct Queue* q){

	if(q->head == NULL){
		return;
	}

	struct Node* temp = q->head;
	q->head = q->head->next;

	if(q->head == NULL){
		q->tail = NULL;
	}
	
	q->size--;
	free (temp);

}

struct Event peek(struct Queue* q){
	return q->head->event;
}

int isEmpty(struct Queue* q){
	return(q->head) == NULL;
}

int size(struct Queue* q){
	return q->size;
}

int main(){

	Queue* q = createQueue();
	Event e1 = createEvent(1,2,"stuff");
	Event e2 = createEvent(2,3,"stuff");
	Event e3 = createEvent(3,1,"stuff");
	enQueue(q, e3);
	enQueue(q, e1);
	enQueue(q, e2);
	printf("size: %d \n", size(q));

	
	while(!isEmpty(q)){
		Event ee = peek(q);
		deQueue(q);
		printf("%d %d %s \n", ee.jobID, ee.time, ee.type);
	}

	return(0);

}







