# Meltdown simulator
Welcome to the Meltdown simulator. Meltdown is an annual event at IIIT hosted by the music club. Seeing as we missed performing at last years Meltdown, here is a simulation for all the performers of our college!
## To run
```
gcc q3.c -lpthread
./a.out
```
These commands should be run in a UNIX based terminal from the directory in which ```q3.c``` is stored.  
## Input format
```
<number of performers> <number of acoustic stages> <number of electric stages> <number of coordinators> <minimum performance time> <maximum performance time> <patience threshold>
<name of performer> <character code of performer> <performer delay>
```  
For example, 
```
1 1 0 1 10 15 5
Aakash v 5
```  
Would mean that there is 1 performer, 1 acoustic stage, 1 coordinator, the minimum performance time is 10 seconds, the maximum performance time is 15 seconds and a performer will wait for a maximum of 5 seconds before leaving out of impatience. The performers name is Aakash, he plays the violin and he comes 5 seconds after the start of the event.  
## Output format
```
Ritvik p arrived!
```  
This indicates that Ritvik who is a pianist has arrived and is waiting to perform.  

```
Aakash is performing g at electric stage 1 for 6 seconds
```
This indicates that Aakash who is a guitarist is performing at an electric stage for 6 seconds.   

```
Aakash's performance at acoustic stage finished!
```
This indicates that Aakash has finished his performance, and the stage is now free for the next musician.  

```
Raghav joined Mehuls's performance, performance extended by 2 seconds
```
This indicates that Raghav who is a singer decided to sing along to Mehul's performance (Mehul is an instrumentalist and we would've discovered which instrument he plays before).  

```
Arihant b got bored and left :(
```
This indicates that Arihant had been in the queue for longer than the patience threshold and decided to leave.  

```
Aman now collecting T-Shirt
```
This indicates that Aman has finished his performance, and is waiting for a coordinator to find his T-Shirt size (this takes 2 seconds).  

```
Abhijeet has received a T-Shirt!
```
This indicates that the coordinator found Abhijeet's tshirt size and has given it to him.  

```
Finished
```
Simulation concluded.  

## Code Breakdown
### General 
```
/* Definitions used in functions */
#define INVALID -1
#define WAITING 0
#define PERFORMING 1
#define MAX_NUM 1024
```
MAX_NUM is the maximum number of performers allowed. The other 3 values are used to indicate the current status of every performer. 

```
/* Colors used for formatting output */
#define BASE "\033[0m"
#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define CYAN "\033[1;36m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[01;33m"
#define MAGENTA "\033[1;35m"
```
These are ANSI color codes used to format the output.  

```
fflush(NULL);
printf(COLOR);
printf("Content");
fflush(NULL);
```
This is the format of a general print statement that prints content in the described color.  

### Structures 
```
/* Structure definitions */
typedef struct performer_struct
{
	char type;
	char name[30];
	int delay;
	int status;
	char stage_type;
	int has_singer;
} performer_struct;
```
This structure stores the date of every performer. The type indicates the instrument that they play, the delay stores how late they are, their status stores one of the 3 values defined before, the stage_type stores either 'a' or 'e' to indicate an acoustic or electric stage and the has_singer flag stores 1 if a singer joined them (or if they are a singer) and 0 otherwise.  

### Global variables
```
int k, a, e;
```
These variables store the number of performers, acoustic stages and electric stages respectively.  

```
int c;
```
This variable stores the number of music club coordinators.  

```
int t1, t2;
```
These variables store the lower and upper bound on performance times respectively.  

```
int t;
```
This variable stores the patience threshold of every performer.  

```
pthread_t performer_ID[MAX_NUM];
```
This array stores the thread IDs of all the performers. It is used to create and join the threads before termination.  

```
performer_struct performer[MAX_NUM];
```
This array stores the data of all the performers.  

```
sem_t acoustic, electric, coordinators;
```
These variables are semaphores that indicate the number of free acoustic stages, electric stages and coordinators at any given point in time.

```
pthread_mutex_t perf_locks[MAX_NUM];
```
This array stores a mutex lock for each performer, to ensure that no two singers can join the same instrumentalist.  

### Functions
#### void master_thread();
```
sem_init(&acoustic, 0, a);
sem_init(&electric, 0, e);
sem_init(&coordinators, 0, c);
```
Initializes all the semaphores to their correct values.  

```
for(int i = 0; i<k; i++)
	pthread_create(&performer_ID[i], NULL, create_performer, &performer[i]);

for(int i = 0; i<k; i++)
	pthread_join(performer_ID[i], NULL);
```
Create all the threads with their respective structure passed as reference. Join all the threads before returning to main.  

#### int randint(int lb, int ub);
```
return lb+rand()%ub-lb+1;
```
Returns a random number between lb and ub, both inclusive.  

#### void *create_performer(void *args);

```
performer_struct *perf = (performer_struct*)args;

sleep(perf->delay);
perf->status = WAITING;
```
Extracts the data passed to the function and sleeps for the delay that the performer has. Also sleeps for the duration of the delay, and changes the status of the performer.  

```
int elapsed_time = 0;

while(elapsed_time < t)
{
	...
}
```
This is used to ensure that the performer only waits t seconds, and leaves if they don't get a stage in that time. The ellipsis represent the rest of the code.   

```
if(perf->type != 'b')
{
	int succ = sem_trywait(&acoustic);
	if(succ == 0)
	{
		perf->stage_type = 'a';
		perf->status = PERFORMING;
	}
}
```
If the performer is not a bassist, then try and wait on the acoustic stage semaphore. If the wait was successful, the semaphore value will be decremented and the function will return a 0. We then set the stage_type to acoustic, and the status of the performer to PERFORMING.  

```
if(perf->status != PERFORMING && perf->type != 'v')
{
	int succ = sem_trywait(&electric);
	if(succ == 0)
	{
		perf->stage_type = 'e';
		perf->status = PERFORMING;
	}
}
```
Similar to the above snippet, however here we check if the status is already performing, in which case we ignore it. 

```
if(perf->status != PERFORMING && perf->type == 's')
{
	for(int i = 0; i<k; i++)
	{
		if(performer[i].status == PERFORMING && performer[i].has_singer != 1 && performer[i].type != 's')
		{
			if(pthread_mutex_trylock(&perf_locks[i]) == 0)
			{
				if(performer[i].status == PERFORMING && performer[i].has_singer != 1)
				{	
					performer[i].has_singer = 1;
					pthread_mutex_unlock(&perf_locks[i]);
					return NULL;
				}
				else
					pthread_mutex_unlock(&perf_locks[i]);
			}
		}
	}
}
```
If the performer is a singer, we loop through all the other performers and see if they're performing currently. If we find a performer who is currently performing and they don't have an accompanying singer, we try and acquire the lock for the performer to ensure that only one singer can join any other performer. We recheck the performing condition to make sure that the performance hasn't concluded and then add them to the performance. If we can't add them to the performance we unlock and keep looking. Singers can't join other singers which is reflected in the third clause in the if statement.  

```
if(perf->status == PERFORMING)
	break;

sleep(1);
elapsed_time++;
<end of while here>
```
If we found a stage for the performer, then exit this loop. If not we sleep and then increase the elapsed time. The sleep here doesn't affect anything as we have no priorities in the performers. This means that the stage will be acquired within the span of a second by some performer, but we don't consider it because there is no priority.  

```
if(elapsed_time == t)
{
	perf->status = INVALID;

	return NULL;
}
```
If we didn't find a stage for the performer, remove them from the list of valid performers and make them leave.  

```
int performance_time = randint(t1, t2);

sleep(performance_time);
if(perf->has_singer)
	sleep(2);
```
Generate a random performance time in the given range, and then sleep (perform) for that duration. If a singer decided to join this performer, then sleep for an additional 2 seconds.  

```
if(perf->stage_type == 'a')
	sem_post(&acoustic);
else
	sem_post(&electric);
```
Here we do a post operation on the semaphore we did the wait operation on. This is equivalent to freeing the stage after the performance, and increasing the semaphore value.

```
sleep(1);
sem_wait(&coordinators);
```
Here the performer tries to wait on the coordinators semaphore to get the tshirt. I also included a slight delay to reflect time taken to get to the tshirt stand. There is no chance of an infinite wait because every performer posts the semaphore roughly 2 seconds after waiting on it.  

```
sleep(2);

sem_post(&coordinators);
return NULL;
```
We wait for the time it takes the coordinator to get the tshirt for the performer, and then do a post operation that frees the coordinator to give tshirts to other performers (increment the semaphore value). After this the performer leaves.  

#### Note for readers
Here I have omitted the print statements to ensure readability and coherence.  
By 'lock the performer' I mean a singer acquires a lock for a particular performer (perf_locks[i] for the i'th performer) and therefore no other singer will be able to manipulate its data.  
A wait operation is equivalent to a 'P' operation on semaphores, while the post operation is the equivalent of the 'V' operation.  

## Assumptions
The performers teleport to stages, and start performing instantly. However while leaving the stage they take finite time to get to the T-Shirt stall (mainly to ensure that it's simple to follow the print statements).  
No two singers can perform together, singers can only perform with instrumentalists.  
