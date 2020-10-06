#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<time.h>

/* Exit codes */
#define EXIT_FAIL 1
#define EXIT_SUCC 0
#define FORCE_QUIT -1

/* Colors used for formatting output */
#define BASE "\033[0m"
#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define CYAN "\033[1;36m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[01;33m"
#define MAGENTA "\033[1;35m"


/* Global variables */
// Number of companies, vaccination zones and students
int n, m, o;

// Array that stores probabilities of success of vaccinations
int *prob_succ;

// Array that stores vaccination attempts per student
int *vacc_attempts;

// Arrays of thread IDs for pharma companies, vaccination zones and students
pthread_t *pharm, *vacc, *stud;


/* Function Declarations */
// Function that returns a random number between ub and lb both inclusive
int randint(int lb, int ub);


int main()
{
	srand(time(0));

}


int randint(int lb, int ub)
{
	return lb+rand()%(ub-lb+1);
}