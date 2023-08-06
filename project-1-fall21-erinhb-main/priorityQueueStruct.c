#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node{
	int value;
	struct Node *next;
}Node;

typedef struct{
	Node *head;
	Node *tail;
	int size;
}Queue;

void init_queue(Queue *q){

	q->head = NULL;
	q->tail = NULL;
	q->size = 0;

}

int size(Queue *q){
	return (q->size);
}

bool isEmpty(Queue *q){
	return ((q->size) == 0);
}

bool push(Queue *q, int value){

	//create new node
	Node *newNode = malloc(sizeof(Node));
	if(newNode == NULL){
		return false;
	}
	newNode->value = value;
	newNode->next = NULL;

	//if there is a tail, connect tail to new node
	if (q->tail != NULL){
		q->tail->next = newNode;
	}

	//if no head, then make it the head
	q->tail = newNode;
	if(q->head == NULL){
		q->head = newNode;
	}

	q->size++;
	return true;

}

int pop(Queue *q){

	//make sure there are items in the queue
	//if no items, then nothing to pop
	if(q->head == NULL){
		return -1;
	}

	//grab the current head
	//make the queue head equal to the next node
	//return the first head
	Node *temp = q->head;

	int result = temp->value;
	q->head = q->head->next;

	if(q->head == NULL){
		q->tail = NULL;
	}

	q->size--;
	return result;

}

int main(){

	Queue q1;
	init_queue(&q1);

	push(&q1, 1);
	push(&q1, 2);
	push(&q1, 3);
	int i = size(&q1);
	printf("size: %d\n", i);
	
	int j;
	while((j = pop(&q1)) != -1){
		printf("j = %d", j);
	}
	
}
