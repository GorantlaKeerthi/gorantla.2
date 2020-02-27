#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#include "shm.h"

static struct memory *mem = NULL;
static char * output = "output.txt";

struct val_max {
	int val;	//current value
	int max;	//maximum value
};
static struct val_max running = {0,2};	//running users
static struct val_max started = {0,4};	//started users
static struct val_max exited = {0, USERS_COUNT};	//exited users

static int first_number = 101;
static int increment_step = 4;

//For handling signals
static void signal_handler(const int sig){
	pid_t pid;
	int status;

	switch(sig){
		case SIGCHLD:
			while((pid = waitpid(-1, &status, WNOHANG)) > 0){
				running.val--;
				exited.val++;
				printf("OSS: Child %u has terminated at time %d s %d ns\n", pid, mem->clock.s, mem->clock.ns);
			}
			break;
		case SIGINT: case SIGTERM: case SIGALRM:
			exited.val = USERS_COUNT; //set the flat to true, to interrupt main loop
		default:
			printf("OSS: Signal %d at time %d s %d ns\n", sig, mem->clock.s, mem->clock.ns);
			break;
	}
}

void oss_usage(){
	printf("Usage: ./oss [-h] [-n %d] [-s %d] [-b %d] [-i %d] [-o %s]\n",
				started.max, running.max, first_number, increment_step, output);
	puts("-h Describe options");
	puts("-n maximum child processes");
	puts("-s simultaneous child processes");
	puts("-b First number");
	puts("-i Increment step");
	puts("-o Output filename");
}

//Convert integer number to string
char * num_arg(const unsigned int number){
	size_t len = snprintf(NULL, 0, "%d", number) + 1;
	char * str = (char*) malloc(len);
	snprintf(str, len, "%d", number);
	return str;
}

//Start a user process, if we can
static int exec_prime_user(){

	//check the limits on running/started users
	if(	(running.val >= running.max) ||
			(started.val >= started.max)){
		return 0;
	}

	//make arguments
	char * my_id = num_arg(started.val);
	char * my_number = num_arg(mem->numbers[started.val]);

	//start the child process
	pid_t pid = fork();
	if(pid < 0){
		perror("fork");
		return -1;

	}else if(pid == 0){
		execl("./user", "./user", my_id, my_number, NULL);
		perror("execl");
		exit(1);

	}else{

		printf("OSS: started child %d for number %d at time %d s %d ns\n",
			started.val, mem->numbers[started.val], mem->clock.s, mem->clock.ns);

		running.val++;
		started.val++;
	}

	free(my_id);
	free(my_number);
	return 0;
}

//print an array of numbers
static void print_numbers(const char * label, const unsigned int arr[], const int size){
	int i;
	printf("%s:", label);
	for(i=0; i < started.val; i++){
		if(arr[i] != 0){
			printf(" %d", arr[i]);
		}
	}
	printf("\n");
}

static int parse_arguments(const int argc, char * const argv[]){

	int option;
	while((option = getopt(argc, argv, "n:s:b:o:i:h")) != -1){
		switch(option){
			case 'h':
				oss_usage();
				exit(0);
			case 'n':
				started.max = atoi(optarg);
				break;
			case 's':
        running.max	= atoi(optarg);
        break;
			case 'b':
				first_number = atoi(optarg);
				break;
			case 'i':
	      increment_step = atoi(optarg);
	      break;
      case 'o':
				output = optarg;
				break;
			default:
				fprintf(stderr, "Error: Unknown option '%c'\n", option);
				return -1;
		}
	}

	if(started.max > USERS_COUNT){
		fprintf(stderr, "Error: -n < %d", USERS_COUNT);
		return -1;
	}

	if(running.max > USERS_COUNT){
		fprintf(stderr, "Error: -s < %d", USERS_COUNT);
		 return -1;
	}
	return 0;
}

int main(const int argc, char * const argv[]) {

	signal(SIGINT,	signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGALRM, signal_handler);
	signal(SIGCHLD, signal_handler);

	mem = shm_attach(0600 | IPC_CREAT);
	if(mem == NULL){
		return 1;
	}

	if(parse_arguments(argc, argv) == -1){
		return 2;
	}

	stdout = freopen(output, "w", stdout);
  if(stdout == NULL){
		perror("freopen");
		return 3;
	}

	//generate the list of numbers to be checked
	int i;
	for(i=0; i < started.max; i++){
		mem->numbers[i] = first_number + (i*increment_step);
	}

	alarm(2);

	//simulation loop
	while(exited.val < started.max){
		exec_prime_user();

    sem_wait(&mem->lock);
		clock_add_ns(&mem->clock, 10000);
		sem_post(&mem->lock);
	}

	printf("Simulation done, %d children done at time %d s %d ns\n", exited.val, mem->clock.s, mem->clock.ns);

	//show the number array with results
	print_numbers("Primes", mem->primes, started.val);
	print_numbers("Not Primes", mem->not_primes, started.val);
	print_numbers("Not Checked", &mem->numbers[started.val], started.max-started.val);

	shm_detach(1);

	return 0;
}
