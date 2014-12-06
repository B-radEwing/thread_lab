/* Project 4: Thread Lab
 * Brad Ewing
 * 
 * This program 'simulates' a simple bank customer/teller interaction. A generator (producer) thread
 * creates customers and places them into a queue (a linked list), while a dynamic number of teller
 * threads (consumers) pops customers out of that queue, and once finishes processing them, places 
 * the customers into a completed queue (linked list). The customers are represented by structs 
 * and contain the time when they were created and the time when they left the waiting queue.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "queue.h"

// Struct to feed multiple arguements into the teller threads.
struct teller_args {
	Queue *line;
	Queue *served;
	int cust_time;
};

// struct to feed multiple arguements into generator thread.
struct customer_args {
	Queue *line;
	float cust_prob;
};

/* Global Variables */
static pthread_mutex_t mutex_line = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_served = PTHREAD_MUTEX_INITIALIZER;
int sim_time;

/* Function Prototypes */
void *teller(void *param);
void *generator(void *param);
void calculate_average_wait(Queue *queue);

int main(int argc, char *argv[]) {

	/* Check for sufficient number of arguements */

	if (argc != 6) {
		printf("ERROR! Usage: ./banksim (Simulation Time) (Customer Arrival Probability), (Number of Tellers), (Customer Service Time), (Sampling Time)\n");
		exit(1);
	} else {
		/* Parse argv */
		sim_time = atoi(argv[1]); // length of simulation
		float customer_prob = atof(argv[2]); // probability of customer arriving in one second intervals
		int num_tellers = atoi(argv[3]); // number of teller threads
		int customer_time = atoi(argv[4]); // time it takes for teller to serve customer
		int sampling_time = atoi(argv[5]); // interval (in minutes) to post queue length to terminal

		// Convert mintues to seconds
		sim_time = sim_time * 60;
		customer_time = customer_time * 60;
		sampling_time = sampling_time * 60;

		int elapsed_time = 0;

		Queue *customer_line = queue_create(); // declare queues
		Queue *customers_served = queue_create();

		struct teller_args tell_args; // teller thread arguements
		tell_args.line = customer_line;
		tell_args.served = customers_served;
		tell_args.cust_time = customer_time;

		struct customer_args cust_args;
		cust_args.line = customer_line;
		cust_args.cust_prob = customer_prob;


		pthread_t tellers[num_tellers]; // declare teller threads
		pthread_t generation; // declare customer generation thread
		pthread_create(&generation, NULL, generator, &cust_args);
		for (int i = 0; i < num_tellers; i++)
			pthread_create(&tellers[i], NULL, teller, &tell_args);

		while (sim_time > 0) {
			sleep(1);
			sim_time--;
			elapsed_time++;
			if ((elapsed_time % sampling_time) == 0) {
				printf("Time: %d   Queue Length: %d\n", elapsed_time/60, customer_line->size);
			}
		}
		pthread_mutex_lock(&mutex_served);
		calculate_average_wait(customers_served);
		pthread_mutex_unlock(&mutex_served);
		exit(0);
	}
}

/* Teller Threads Function. */
void *teller(void *args) {
	unsigned int seed = time(NULL);
	struct teller_args *queues = args;
	while(1) {
		sleep(rand_r(&seed) % 5 + 1); // sleep random length 1-5 seconds
		pthread_mutex_lock(&mutex_line);
		if (queues->line->size > 0) {
			Customer * customer = queue_pop(queues->line);
			pthread_mutex_unlock(&mutex_line);
			int randomizer = ((rand_r(&seed) % 60) - 30);
			sleep(queues->cust_time + randomizer);
			pthread_mutex_lock(&mutex_served);
			served_queue_add(queues->served, customer);
			pthread_mutex_unlock(&mutex_served);
		} else {
			pthread_mutex_unlock(&mutex_line);
		}
	}	
}

/* Customer Generator Thread Function */
void *generator(void *args) {
	struct customer_args *info = args;
	unsigned int seed = 12345;
	while(1) {
		sleep(1);
		float rand_value = (float)rand_r(&seed) / RAND_MAX;
		if (rand_value <= info->cust_prob) {
			pthread_mutex_lock(&mutex_line);
			line_queue_add(info->line, sim_time);
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
