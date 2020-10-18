# COVID simulator
Welcome the the covid simulator. This program is intended to utilize multiple threads to simulate a scenario where there are several companies creating vaccines and distributing the vaccines to zones. These zones then use the vaccines on students returning to IIIT to ensure their safety on campus.
## To run
```
gcc q2.c -lpthread
./a.out
```
These commands should be run in a terminal window(assuming unix based systems here) from the directory where the ```q2.c``` file is stored.  
## Input format
```
<number of companies> <number of zones> <number of students>
<space separated list of probabilities of success of vaccines from companies>
```
For example, 
```
1 1 1
0.5
```
Would mean that there is one company, one vaccination zone and one student. The vaccines produced by the company succeed 50% of the time.  
## Output format
```
Pharmaceutical company 0 now exists!
```
This signifies the creation of a company thread.  

```
Company 1 is preparing 5 batches of vaccines each with probability 0.000000 of succeeding!
```
This indicates that a company is trying to make a certain number of batches of a vaccine.  

```
Vaccination zone 0 has been created!
```
This signifies the creation of a zone thread.  

```
Company 1 has made 5 batches of vaccines each with probability 0.000000 of succeeding! Waiting for them to be used to restart production.
```
This signifies that the company has created vaccines, and will try to send them to zones. The company will only restart production once all the vaccines it made are consumed.  

```
Company 0 is sending vaccines to zone 5 with success probability 1.000000
```
This indicates that a company is trying to send vaccines to a zone with the probability of success mentioned.  

```
Vaccination zone 2 has received 13 vaccines from company 1
```
This indicates that the zone has received a certain number of vaccines from the company.  

```
Student 3 has arrived for vaccination attempt 1!
```
This indicates that the student is waiting to be allocated to a zone, and it also shows the vaccination attempt number.  

```
Vaccination zone 6 is ready to vaccinate 1 students
```
This indicates that the zone is ready to allow a certain number of students in for vaccination.  

```
Vaccination zone 1 now entering allocation phase
```
This indicates that the zone is now ready to allocate students for vaccination and will start vaccinating when enough students have been allocated.  

```
Student 12 is waiting for their slot!
```
This indicates that the student is waiting for their slot at one of the vaccination zones.  

```
Student 3 has been allocated a slot at zone 1, and will be vaccinated
```
This indicates that the student has been given a slot by the zone, and the vaccine is being adminstered. After the zone acknowledges the last student being vaccinated, it starts a new batch.

```
Zone 1 has finished this round of vaccination!
```
This means that either the zone ran out of vaccines, or the allowed number of students per iteration of vaccine delivery has been met.  

```
Student 57 is waiting for antibody test
```
This means that the student has been vaccinated, and is waiting to know if the vaccine was successful or not.

```
Student 12 failed to be vaccinated at zone 1 :(
```
This means that the vaccination attempt failed, and the student must come again.  

```
Student 27 has been vaccinated successfully at zone 5!
```
This means that the student has been successfully vaccinated and may attend college.  

```
Vaccination zone 8 has run out of vaccines
```
This indicates that the zone has consumed all the vaccines sent to it by the company. It may take a new batch from any company.

```
Vaccines made by company 0 have been consumed, resuming production
```
This indicates that the company must restart production, as the vaccines it produced earlier are over.  

```
Student 68 failed to be vaccinated 3 times, they can't be let into college :(
```
This means that the student failed to be vaccinated 3 times, and will not be allowed to attend college.  

```
Simulation over.
```
The end.  

## Code Breakdown
### General
```
/* Definitions used in functions */
#define MAX_NUM 1024
#define INVALID -1
#define WAITING 0
#define ASSIGNED 1
```
MAX_NUM is the upper bound on number of vaccination companies, zones and students.  
The other 3 definitions are used for students to indicate their status.  

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
typedef struct pharmaceutics
{
	int id;
	int batches;
	float prob_succ;
	int unconsumed;
}pharmaceutics;
```
This structure is for companies. It stores the id of the company, the number of batches it has currently produced, the probability of success of the vaccine it makes and the number of batches left.  

```
typedef struct vaccinnation
{
	int id;
	int num_vacc;
	int has_vaccines;
	int pharma_id;
	pthread_mutex_t lock;
	int stud_left;
	int vaccinating;
}vaccination;
```
This structure is for vaccination zones. It stores the id of the zone, the number of vaccines it currently has, a flag to indicate whether it has received vaccines or not, the id of the company it got its vaccines from and a mutex lock used to ensure that no more than one company sends vaccines to each zone. It also stores a variable to track how many students are yet to receive the vaccine, a flag to indicate whether the zone is in its vaccinating phase or not.

```
typedef struct students
{
	int id;
	int status;
	int vacc_zone_num;
	int failures;
	float succ_prob;
	pthread_mutex_t lock;
}students;
```
This structure is for students. It stores the id of the zone, the status signified by the 3 flags defined earlier, the id of the vaccination zone the student has been assigned to, the number of failed vaccination attempts, the probability that the vaccine will succeed and a lock to ensure that every student only gets vaccinated at one zone at a time.  

### Global variables
```
int n, m, o;
```
The input from the user ie the number of companies, zones and students respectively.  

```
int num_stud;
```
A copy of the number of students that is used to exit from most functions(Program ends when we have no students left to vaccinate).  

```
pthread_t pharm_id[MAX_NUM]; 
pthread_t vacc_id[MAX_NUM];
pthread_t stud_id[MAX_NUM];
```
Stores the thread IDs of the companies, zones and students. Used to join them later to terminate the program.

```
pharmaceutics pharma_comp[MAX_NUM];
vaccination vacc_zone[MAX_NUM];
students student[MAX_NUM];
```
Arrays of structures that store the data of the companies, zones and students.  
### Functions
#### void master_thread();
```
for(int i = 0; i<n; i++)
{
	pharma_comp[i].id = i;
	pthread_create(&pharm_id[i], NULL, create_company, (void*)&pharma_comp[i]);
}

for(int i = 0; i<m; i++)
{
	vacc_zone[i].id = i;
	pthread_create(&vacc_id[i], NULL, create_zone, (void*)&vacc_zone[i]);
}

for(int i = 0; i<o; i++)
{
	student[i].id = i;
	pthread_create(&stud_id[i], NULL, create_student, (void*)&student[i]);
}
```
Creates the threads for the companies, zones and students.  

```
for(int i = 0; i<n; i++)
	pthread_join(pharm_id[i], NULL);

for(int i = 0; i<m; i++)
	pthread_join(vacc_id[i], NULL);

for(int i = 0; i<o; i++)
	pthread_join(stud_id[i], NULL);
```
Waits for them to terminate and then joins them.  

```
printf(RED);
printf("Simulation over.\n");
```
Also prints the simulation over message.  
#### int randint(int lb, int ub);
```
return lb+rand()%(ub-lb+1);
```
Simply returns a random number between ub and lb both inclusive.

#### void *create_comapny(void *args);

```
	pharmaceutics *comp = (pharmaceutics*)args;
	int id = comp->id;
```
Extracts the ID of the company into a local variable. The problem with pointers is that they reference memory and can be manipulated by other threads.  

```
while(num_stud)
{
	...
}

```
The company is only operational while there are students. The ellipsis indicates the rest of the code.  
```
pharma_comp[id].batches = randint(1, 5);
pharma_comp[id].unconsumed = pharma_comp[id].batches;

```
Set the number of batches and the number of unconsumed batches to the same random number.  

```
int vacc_time = randint(2, 5);
sleep(vacc_time);
```
This is the time taken by the company to produce the vaccines. Here we emulate time taken with a sleep() which just makes the thread idle.  

```
dispatch_vaccines(id);
```
This is the function that attempts to send vaccines to the zones.  

#### void dispatch_vaccines(int id);
```
while(pharma_comp[id].batches && num_stud)
{
	...
}
```
The program runs while there are unvaccinated students, and the batches made by the company haven't been exhausted.  

```
for(int i = 0; i<m; i++)
{
	...
}

```
We iterate over all the zones, checking if they haven't received a batch of vaccines yet.  

```
if(vacc_zone[i].has_vaccines == 0)
{
	int check_lock = pthread_mutex_trylock(&vacc_zone[i].lock);
	if(check_lock == 0)
	{
		if(vacc_zone[i].has_vaccines == 0)
		{
			...
		}
	}
	else
    		pthread_mutex_unlock(&vacc_zone[i].lock);
}
```
Here we check if the zone hasn't been given a vaccine. To assure that only one company tries to send the zone vaccines we try to acquire its lock. We recheck the value of the flag in case it has been manipulated. If it is free, then we go ahead with our function. If not we free the lock.  

```
vacc_zone[i].pharma_id = id;
vacc_zone[i].has_vaccines = 1;
pthread_mutex_unlock(&vacc_zone[i].lock);
pharma_comp[id].batches -= 1;
```
If we can access the zone (ie it was free), update its company id, and set the flag to true. Also unlock the zone so that it can carry out its functionality, and then decrement the number of available batches by 1.

```
if(pharma_comp[id].batches == 0)
{
	while(pharma_comp[id].unconsumed && num_stud)
		sleep(1);

	if(num_stud == 0)
		return;
}
```
If all the batches have been dispatched, wait for them to be consumed or for the number of students to reach 0. If there are no students left, return.  

#### void *create_zone(void *args);

```
vaccination *temp = (vaccination*)args;
int id = temp->id;
```
Extract the id of the zone from the argument passed.  

```
while(num_stud)
{
	...
}
```
Again we make sure that we execute the program only when there are students left to be vaccinated.  

```
while(vacc_zone[id].has_vaccines == 0 && num_stud)
	sleep(1);
if(num_stud == 0)
	return NULL;
while(pthread_mutex_trylock(&vacc_zone[id].lock))
	sleep(1);
```
Wait for the zone to acquire vaccines, or for the number of students left to vaccinate become 0. If there are no students left, join the thread. If there are students left, wait till the lock to the zone is acquired ensuring that no company can send vaccines.  

```
vacc_zone[id].num_vacc = randint(10, 20);
```
Assign a random number of vaccines that this zone has acquired from the company.  

```
while(num_stud && vacc_zone[id].num_vacc)
{
	sleep(1);
	vaccinate_students(id);
}
```
While there are still students left to be vaccinated and vaccines left in this zone, try to vaccinate students. The sleep here is just to ensure that the vaccination restarts after a slight delay for the next round.  

```
if(vacc_zone[id].num_vacc == 0)
{
	pharma_comp[vacc_zone[id].pharma_id].unconsumed -= 1;	
}

vacc_zone[id].has_vaccines = 0;

pthread_mutex_unlock(&vacc_zone[id].lock);
```
If there are no vaccines left in this zone, then indicate that the batch has been consumed to the company. Also set the flag back to false, and unlock the thread so that a company can send vaccines again.  

#### void *create_student(void *args);

```
students *temp = (students*)args;
int id = temp->id;
```
Again we extract the id of the student to reduce instruction length.  

```
while(1)
{
	...
}
```
Here we run the code infinitely to ensure that every student is either successfully vaccinated or has 3 failed vaccination attempts.  

```
int delay = randint(2, 5);
sleep(delay);

```
Ensure that the students are slightly late, as students never arrive on time.  

```
student[id].status = WAITING;

while(student[id].status == WAITING)
	sleep(1); 

while(vacc_zone[student[id].vacc_zone_num].vaccinating == 0)
	sleep(1);

while(pthread_mutex_trylock(&student[id].lock))
	sleep(1);
```
Change the status of the student, so that the zone can invite the student. Once the student has been allocated to a zone, wait for the zone to start vaccinating which is reflected in the change in the flag variable. When the zone starts vaccinating, try and acquire the lock to the student to ensure that no zone can try and allocate the student.  

```
vacc_zone[student[id].vacc_zone_num].stud_left -= 1;
```
Here we decrement the number of students left to be vaccinated after the student has been vaccinated at the zone. This helps ensure that once all students allocated to a zone have been vaccinated, we can start a new round of vaccination at the zone. 

```
float check = ((float)randint(0, 10000000))/10000000;

if(check <= student[id].succ_prob)
{
	num_stud -= 1;

	student[id].status = INVALID;
	return NULL;
}

student[id].failures += 1;

if(student[id].failures == 3)
{
	num_stud -= 1;

	// Invalidate the student
	student[id].status = INVALID;
	return NULL;
}

pthread_mutex_unlock(&student[id].lock);
```
Here we do a simple probability calculation by generating a random number and dividing it by the upper bound of the number. If this generated float in the range [0, 1] is less than or equal to the probability of the success of the vaccine, we treat it as a success. If the vaccination is successful, invalidate this student and decrease the number of unvaccinated students by 1. If the vaccination fails, increase the number of failures of this student by 1. If the number of failures has reached 3 then invalidate the student and decrease number of pending students by 1. Now unlock the student thread so that the zones can try and invite them again.  

#### void vaccinate_students(int id);
```
while(num_stud)
{
	...
}
```
Here again we only execute when there are students left to be vaccinated.  

```
if(vacc_zone[id].num_vacc == 0)
	return;
```
If the zone has run out of vaccines, then we return and allow the companies to send vaccines to us.  

```
int allowed_stud = randint(1, 8);
if(allowed_stud > vacc_zone[id].num_vacc)
	allowed_stud = vacc_zone[id].num_vacc;
vacc_zone[id].stud_left = 0;
```
Randomly generate the number of students that are allowed to enter the vaccination zone at any point in time. If this number is greater than the number of vaccines left in the zone, truncate it. We also set the number of students left to be vaccinated to be 0, as no students have been allocated yet

```
while(num_stud && vacc_zone[id].stud_left == 0)
{
	for(int i = 0; i<o; i++)
	{
		if(allowed_stud == 0 || num_stud == 0)
					break;

		if(student[i].status == WAITING)
		{
		    int check_lock = pthread_mutex_trylock(&student[i].lock);

			if(check_lock == 0)
			{
		    		if(student[i].status == WAITING)
				{
					...
				}
			}
		        else
		            pthread_mutex_unlock(&student[i].lock);
	      	}
	 }
}
```
While there are still students pending, and our zone has space for them, iterate through the student pool and try to select one. If the student is waiting, try and acquire their lock. If we can, recheck the status to ensure that it hasn't changed and then carry on. If the status has changed the unlock the thread and try and acquire another student. If there are no more students, or our zone has met its capacity, break from the while loop. Here we greedily try to select students. The zone will make passes over the students until it gets one student. It does not wait to check if any students have failed their previous vaccination as it assumes that they have already been allocated (at that instant in time) to another zone. This also prevents deadlocks from happening, as zones only wait till they get one student or there are no students left to vaccinate.  

```
student[i].vacc_zone_num = id;
student[i].succ_prob = pharma_comp[vacc_zone[id].pharma_id].prob_succ;
student[i].status = ASSIGNED;
pthread_mutex_unlock(&student[i].lock);
```
Assign the student the vaccination zone id of the zone it has been assigned to. Also assign the probability of success from the company probability of success. Then change the status of the student to ensure that it can go ahead with its operation. Unlock the student thread.  

```
vacc_zone[id].num_vacc -= 1;
allowed_stud -= 1;
vacc_zone[id].stud_left += 1;
```
Decrease the number of vaccines left in the zone, and also change the number of students that will now be allowed into the zone for this run. Increments the number of students left to be vaccinated in the zone by one.  

```
<outside while>
if(num_stud == 0)
			return;
```
If there are no more students left, return.  

```
vacc_zone[id].vaccinating = 1;

while(vacc_zone[id].stud_left && num_stud)
	sleep(1);

vacc_zone[id].vaccinating = 0;
``` 
If there are students left to vaccinate, then start vaccinating the students currently allocated to this zone. Wait till all the students have been vaccinated, and then reset the zone for the next run.


#### Note for readers
Here I have omitted the print statements to ensure readability and coherence.  
By 'lock the thread' I mean the thread acquires the lock the the referred thread and ensures that no other thread can manipulate its data until it has released the lock. The company 'locks' the zone thread means that the company has acquired the lock to a particular zone, inhibiting other companies from sending vaccines to that zone. It also prevents the zone from carrying on its train of execution until the company is done manipulating its data and releases the thread.   

## Assumptions 
The antibody tests are dispatched to students via email after they receive the vaccine to ensure that the maximum number of students can be vaccinated in any interval of time. The vaccination zones will try and find new students to find after vaccinating the last student in the batch, while the student awaits their antibody test.  
The companies create a random number of batches only when the batches they made previously have been consumed. A similar mechanism could've been implemented for the zone-student relation but it felt unneccessary to make the zones wait until the students received their antibody test results.  
Wait times for students are only 1 second between waiting for antibody test and getting their result to indicate superior testing capability.  
The students are capable of teleporting to the testing zone once the zone opens up for vaccination.  

## Justifications
Although locks are not necessarily required in most of the code, I chose to use them out of simplicity. I also placed sleep(1) statements in all the busy waiting loops, to minimize the CPU consumption of each thread. This doesn't affect anything because after the lock is released, no other thread can claim the lock due to the flag.  
The busy waiting is to ensure smooth flow of the program, and could've been avoided with conditional lock and signal. However the busy wait is much more intuitive and readable.  
The lock for the vaccination zone stud_left is not required because we are only decrementing/incrementing in any given phase of the program. However to highlight that it is a critical section in the program, locks were used to signify their criticality.  
Zones try and allocate students until they get atleast one student. This was done in an attempt to make the vaccination efficient as otherwise the zones would keep waiting until some student failed their previous test. Although this does lead to more vaccination rounds, overall the vaccination should be faster.  
The ```sleep(1)``` statements in all the busy wait while() loops do not affect performance, as they will reach their exit clause within one second. This effect is nominal on the program, as the program itself takes quite some time to run. It is also present to reduce the CPU consumption of the threads, as we check the condition only once every second instead of very frequently. 