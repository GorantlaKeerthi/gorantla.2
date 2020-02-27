#include <stdio.h>
#include <stdlib.h>	//atoi
#include "shm.h"

int main(const int argc, char * argv[]) {

	if(argc != (2+1)){	/* if we don't have 2 arguments */
		fprintf(stderr, "Usage: ./user my_index my_number\n");
		return -1;
	}
	const int my_index = atoi(argv[1]);
	int my_number = atoi(argv[2]);

	/* Get a pointer to the shared memory */
	struct memory *mem = shm_attach(0);
	if(mem == NULL){
		return -1;
	}

	sem_wait(&mem->lock);
	struct clock alarm = mem->clock;	/* Copy clock */
	sem_post(&mem->lock);

	clock_add_ns(&alarm, 1000000);	/* Set runtime to 1 milisecond */

	/* Check if my number is a prime number */
	int i=2, past = 0, not_prime = 0;
  while((i < my_number) && !past) {

			if((my_number % i) == 0){
				not_prime = 1;
				break;
			}

			/* check clock alarm*/
			sem_wait(&(mem->lock));
			past = clock_alarm(&mem->clock, &alarm);
			sem_post(&(mem->lock));

			i=i+1;
  }

	/* save our result to memory */
	sem_wait(&(mem->lock));
	if(not_prime){
		mem->not_primes[my_index] = -1*my_number;
	}else{
		mem->primes[my_index] = my_number;
	}
	sem_post(&(mem->lock));

	shm_detach(0);

	return 0;
}
