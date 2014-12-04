#import "queue.h"
#include <stdlib.h>
#include <stdio.h>

/* Create and initialize a queue */
Queue *queue_create() {
	Queue *queue = malloc(sizeof(*queue));
	queue->head = NULL;
	queue->size = 0;
	return queue;
}

/* Function used by the customer generator thread to add customers to the line queue */
Customer *line_queue_add(Queue *queue, int i) { 
	Customer *customer;
	customer = malloc(sizeof(*customer));
	customer->start_time = i; 
	customer->next = NULL;
	if (queue->head == NULL) {
		queue->head = customer;
	} else {
		Customer *temp = queue->head;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = customer;
	}
	queue->size++;
	return customer;
}
/* Function used by teller threads to add customers to the served queue */
Customer *served_queue_add(Queue *queue, Customer *c) {
	c->next = NULL;
	if (queue->head == NULL) {
		queue->head = c;
	} else {
		Customer *temp = queue->head;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = c;
	}
	queue->size++;
	return c;

}

/* Removes the head of the queue, fills in the end time for the customer data */
Customer *queue_pop(Queue *queue) {
	Customer* head = queue->head;
	head->end_time = sim_time;

	queue->head = head->next;
	queue->size--;
	
	return head;

}