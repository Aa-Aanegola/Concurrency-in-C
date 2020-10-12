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
sem_t acoustic, electric, coordinators;
// The lock for the performers
pthread_mutex_t perf_locks[MAX_NUM];


/* Function declarations */
// Function to create all the necessary threads and semaphores
void master_thread();
// Function to generate a random number in the range [lb, ub] 
int randint(int lb, int ub);
// Function to create the performers
void *create_performer(void *args);


int main()
{
	srand(time(0));

	// Inputing the necessary values
	scanf("%d %d %d %d %d %d %d", &k, &a, &e, &c, &t1, &t2, &t);

	k = abs(k);
	a = abs(a);
	e = abs(e);
	c = abs(c);
	t1 = abs(t1);
	t2 = abs(t2);
	t = abs(t);

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
	printf(GREEN);
	printf("Finished\n");
}


void master_thread()
{
	// Initializing the semaphores
	sem_init(&acoustic, 0, a);
	sem_init(&electric, 0, e);
	sem_init(&coordinators, 0, c);

	// Spawn all the performers
	for(int i = 0; i<k; i++)
		pthread_create(&performer_ID[i], NULL, create_performer, &performer[i]);

	// Join them after finishing 
	for(int i = 0; i<k; i++)
		pthread_join(performer_ID[i], NULL);
}


int randint(int lb, int ub)
{
	return lb+rand()%ub-lb+1;
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

	// Make sure that the impatient performers leave
	int elapsed_time = 0;

	// While the performer is still patient
	while(elapsed_time < t)
	{
		// If they're not a bassist check the acoustic stage
		if(perf->type != 'b')
		{
			int succ = sem_trywait(&acoustic);
			if(succ == 0)
			{
				perf->stage_type = 'a';
				perf->status = PERFORMING;
			}
		}

		// If they weren't assigned a slot on the acoustic stage
		// If they aren't a violinist check electric stage
		if(perf->status != PERFORMING && perf->type != 'v')
		{
			int succ = sem_trywait(&electric);
			if(succ == 0)
			{
				perf->stage_type = 'e';
				perf->status = PERFORMING;
			}
		}

		// If they're a singer and not performing, check other musicians
		if(perf->status != PERFORMING && perf->type == 's')
		{
			for(int i = 0; i<k; i++)
			{
				// If the musician is performing and doesn't have a singer
				if(performer[i].status == PERFORMING && performer[i].has_singer != 1 && performer[i].type != 's')
				{
					// Try to acquire the lock to that performer
					if(pthread_mutex_trylock(&perf_locks[i]) == 0)
					{
						// If you can, recheck condition
						if(performer[i].status == PERFORMING && performer[i].has_singer != 1)
						{	
							// Set the as singer flag to extend performance
							performer[i].has_singer = 1;

							// Display message
							fflush(NULL);
							printf(CYAN);
							printf("%s joined %s's performance, performance extended by 2 seconds\n", perf->name, performer[i].name);
							fflush(NULL);

							// Unlock the thread - not strictly required but clean
							pthread_mutex_unlock(&perf_locks[i]);

							return NULL;
						}
						// If it was modified, unlock it and check next
						else
							pthread_mutex_unlock(&perf_locks[i]);
					}
				}
			}
		}

		// If the performer found a stage, start performance
		if(perf->status == PERFORMING)
			break;

		// Otherwise wait one second and retry
		sleep(1);
		elapsed_time++;
	}

	// If we didn't find a stage in the time alotted leave
	if(elapsed_time == t)
	{
		fflush(NULL);
		printf(RED);
		printf("%s %c got bored and left :(\n", perf->name, perf->type);
		fflush(NULL);

		// Remove this performer
		perf->status = INVALID;

		return NULL;
	}

	// Time taken for the performance
	int performance_time = randint(t1, t2);

	// Display message
	fflush(NULL);
	printf(BLUE);
	if(perf->stage_type == 'a')
		printf("%s is performing %c at acoustic stage for %d seconds\n", perf->name, perf->type, performance_time);
	else
		printf("%s is performing %c at electric stage for %d seconds\n", perf->name, perf->type, performance_time);
	fflush(NULL);

	// Perform the song, and if a singer joined add the time 
	sleep(performance_time);
	if(perf->has_singer)
		sleep(2);

	// Remove this performer
	perf->status = INVALID;

	// Display message
	fflush(NULL);
	printf(YELLOW);
	if(perf->stage_type == 'a')
		printf("%s's performance at acoustic stage finished!\n", perf->name);
	else
		printf("%s's performance at electric stage finished!\n", perf->name);
	fflush(NULL);

	// Reset the semaphore (release)
	if(perf->stage_type == 'a')
		sem_post(&acoustic);
	else
		sem_post(&electric);

	// Short delay, and then try and acquire the coordinator semaphore, here we wait indefinitely
	sleep(1);
	sem_wait(&coordinators);

	// Display message
	fflush(NULL);
	printf(MAGENTA);
	printf("%s now collecting T-Shirt\n", perf->name);
	fflush(NULL);

	// Delay for coordinator to find size
	sleep(2);

	// Display message
	fflush(NULL);
	printf(CYAN);
	printf("%s has received a T-Shirt!\n", perf->name);
	fflush(NULL);

	// Release the coordinator
	sem_post(&coordinators);

	return NULL;
}