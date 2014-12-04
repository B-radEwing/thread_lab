#ifndef _queue_h
#define _queue_h

typedef struct Customer Customer;
typedef struct Queue Queue;

struct Customer { // customer struct. nodes for linked list
	int start_time;
	int end_time;
	Customer *next;
};

struct Queue { // queue struct. a linked list
	int size;
	Customer *head;
};

extern int sim_time;

struct Queue *queue_create();
struct Customer *line_queue_add(Queue *queue, int i);
struct Customer *served_queue_add(Queue *queue, Customer *c);
struct Customer *queue_pop(Queue *queue);

#endif