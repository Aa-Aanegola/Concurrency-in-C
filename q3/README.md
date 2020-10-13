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
Aakash is performing g at electric stage for 6 seconds
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
This indicates that Arihant who is a bassist had been in the queue for longer than the patience threshold and decided to leave.  

```
Aman is waiting for the coordinator to find their T-shirt size...
```
This indicates that Aman has finished his performance, and is waiting for a coordinator to find his T-Shirt size (this takes 2 seconds).  

```
Abhijeet has received a T-Shirt!
```
This indicates that the coordinator found Abhijeet's tshirt size and has given it to him.  

```
Simulation concluded.
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
	pthread_mutex_t lock;
} performer_struct;
```
This structure stores the date of every performer. The type indicates the instrument that they play, the delay stores how late they are, their status stores one of the 3 values defined before, the stage_type stores either 'a' or 'e' to indicate an acoustic or electric stage and the has_singer flag stores 1 if a singer joined them (or if they are a singer) and 0 otherwise. The mutex is to ensure that no parallel updates occur on the data stored in the structure (as multiple threads can update a single instance of the structure).  

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
sem_t acoustic, electric, coordinators, singer_join;
```
These variables are semaphores that indicate the number of free acoustic stages, electric stages and coordinators at any given point in time. The last semaphore is an interesting one. It stores the number of currently performances that do not have a singer. Since we can't do this using a semaphore, we 'invert' it by initializing it with the number of performers, and then calling ```sem_wait()``` until its value reaches 0. Now whenever there is a new performance without a singer, we simply do a ```sem_post()``` operation.  

### Functions
#### void master_thread();
This function is used to create all the performer threads, initialize all the semaphores and join all of them before returning to main.  
```
sem_init(&acoustic, 0, a);
sem_init(&electric, 0, e);
sem_init(&coordinators, 0, c);
```
Initializes all the semaphores to their correct values.  

```
for(int i = 0; i<k; i++)
		sem_wait(&singer_join);
```
Here we 'invert' the singer_join semaphore by driving its value to 0.  

```
for(int i = 0; i<k; i++)
	pthread_create(&performer_ID[i], NULL, create_performer, &performer[i]);

for(int i = 0; i<k; i++)
	pthread_join(performer_ID[i], NULL);
```
Create all the threads with their respective structure passed as reference. Join all the threads before returning to main.  

#### int randint(int lb, int ub);

This function is used to generate random numbers used for the performance times.  
```
return lb+rand()%ub-lb+1;
```
Returns a random number between lb and ub, both inclusive.  

#### void *create_performer(void *args);

This function is the function that all the performer threads are initialized with. It is responsible for all the tasks of the performers.  
```
performer_struct *perf = (performer_struct*)args;

sleep(perf->delay);
perf->status = WAITING;
```
Extracts the data passed to the function and sleeps for the delay that the performer has. Also sleeps for the duration of the delay, and changes the status of the performer.  

```
pthread_t acoustic_thread, electric_thread, singer_thread;

if(perf->type != 'b')
	pthread_create(&acoustic_thread, NULL, wait_for_acoustic, args);

if(perf->type != 'v')
	pthread_create(&electric_thread, NULL, wait_for_electric, args);

if(perf->type == 's')
	pthread_create(&singer_thread, NULL, wait_for_perf, args);

if(perf->type != 'b')
	pthread_join(acoustic_thread, NULL);
if(perf->type != 'v')
	pthread_join(electric_thread, NULL);
if(perf->type == 's')
	pthread_join(singer_thread, NULL);
```
Here we create separate threads to monitor the acoustic stage, the electric stage and if the performer is a singer then the other performers (the sing_join semaphore). We also wait for them to terminate, so we can carry on with the functionality of this thread.  

```
if(perf->status == WAITING)
	return NULL;

if(perf->type == 's')
	return NULL;
```
After the threads created previously terminate (indicating that the performer has finished their task), we check if the performer was still waiting after the allowed time. If they were we return. If the performer is a singer, we return as singers do not receive T-shirts.  

```
	sem_wait(&coordinators);	

	sleep(2);

	sem_post(&coordinators);

	return NULL;
```
If the performer was not a singer, and they did not leave out of boredom, they wait for a coordinator to be assigned to them (wait for semaphore) and then after a small delay receive their T-shirt. After the performer receives their T-shirt, the coordinator is free to help other performers. The performer leaves (return).

#### void *wait_for_acoustic(void *args);

This function is used to monitor the acoustic stages, and allocate one of the stages to the performer that calls it.  
```
performer_struct *perf = (performer_struct*)args;
```
Extracts the data passed to the function.  

```
struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	
ts.tv_sec += t;
	
int check = sem_timedwait(&acoustic, &ts);
```
Here we define a timespec struct that the ```sem_timedawait()``` function uses. We increase its seconds counter by t indicating the patience threshold, and then wait on the acoustic stage semaphore. If we acquire it then wait will return 0, otherwise it will return -1.  

```
if(check != -1)
{
	...
}
```
If we successfully waited on the semaphore, then proceed to allocate the stage to the performer.  

```
pthread_mutex_lock(&perf->lock);

if(perf->status == PERFORMING)
{
	sem_post(&acoustic);
	pthread_mutex_unlock(&perf->lock);		
	return NULL;
}

if(perf->status == INVALID)
{
	sem_post(&acoustic);
	pthread_mutex_unlock(&perf->lock);
	return NULL;
}

perf->status = PERFORMING;
if(perf->type != 's')
	sem_post(&singer_join);
pthread_mutex_unlock(&perf->lock);
```
Here we lock the performer data so that no other thread can manipulate it. If the performer is already performing, then we simply restore the stage (post) and unlock the data. If the performer status is invalid, we again restore the stage acquired, and unlock the data. Only if the performer was waiting for a stage, we assign its status to be WAITING. If the performer is not a singer, we increase the number of performers that singers can join (post the singer_join semaphore) and unlock the data.   

```
int perf_time = randint(t1, t2);	

sleep(perf_time);

pthread_mutex_lock(&perf->lock);
if(perf->has_singer)
	sleep(2);
else
	sem_trywait(&singer_join);
perf->status = INVALID;

pthread_mutex_unlock(&perf->lock);
sem_post(&acoustic);

return NULL;
```
We generate a random performance time for the performer, and simply sleep that duration. Once the performance (might not be) is over, we check if a singer joined them. If a singer did join them then we sleep for 2 more seconds indicating an extended performance. Here we need to lock the data to ensure that no other thread can modify its data, especially the has_singer flag. After doing this check, we release the lock and the stage.  

#### void *wait_for_electric(void *args);

This function is an exact replica of the ```void *wait_for_acoustic(void *args);``` function. The only changes are the semaphore names, as this function is used to monitor the electric stages.  

#### void *wait_for_perf(void *args);

This function is used by singers to monitor the other performers.  

```
performer_struct *perf = (performer_struct*)args;

struct timespec ts;
clock_gettime(CLOCK_REALTIME, &ts);
	
ts.tv_sec += t;
	
int check = sem_timedwait(&singer_join, &ts);
```
Here we follow the same method as the ```wait_for_acoustic()``` function in that we wait for t seconds. However we wait on the singer_join semaphore that indicates that there are performers who don't have singers on stage.  

```
if(check != -1)
{
	...
}
```
If the wait returns successfully, we scan all the performers to find one we can join.  

```
pthread_mutex_lock(&perf->lock);
if(perf->status == PERFORMING)
{
	sem_post(&singer_join);
	pthread_mutex_unlock(&perf->lock);		
	return NULL;
}
```
Here we check if the singer is already performing. If they are, then we release the singer_join semaphore so that another singer can join the performer.  

```
for(int i = 0; i<k; i++)
{
	if(performer[i].type != 's' && performer[i].status == PERFORMING && performer[i].has_singer == 0)
	{
		int check = pthread_mutex_trylock(&performer[i].lock);
		if(check == 0 && performer[i].status == PERFORMING && performer[i].has_singer == 0)
		{
			perf->status = INVALID;
			performer[i].has_singer = 1;
			pthread_mutex_unlock(&performer[i].lock);	
			break;
		}
		else
			pthread_mutex_unlock(&performer[i].lock);
	}
}

pthread_mutex_unlock(&perf->lock);
```
Here we iterate over all the performers, and if a performer that doesn't have a singer and is currently performing exists we try and acquire its lock. If we are able to acquire its lock, we check if the performer is still performing, and if they still don't have a singer. If they do not, then we change the status of the singer and set the has_singer flag. The lock is required here to ensure that multiple singers do not try and join the same performer at the same time. If we couldn't find a performer to join, then we simply unlock the singer. This will not result in any kind of loss, as the singer still has two threads scanning the stages to perform solo. Only if all these threads fail will the singer get bored and leave. 

#### Note for readers
Here I have omitted the print statements to ensure readability and coherence.  
By 'lock the performer' I mean a performer acquires a lock for a particular performer (performer[i].lock for the i'th performer) and therefore no other performer will be able to manipulate its data.  
A wait operation is equivalent to a 'P' operation on semaphores, while the post operation is the equivalent of the 'V' operation.  

## Assumptions
The performers teleport to stages, and start performing instantly. However while leaving the stage they take 2 seconds time to receive their T-shirt (mainly to ensure that it's simple to follow the print statements).  
No two singers can perform together, singers can only perform with instrumentalists.  
Acoustic stages are always given preference over electric stages, and singers always prefer solo performances over joining other performers.  

## Justifications
Semaphores were used to represent the acoustic and electric stages as the ordering of stages doesn't matter.  
The inverted semaphore used for the singer_join condition works losslessly the following way :   
- If the performer is not a singer, it does a post operation on the singer_join semaphore. This increments its value by one making the wait function feasible.
- If a singer is able to commit a wait operation on this semaphore, there exists atleast one performer who is performing currently but does not have an accompanying singer. The singer now checks all the performers to see if they have an accompanying singer, and if they're still performing.
- If the above check fails, there is no need to post the semaphore as all the performers can't be joined anyway. 
- If the performance of the performer ends, and they weren't joined by a singer they try and perform a wait operation to indicate that their performance is invalid.   

Mutex locks were used on each performer to ensure that :  
- Only one type of stage was allocated to each performer, the other function will have to wait for the lock to edit the data, and when it tries to edit it it will find that the performer is already performing, and hence will exit.  
- A singer can either perform solo, or can join a performer. In the absence of a lock singers would be able to do both simultaneously (maybe).  

A maximum number of performers was declared explicitly to remove unnecessary memory allocations. If the user wishes to include more performers, the upper bound can easily be changed.  