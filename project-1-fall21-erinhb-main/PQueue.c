#include <stdio.h>
#include <stdlib.h>

typedef struct Event{

	int jobID;
	int time;
	char * type;

}Event;

struct Event* createEvent(int j, int ti, char* ty){

	Event* event = (Event*)malloc(sizeof(Event));
	event->jobID = j;
	event->time = ti;
	event->type = ty;
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

typedef struct PQueue{

	struct Node* head;
	struct Node* tail;
	int size;

}PQueue;

struct PQueue* createPQueue(){

	struct PQueue* q = (struct PQueue*)malloc(sizeof(struct PQueue));
	q->head = NULL;
	q->tail = NULL;
	return q;

}

//returns the value at head
struct Node* peek(struct PQueue *q){
	return q->head;
}

void push(struct PQueue* q, struct Event event){

	Node* temp = createNode(event);

	if(q->tail == NULL){
		q->head = q->tail = temp;
		q->size++;
		return;
	}

	Node* start = q->head;
	int p = temp->event.time;
	//put the node in the list according to prio
	//if the head is greater than new event's prio
	//make it the head
	//else, go through queue and put it into place
	if((start->event.time) > p){
		temp->next = q->head;
		q->head = temp;
	}else{
	
		while((start->next != NULL) && (start->next->event.time < p)){
			start = start->next;
		}
		temp->next = start->next;
		start->next = temp;
	}
	q->size++;

}

void pop(struct PQueue* q){

	q->head = q->head->next;
	q->size--;

}

int isEmpty(struct PQueue* q){
	return (q->head) == NULL;
}

int size(struct PQueue* q){
	return q->size;
}

int main(){

	return(0);

}




