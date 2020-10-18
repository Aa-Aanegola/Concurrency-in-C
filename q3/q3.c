/* Header files */
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
#include<semaphore.h>
#include<signal.h>
#include<errno.h>


/* Definitions used in functions */
#define INVALID -1
#define WAITING 0
#define PERFORMING 1
#define MAX_NUM 1024


/* Colors used for formatting output */
#define BASE "\033[0m"
#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define CYAN "\033[1;36m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[01;33m"
#define MAGENTA "\033[1;35m"


/* Structure definitions */
typedef struct performer_struct
{
	// The type of the performer
	char type;
	// The name of the performer
	char name[30];
	// The time after which the performer comes
	int delay;
	// The status of the performer
	int status;
	// The stage type they were allocated
	char stage_type;
	// This indicates if the performer already has a singer
	int has_singer;
	// Mutex lock for the performer
	pthread_mutex_t lock;
} performer_struct;


/* Global variables */
// Number of performers, acoustic stages and electric stages
int k, a, e;
// Number of coordinators
int c;
// Lower and upper bound for performance times
int t1, t2;
// Time after which performer will leave
int t;
// The thread ID arrays
pthread_t performer_ID[MAX_NUM];
// The array of performer structures
performer_struct performer[MAX_NUM];
// The semaphores for stages and coordinators
sem_t acoustic, electric, coordinators, singer_join;


/* Function declarations */
// Function to create all the necessary threads and semaphores
void master_thread();
// Function to generate a random number in the range [lb, ub] 
int randint(int lb, int ub);
// Function to create the performers
void *create_performer(void *args);
// Function to wait for an acoustic stage
void *wait_for_acoustic(void *args);
// Function to wait for an electric stage
void *wait_for_electric(void *args);
// Function to wait for a performer to be on stage for singers to join
void *wait_for_perf(void *args);



int main()
{
	srand(time(0));

	// Inputing the necessary values
	scanf("%d %d %d %d %d %d %d", &k, &a, &e, &c, &t1, &t2, &t);

	// Inputing the details of each performer
	for(int i = 0; i<k; i++)
	{
		performer[i].status = INVALID;
		scanf("%s %c %d", performer[i].name, &performer[i].type, &performer[i].delay);
		performer[i].delay = abs(performer[i].delay);
	}

	if(c == 0)
	{	
		fflush(NULL);
		printf(RED);
		printf("Abhinav found out that none of the coordinators were interested and decided to come!\n");
		fflush(NULL);
		c++;
	}

	master_thread();

	// Display message
	fflush(NULL);
	printf(RED);
	printf("Simulation concluded.\n");
}


void master_thread()
{
	// Initializing the semaphores
	sem_init(&acoustic, 0, a);
	sem_init(&electric, 0, e);
	sem_init(&coordinators, 0, c);
	sem_init(&singer_join, 0, 0);

	// Spawn all the performers
	for(int i = 0; i<k; i++)
		pthread_create(&performer_ID[i], NULL, create_performer, &performer[i]);

	// Join them after finishing 
	for(int i = 0; i<k; i++)
		pthread_join(performer_ID[i], NULL);
}


int randint(int lb, int ub)
{
	return lb+rand()%(ub-lb+1);
}


void *create_performer(void *args)
{
	// Extract the argument and store locally
	performer_struct *perf = (performer_struct*)args;

	// Make the performer come after a certain delay
	sleep(perf->delay);
	perf->status = WAITING;

	// Display message
	fflush(NULL);
	printf(GREEN);
	printf("%s %c arrived!\n", perf->name, perf->type);
	fflush(NULL);

	// Create threads that will monitor the stages
	pthread_t acoustic_thread, electric_thread, singer_thread;

	if(perf->type != 'b')
		pthread_create(&acoustic_thread, NULL, wait_for_acoustic, args);

	if(perf->type != 'v')
		pthread_create(&electric_thread, NULL, wait_for_electric, args);

	if(perf->type == 's')
		pthread_create(&singer_thread, NULL, wait_for_perf, args);

	// Wait for them to terminate
	if(perf->type != 'b')
		pthread_join(acoustic_thread, NULL);
	if(perf->type != 'v')
		pthread_join(electric_thread, NULL);
	if(perf->type == 's')
		pthread_join(singer_thread, NULL);

	// If the performer was still waiting even after t seconds elapsed, display message
	if(perf->status == WAITING)
	{
		fflush(NULL);
		printf(RED);
		printf("%s %c got bored and left :(\n", perf->name, perf->type);
		fflush(NULL);
		return NULL;
	}

	// If the performer is a singer don't give them a tshirt
	if(perf->type == 's')
	return NULL;
		
	// Wait for the coordinator to be assigned to you 
	sem_wait(&coordinators);

	// Display messages and small delay
	fflush(NULL);
	printf(CYAN);	
	printf("%s is waiting for the coordinator to find their T-shirt size...\n", perf->name);
	fflush(NULL);

	sleep(2);

	fflush(NULL);
	printf(GREEN);	
	printf("%s has received a T-shirt!\n", perf->name);
	fflush(NULL);	

	// Release the coordinator 
	sem_post(&coordinators);

	return NULL;
}

void *wait_for_acoustic(void *args)
{
	// Extract relevant information
	performer_struct *perf = (performer_struct*)args;

	// Timespec structure for sem_timedwait
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	
	ts.tv_sec += t;
		
	// Wait only for t seconds
	int check;
	while((check = sem_timedwait(&acoustic, &ts)) == -1 && errno == EINTR);

	// If we acquired the semaphore
	if(check != -1)
	{
		// Lock this performer and update it's information
		pthread_mutex_lock(&perf->lock);

		// If the performer was performing then release the stage, and unlock
		if(perf->status == PERFORMING)
		{
			sem_post(&acoustic);
			pthread_mutex_unlock(&perf->lock);	
			return NULL;
		}

		// If the status is invalid, ie they've terminated then again release the stage and unlock
		if(perf->status == INVALID)
		{
			sem_post(&acoustic);
			pthread_mutex_unlock(&perf->lock);
			return NULL;
		}

		// Otherwise set the status to performing, and if the performer is not a singer allow singers to join
		perf->status = PERFORMING;
		if(perf->type != 's')
			sem_post(&singer_join);
		pthread_mutex_unlock(&perf->lock);
		
		int perf_time = randint(t1, t2);	

		// Display message and sleep for the performance time
		fflush(NULL);
		printf(YELLOW);
		printf("%s is performing %c at acoustic stage for %d seconds!\n", perf->name, perf->type, perf_time);
		fflush(NULL);

		sleep(perf_time);

		// Acquire the lock to the performer, and update it's information
		pthread_mutex_lock(&perf->lock);
		if(perf->has_singer)
			sleep(2);
		else
			sem_trywait(&singer_join);
		perf->status = INVALID;

		// Release the lock and the stage
		pthread_mutex_unlock(&perf->lock);
		sem_post(&acoustic);

		// Display message
		fflush(NULL);
		printf(MAGENTA);
		printf("%s's performance ended.\n", perf->name);
		fflush(NULL);
	}

	return NULL;
}

void *wait_for_electric(void *args)
{
	// This function is an exact copy of the wait_for_acoustic function

	performer_struct *perf = (performer_struct*)args;

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	
	ts.tv_sec += t;
	
	int check;
	while((check = sem_timedwait(&electric, &ts)) == -1 && errno == EINTR);

	if(check != -1)
	{
		pthread_mutex_lock(&perf->lock);
		if(perf->status == PERFORMING)
		{
			sem_post(&electric);
			pthread_mutex_unlock(&perf->lock);		
			return NULL;
		}

		if(perf->status == INVALID)
		{
			sem_post(&electric);
			pthread_mutex_unlock(&perf->lock);
			return NULL;
		}

		perf->status = PERFORMING;
		if(perf->type != 's')
			sem_post(&singer_join);
		pthread_mutex_unlock(&perf->lock);		
		
		int perf_time = randint(t1, t2);	

		fflush(NULL);
		printf(YELLOW);
		printf("%s is performing %c at electric stage for %d seconds!\n", perf->name, perf->type, perf_time);
		fflush(NULL);

		sleep(perf_time);

		pthread_mutex_lock(&perf->lock);
		if(perf->has_singer)
			sleep(2);
		else
			sem_trywait(&singer_join);
		perf->status = INVALID;
		pthread_mutex_unlock(&perf->lock);

		fflush(NULL);
		printf(MAGENTA);
		printf("%s's performance ended.\n", perf->name);
		fflush(NULL);

		sem_post(&electric);
	}

	return NULL;
}

void *wait_for_perf(void *args)
{
	// Extract data
	performer_struct *perf = (performer_struct*)args;

	// Timespec struct for sem_timedwait
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	
	ts.tv_sec += t;
	
	int check;
	while((check = sem_timedwait(&singer_join, &ts)) == -1 && errno == EINTR);

	// If we acquired the semaphore
	if(check != -1)
	{
		// Lock this singer and update data
		pthread_mutex_lock(&perf->lock);
		if(perf->status == PERFORMING)
		{
			sem_post(&singer_join);
			pthread_mutex_unlock(&perf->lock);
			return NULL;
		}
			
		// Iterate over all performers and see if the singer can join one of them
		for(int i = 0; i<k; i++)
		{
			if(performer[i].type != 's' && performer[i].status == PERFORMING && performer[i].has_singer == 0)
			{
				int check = pthread_mutex_trylock(&performer[i].lock);
				if(check == 0 && performer[i].status == PERFORMING && performer[i].has_singer == 0)
				{
					perf->status = INVALID;
					performer[i].has_singer = 1;
					fflush(NULL);
					printf(BLUE);
					printf("%s has joined %s's performance, the performance is extended by 2 seconds!\n", perf->name, performer[i].name);
					fflush(NULL);
					pthread_mutex_unlock(&performer[i].lock);	
					break;
				}
				else
					pthread_mutex_unlock(&performer[i].lock);
			}
		}
	}

	pthread_mutex_unlock(&perf->lock);

	return NULL;
}