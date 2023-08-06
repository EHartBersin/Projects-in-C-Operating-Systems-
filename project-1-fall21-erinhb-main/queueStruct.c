#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
	int *values;
	int head, tail, num_entries, size;
}Queue;

void init_queue(Queue *q, int max_size){

	q->size = max_size;
	q->values = malloc(sizeof(int) * q->size);
	q->num_entries = 0;
	q->head = 0;
	q->tail = 0;

}

bool isEmpty(Queue *q){
	return(q->num_entries==0);
}

bool isFull(Queue *q){
	return(q->num_entries == q->size);
}

int size(Queue *q){
	return (q->size);
}

void enqueue(Queue *q, int value){

	//if the queue is full, create a new queue
	//put the items into that new queue with larger size

	if(isFull(q)){
		q->values = realloc(q->values, (q->num_entries+1));
	}

	q->values[q->tail] = value;
	q->num_entries++;
	q->tail = (q->tail + 1) % q->size;

}

int dequeue(Queue *q){

	int result;

	if(isEmpty(q)){
		return -1;
	}

	result = q->values[q->head];
	q->head = (q->head + 1) % q->size;
	q->num_entries--;

	return result;

}


int main(){

	Queue q1;
	init_queue(&q1, 5);
	
	enqueue(&q1, 1);
	enqueue(&q1, 2);
	enqueue(&q1, 3);
	enqueue(&q1, 4);
	enqueue(&q1, 5);

	int i;
	while((i = dequeue(&q1)) != -1){
		printf("i = %d\n", i);
	}

}

