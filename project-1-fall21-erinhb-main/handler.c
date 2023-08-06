#include <stdio.h>
#include <stdlib.h>


//variables needed from config file
int SEED;
int INIT_TIME;
int FIN_TIME;
int ARRIVE_MIN;
int ARRIVE_MAX;
int QUIT_PROB;
int NETWORK_PROB;
int CPU_MIN;
int CPU_MAX;
int DISK1_MIN;
int DISK1_MAX;
int DISK2_MIN;
int DISK2_MAX;
int NETWORK_MIN;
int NETWORK_MAX;


//time for entire program
int globalTime = 0;


//setting to idle = 0
//busy = 1
int cpu_status = 0;
int disk1_status = 0;
int disk2_status = 0;
int network_status = 0;


//possible event types
#define ARRIVAL 1
#define CPU_ARRIVAL 2
#define CPU_FINISH 3
#define DISK_ARRIVAL 4
#define DISK1_ARRIVAL 5
#define DISK2_ARRIVAL 6
#define	DISK1_FINISH 7
#define DISK2_FINISH 8
#define NETWORK_ARRIVAL 9
#define NETWORK_FINISH 10
#define FINISHED 11


//functions
int randNum(int, int);
int getProb(int QUIT_PROB, int NET_PROB);
void readFile();


//STRUCTURES:
//structures Event and Node for the Priority Queue and Device Queue
typedef struct Event{
	int jobID;
	int time;
	int type;
}Event;

struct Event createEvent(int j, int ti, int ty){
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

//Priority Queue and its functions
typedef struct PQueue{
	struct Node* head;
	struct Node* tail;
	int size;
}PQueue;

struct PQueue* createPQueue(){
	struct PQueue* q = (struct PQueue*)malloc(sizeof(struct PQueue));
	q->head = NULL;
	q->tail = NULL;
	q->size = 0;
	return q;
}

struct Event PQpeek(struct PQueue *q){
	return q->head->event;
}

void push(struct PQueue* q, struct Event event){
	Node* temp = createNode(event);
	
	if (q->tail == NULL){
		q->head = q->tail = temp;
		q->size++;
		return;
	}
	
	Node* start = q->head;
	int p = temp->event.time;
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

int isPQEmpty(struct PQueue* q){
	return (q->head) == NULL;
}

int PQsize(struct PQueue* q){
	return q->size;
}

typedef struct Queue{
	struct Node *head;
	struct Node *tail;
	int size;
}Queue;

struct Queue* createQueue(){
	struct Queue *q = (struct Queue*)malloc(sizeof(struct Queue));
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
	free(temp);

}

struct Event Qpeek(struct Queue* q){
	return q->head->event;
}

int isQEmpty(struct Queue* q){
	return (q->head) == NULL;
}

int Qsize(struct Queue* q){
	return q->size;
}


//Handler functions
void process_Arrival(Event event, PQueue* pq, Queue* cpuQ);
void process_CPU (Event event, PQueue* pq, Queue* cpuQ);
void process_DISKARRIVAL (Event event, Queue* disk1Q, Queue* disk2Q);
void process_DISK (Event event, PQueue* pq, Queue* disk1Q, Queue* disk2Q, Queue* cpuQ);
void process_NETWORK (Event event, PQueue* pq, Queue* networkQ, Queue* cpuQ);


//START OF PROGRAM AND FUNCTIONS
//read the config file and set values equal to our constant variables
void readFile(){

	//open file and check if it opened
	FILE *file = fopen("config.txt", "r");
	if(file == NULL){
		printf("readFile() could not open config.txt file \n");
		exit(1);
	}

	int* temps = malloc(15 * sizeof(int));
	char name[15][15];

	int count = 0;
	while(fscanf(file, "%s %d\n", name[count], &temps[count]) != EOF){
		count++;
	}

	//getting the values for variables from temps
	SEED = *(temps);
	INIT_TIME = *(temps+1);
	FIN_TIME = *(temps+2);
	ARRIVE_MIN = *(temps+3);
	ARRIVE_MAX = *(temps+4);
	QUIT_PROB = *(temps+5);
	NETWORK_PROB = *(temps+6);
	CPU_MIN = *(temps+7);
	CPU_MAX = *(temps+8);
	DISK1_MIN = *(temps+9);
	DISK1_MAX =*(temps+10);
	DISK2_MIN = *(temps+11);
	DISK2_MAX = *(temps+12);
	NETWORK_MIN = *(temps+13);
	NETWORK_MAX = *(temps+14);

	//printf("%d \n", FIN_TIME);

	fclose(file);

}


//returns a random number from the given interval of ints low to high
int randNum(int low, int high){
	return rand() % (high - low + 1) + low;
}


//sends a number to where job goes next
//returns 0 for quit
//1 for network
//2 for disk
int getProb(int QUIT_PROB, int NET_PROB){
	
	//get a random number
	int prob = randNum(1, 100);
	
	//based on that number, return where job goes
	if(prob < QUIT_PROB){
		return 0;	//in bounds of quit prob, job will quit
	}else if((QUIT_PROB < prob) && prob < (QUIT_PROB + NET_PROB)){
		return 1;	//in bounds of quit prob and netprob + quitprob, go to network
	}else{
		return 2;	//anything else, goes to disk
	}

}

//MAIN FUNCTION
//call readFile() to get variable values
//Initializes queues and prio queue
//starts the priority queue while loop
//switch case for each possible type of event
int main(){

	//read through file to get our variables
	readFile();
	
	//initalize rand
	srand(SEED);

	//open the log file
	FILE *logFile = fopen("log.txt", "w");
	if (logFile == NULL){
		printf("Handler.c could not open log.txt file \n");
		exit(1);
	}
	fclose(logFile);
	
	
	//initalize the priority queue and the device queues
	PQueue* pq = createPQueue();

	Queue* cpuQ = createQueue();
	Queue* disk1Q = createQueue();
	Queue* disk2Q = createQueue();
	Queue* networkQ = createQueue();

	//create the first job and put it into the queue
	Event e1 = createEvent(1, INIT_TIME, ARRIVAL);
	push(pq, e1);
	
	Event e2 = createEvent(2, FIN_TIME, FINISHED);
	push(pq, e2);

	//while loop to go through the priority queue until it ends or time runs out
	while((PQsize(pq) != 0) && (globalTime < FIN_TIME)){
		Event curr = PQpeek(pq);
		pop(pq);
		switch(curr.type){
			case ARRIVAL:
				globalTime = curr.time;
				process_CPU(curr, pq, cpuQ);
				break;
			case CPU_ARRIVAL:
				process_CPU(curr, pq, cpuQ);
				break;
			case CPU_FINISH:
				globalTime = curr.time;
				process_CPU(curr, pq, cpuQ);
				break;
			case DISK_ARRIVAL:
				globalTime = curr.time;
				process_DISK(curr, pq, disk1Q, disk2Q, cpuQ);
				break;
			case DISK1_ARRIVAL:
				process_DISK(curr, pq, disk1Q, disk2Q, cpuQ);
				break;
			case DISK2_ARRIVAL:
				process_DISK(curr, pq, disk1Q, disk2Q, cpuQ);
				break;
			case DISK1_FINISH:
				globalTime = curr.time;
				process_DISK(curr, pq, disk1Q, disk2Q, cpuQ);
				break;
			case DISK2_FINISH:
				globalTime = curr.time;
				process_DISK(curr, pq, disk1Q, disk2Q, cpuQ);
				break;
			case NETWORK_ARRIVAL:
				globalTime = curr.time;
				process_NETWORK(curr, pq, networkQ, cpuQ);
				break;
			case NETWORK_FINISH:
				globalTime = curr.time;
				process_NETWORK(curr, pq, networkQ, cpuQ);
				break;
			case FINISHED:
			{
				FILE* logFile = fopen("log.txt", "a");
				fprintf(logFile, "%d event%d FINISHED\n", curr.time, curr.jobID);
				fclose(logFile);
				break;
			}	
		}
	}

	return(0);

}

//CPU HANDLERS
//Job Arrival handler
void process_Arrival(Event event, PQueue* pq, Queue* cpuQ){

	//print to log and add the event to cpu queue
	FILE* logFile = fopen("log.txt", "a");
	fprintf(logFile, "%d event%d CPU_ARRIVAL arrives at CPU\n", event.time, event.jobID);
	fclose(logFile);

	enQueue(cpuQ, event);

	//create a new job to send to priority queue
	int jobID = event.jobID+1;
	int time = (randNum(CPU_MIN, CPU_MAX)) + globalTime;
	int type = ARRIVAL;
	Event next_job = createEvent(jobID, time, type);

	push(pq, next_job);

}

//Cpu handler
void process_CPU(Event event, PQueue* pq, Queue* cpuQ){

	//if new job, go to arrival handler
	if(event.type == ARRIVAL){
		process_Arrival(event, pq, cpuQ);
	}

	//if arriving to cpu
	//print to log
	//put new job into priority queue
	//put job into cpu queue
	if(event.type == CPU_ARRIVAL){
		
		int jobID = event.jobID;
		int time = (randNum(CPU_MIN, CPU_MAX)) + globalTime;
		int type = CPU_FINISH;
		Event eventFin = createEvent(jobID, time, type);
		push(pq, eventFin);
		
	}

	if(event.type == CPU_FINISH){
		
		//decide where it is going next
		//if returns 0, event goes to finish
		int prob = getProb(QUIT_PROB, NETWORK_PROB);
		if(prob == 0){
			int jobID = event.jobID;
			int time = event.time;
			int type = FINISHED;
			Event finish = createEvent(jobID, time, type);
			push(pq, finish);
		}else if(prob == 1){	//going to network 
			int jobID = event.jobID;
			int time = event.time;
			int type = NETWORK_ARRIVAL;
			Event network = createEvent(jobID, time, type);
			push(pq, network);
		}else{			//going to disk
			int jobID = event.jobID;
			int time = event.time;
			int type = DISK_ARRIVAL;
			Event disk = createEvent(jobID, time, type);
			push(pq, disk);
		}
	
		cpu_status = 0;
		
		FILE* logFile = fopen("log.txt", "a");
		fprintf(logFile, "%d event%d CPU_FINISH finishes at CPU\n", event.time, event.jobID);	
		fclose(logFile);

	}
	
	//going through the cpu queue
	if((Qsize(cpuQ) > 0) && (cpu_status == 0)){
		
		//take next event off cpu queue and print to log
		Event current = Qpeek(cpuQ);
		deQueue(cpuQ);

		FILE* logFile = fopen("log.txt", "a");
		fprintf(logFile, "%d event%d CPU_ARRIVAL arrives at CPU\n", current.time, current.jobID);
		fclose(logFile);		

		//create new event cpuArrival and add it to the priority queue
		int jobID = current.jobID;
		int time = current.time;
		int type = CPU_ARRIVAL;
		Event cpuArrival = createEvent(jobID, time, type);
		
		push(pq, cpuArrival);
		cpu_status = 1;
	}

}

//DISK HANDLERS
//prints to log.txt the new event
//then figures out what disk to send the event to
//compares size and status of both disks
void process_DISKARRIVAL(Event event, Queue* disk1Q, Queue* disk2Q){

	//opening log file and writing to it
	FILE* logFile = fopen("log.txt", "a");
	fprintf(logFile, "%d event%d DISK_ARRIVAL arrives at disk\n", event.time, event.jobID);
	fclose(logFile);

	int size1 = Qsize(disk1Q);
	int size2 = Qsize(disk2Q);

	//compare the status of both disks
	//then compare size
	//prefer the disk that is not busy, and the disk with the smallest queue size
	if((disk1_status == 0) &&(disk2_status == 0)){
		if(size1 > size2){
			enQueue(disk2Q, event);
		}else{
			enQueue(disk1Q, event);
		}
	}else{	//one of the disks are busy, check which one
		if((disk1_status == 0) && (disk2_status == 1)){
			enQueue(disk1Q, event);
		}
		if((disk1_status == 1) && (disk2_status == 0)){
			enQueue(disk2Q, event);
		}
	}

}

//processing the disk arrival, deciding what disk to send the event
//then processing that event based off the disk it goes to
void process_DISK(Event event, PQueue* pq, Queue* disk1Q, Queue* disk2Q, Queue* cpuQ){

	//opening log file so its ready to write to
	FILE* logFile = fopen("log.txt","a");

	//switch case to deal with each event type properly
	switch(event.type){
		//if event is first arriving at the disk
		//decide what disk to send it to with process_DISKARRIVAL
		case DISK_ARRIVAL:
			process_DISKARRIVAL(event, disk1Q, disk2Q);
			break;
		case DISK1_ARRIVAL:
		{
			//create new event and push it to disk 1 queue
			int jobID = event.jobID;
			int time = (randNum(DISK1_MIN, DISK1_MAX)) + globalTime;
			int type = DISK1_FINISH;
			Event finish1 = createEvent(jobID, time, type);
			push(pq, finish1);
			break;
		}
		case DISK2_ARRIVAL:
		{
			//same as above, only difference is type is disk2_finish
			int jobID2 = event.jobID;
			int time2 = (randNum(DISK2_MIN, DISK2_MAX)) + globalTime;
			int type2 = DISK2_FINISH;
			Event finish2 = createEvent(jobID2, time2, type2);
			push(pq, finish2);
			break;
		}
		case DISK1_FINISH:
			//print to logFile
			fprintf(logFile, "%d event%d DISK1_FINISH finishes at disk 1\n", event.time, event.jobID);
			enQueue(cpuQ, event);
			disk1_status = 0;
			break;
		case DISK2_FINISH:
			//same as disk1_finish, different status change
			fprintf(logFile, "%d event%d DISK2_FINISH finishes at disk 2\n", event.time, event.jobID);
			enQueue(cpuQ, event);
			break;
	}

	//go through disk1 and disk 2 queue and pop and process event
	if((Qsize(disk1Q) > 0) && (disk1_status == 0)){
		//grap next event in disk 1 queue
		Event current = Qpeek(disk1Q);
		deQueue(disk1Q);

		//printf to log
		fprintf(logFile, "%d event%d DISK1_ARRIVAl arrives at disk 1\n", current.time, current.jobID);
		
		//create new event
		int jobID = current.jobID;
		int time = globalTime;
		int type = DISK1_ARRIVAL;
		Event disk1Arrival = createEvent(jobID, time, type);
		push(pq, disk1Arrival);		
		
		//change status
		disk1_status = 1;
	}
	//do the same for disk 2 queue
	if((Qsize(disk2Q) > 0) && (disk2_status == 0)){
		Event current2 = Qpeek(disk2Q);
		deQueue(disk2Q);

		fprintf(logFile, "%d event%d DISK2_ARRIVAL arrives at disk 2\n", current2.time, current2.jobID);
		
		int jobID = current2.jobID;
		int time = globalTime;
		int type = DISK2_ARRIVAL;
		Event disk2Arrival = createEvent(jobID, time, type);
		push(pq, disk2Arrival);

		disk2_status = 1;
	}
	
	fclose(logFile);
	
}

void process_NETWORK(Event event, PQueue* pq, Queue* networkQ, Queue* cpuQ){

	//opening file to write to
	FILE* logFile = fopen("log.txt", "a");

	//if event type is arrival
	//put new event into prio queue
	if(event.type == NETWORK_ARRIVAL){

		int jobID = event.jobID;
		int time = (randNum(NETWORK_MIN, NETWORK_MAX)) + globalTime;
		int type = NETWORK_FINISH;
		Event networkFin = createEvent(jobID, time, type);
		push(pq, networkFin);
	}
	if(event.type == NETWORK_FINISH){
		fprintf(logFile, "%d event%d NETWORK_FINISH finishes at NETWORK\n", event.time, event.jobID);
		
		enQueue(cpuQ, event);
		network_status = 0;
	}

	//going through queue
	if((Qsize(networkQ)) > 0 && (network_status == 0)){
		Event next = Qpeek(networkQ);
		deQueue(networkQ);
		
		fprintf(logFile, "%d event%d NETWORK_ARRIVAL arrives at NETWORK\n", next.time, next.jobID);
		
		int jobID = next.jobID;
		int time = globalTime;
		int type = NETWORK_ARRIVAL;
		Event networkArrival = createEvent(jobID, time, type);
		push(pq, networkArrival);
		network_status = 1;
	}

	fclose(logFile);

}


























