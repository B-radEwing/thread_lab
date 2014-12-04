#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "queue.h"

// Struct to feed multiple arguements into the teller threads.
struct teller_args {
	Queue *line;
	Queue *served;
};


/* Global Variables */
pthread_mutex_t mutex_line;
pthread_mutex_t mutex_served;
int sim_time;
float customer_prob;
int customer_time;

/* Function Prototypes */
void *teller(void *param);
void *generator(void *param);
void calculate_average_wait(Queue *queue);

/* Queue Helper Functions */
/*
struct Queue *queue_create();
struct Customer *line_queue_add(Queue *queue, int i);
struct Customer *served_queue_add(Queue *queue, Customer *c);
struct Customer *queue_pop(Queue *queue);
*/



int main(int argc, char *argv[]) {

	/* Check for sufficient number of arguements */

	if (argc != 6) {
		printf("ERROR! Usage: ./banksim (Simulation Time) (Customer Arrival Probability), (Number of Tellers), (Customer Service Time), (Sampling Time)\n");
		exit(1);
	} else {
		/* Parse argv */
		sim_time = atoi(argv[1]);
		customer_prob = atof(argv[2]);
		int num_tellers = atoi(argv[3]);
		customer_time = atoi(argv[4]);
		int sampling_time = atoi(argv[5]);

		// Convert mintues to seconds
		sim_time = sim_time * 60;
		customer_time = customer_time * 60;
		sampling_time = sampling_time * 60;

		int elapsed_time = 0;

		Queue *customer_line = queue_create(); // declare queues
		Queue *customers_served = queue_create();

		struct teller_args args;
		args.line = customer_line;
		args.served = customers_served;

		pthread_t tellers[num_tellers]; // declare teller threads
		pthread_t generation; // declare customer generation thread
		pthread_create(&generation, NULL, generator, customer_line);
		for (int i = 0; i < num_tellers; i++)
			pthread_create(&tellers[i], NULL, teller, &args);

		while (sim_time > 0) {
			sleep(1);
			sim_time--;
			elapsed_time++;
			if ((elapsed_time % sampling_time) == 0) {
				printf("Time: %d   Queue Length: %d\n", elapsed_time/60, customer_line->size);
			}
		}
		calculate_average_wait(customers_served);
		exit(0);
	}
}



/* Teller Function. */
void *teller(void *args) {
	unsigned int seed = time(NULL);
	struct teller_args *queues = args;
	while(1) {
		sleep(1);
		if (queues->line->size > 0) {
			pthread_mutex_lock(&mutex_line);
			Customer * customer = queue_pop(queues->line);
			pthread_mutex_unlock(&mutex_line);
			int randomizer = ((rand_r(&seed) % 60) - 30);
			sleep(customer_time + randomizer);
			pthread_mutex_lock(&mutex_served);
			served_queue_add(queues->served, customer);
			pthread_mutex_unlock(&mutex_served);
		} else {
			pthread_mutex_unlock(&mutex_line);
		}
	}	
}
/* Custommer Generator Function */
void *generator(void *queue) {
	unsigned int seed = 12345;
	while(1) {
		sleep(1);
		float rand_value = (float)rand_r(&seed) / RAND_MAX;
		if (rand_value <= customer_prob) {
			pthread_mutex_lock(&mutex_line);
			line_queue_add(queue, sim_time);
			pthread_mutex_unlock(&mutex_line);
		}
	}
	
}
/* Helper Function to print ending information */
void calculate_average_wait(Queue *queue) {
	float average_wait = 0;
	int size = queue->size;

	const Customer *c = queue->head;
	while (c) {
		average_wait = average_wait + c->start_time - c->end_time;
		c = c->next;
	}
	average_wait = (average_wait / size) / 60 ; //convert back to minutes
	printf("Customers Served: %d   Average Wait Time: %f\n", size, average_wait);
}
