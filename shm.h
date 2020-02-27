#include <semaphore.h>
#include "clock.h"

#define USERS_COUNT 20

struct memory {
	sem_t lock;
	struct clock clock;
	unsigned int numbers[USERS_COUNT];
	unsigned int primes[USERS_COUNT];
	unsigned int not_primes[USERS_COUNT];
};

struct memory * shm_attach(const int flags);
						int shm_detach(const int clear);
