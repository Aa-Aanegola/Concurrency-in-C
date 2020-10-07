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


/* Definitions used in functions */
#define MAX_NUM 1024
#define INVALID -1
#define WAITING 0
#define ASSIGNED 1


/* Colors used for formatting output */
#define BASE "\033[0m"
#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define CYAN "\033[1;36m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[01;33m"
#define MAGENTA "\033[1;35m"


/* Structure definitions */
typedef struct pharmaceutics
{
	// Stores its id
	int id;
	// Stores number of batches made
	int batches;
	// Probability of success of the vaccines
	float prob_succ;
	// Stores number of batches unconsumed
	int unconsumed;
}pharmaceutics;

typedef struct vaccinnation
{
	// Stored the id of the zone
	int id;
	// Stores number of vaccines it gets 
	int num_vacc;
	// Stores 0 if it has no vaccines, and 1 if it has vaccines
	int has_vaccines;
	// Stored the id of the pharma company it got vaccines from
	int pharma_id;
	// Mutex to ensure that each zone receives only one batch of vaccinations
	pthread_mutex_t lock;
}vaccination;

typedef struct students
{
	// Stores the id of the student
	int id;
	// Status INVALID - not arrived, WAITING - arrived and waiting, ASSIGNED - slot assigned
	int status;
	// Vaccination zone number of the student
	int vacc_zone_num;
	// Number of failed vaccination attempts
	int failures;
	// Probability of success of vaccine adminstered to student
	float succ_prob;
	// Mutex to ensure that one student is only picked at one vacc_zone
	pthread_mutex_t lock;
}students;


/* Global variables */
// Number of companies, vaccination zones and students
int n, m, o;

// Number of students
int num_stud;

// Arrays of thread IDs for pharma companies, vaccination zones and students
pthread_t pharm_id[MAX_NUM]; 
pthread_t vacc_id[MAX_NUM];
pthread_t stud_id[MAX_NUM];

// Arrays to store the data of pharma comapanies, vacc zones and students
pharmaceutics pharma_comp[MAX_NUM];
vaccination vacc_zone[MAX_NUM];
students student[MAX_NUM];


/* Function Declarations */
// Function that returns a random number between ub and lb both inclusive
int randint(int lb, int ub);

// Function that the threads created for companies call
void *create_company(void *args);

// Function that the threads created for zones call
void *create_zone(void *args);

// Function that the threads created for students call
void *create_student(void *args);

// Function to send all batches of a vaccine to a zone
void dispatch_vaccines(int id);

// Function to assign students to vaccination zones
void vaccinate_students(int id);

// Function to create all the threads
void master_thread();


// Main 
int main()
{
	// Setting up the seed for rand function
	srand(time(0));

	// Taking input from user
	scanf("%d %d %d", &n, &m, &o);

	// Error handling
	if(n < 0 || m < 0 || o < 0)
	{
		printf(RED);
		printf("IIIT students kindly request you to not try and mess with the simulation :)\n");
		printf("Simulation over.\n");
		return 0;
	}

	if(n > MAX_NUM || m > MAX_NUM || o > MAX_NUM)
	{
		printf(RED);
		printf("Limit on companies, zones and students is 1024. \nExiting...\n");
		printf("Simulation over.\n");
		return 0;
	}

	if(n == 0 || m == 0)
	{
		printf(RED);
		printf("There must be atleast 1 company and 1 zone. All students at IIIT now hate you :(\n");
		printf("Simulation over.\n");
		return 0;
	}

	if(o == 0)
	{
		printf(RED);
		printf("Did all the students at IIIT disappear?\n");
		printf("Simulation over.\n");
		return 0;
	}

	// Setting up the simulation
	num_stud = o;

	// Inputting the success rates of the vaccines
	for(int i = 0; i<n; i++)
		scanf("%f", &pharma_comp[i].prob_succ);

	// Initializing the structure arrays with their necessary values to indicate status
	for(int i = 0; i<n; i++)
		pharma_comp[i].batches = 0;

	for(int i = 0; i<m; i++)
		vacc_zone[i].has_vaccines = 0;

	for(int i = 0; i<o; i++)
		student[i].status = INVALID;

	// Creation of individual threads
	master_thread();
}


int randint(int lb, int ub)
{
	return lb+rand()%(ub-lb+1);
}


void master_thread()
{
	// Creation of the pharma companies
	for(int i = 0; i<n; i++)
	{
		pharma_comp[i].id = i;
		pthread_create(&pharm_id[i], NULL, create_company, (void*)&pharma_comp[i]);
	}

	// Creation of the vaccination zones
	for(int i = 0; i<m; i++)
	{
		vacc_zone[i].id = i;
		pthread_create(&vacc_id[i], NULL, create_zone, (void*)&vacc_zone[i]);
	}

	// Creation of the students
	for(int i = 0; i<o; i++)
	{
		student[i].id = i;
		pthread_create(&stud_id[i], NULL, create_student, (void*)&student[i]);
	}

	// Waiting for all the threads to quit so we can end the program 
	for(int i = 0; i<n; i++)
		pthread_join(pharm_id[i], NULL);

	for(int i = 0; i<m; i++)
		pthread_join(vacc_id[i], NULL);

	for(int i = 0; i<o; i++)
		pthread_join(stud_id[i], NULL);

	printf(RED);
	printf("Simulation over.\n");
}


void *create_company(void *args)
{
	// Get the pharma company number
	pharmaceutics *comp = (pharmaceutics*)args;
	int id = comp->id;

	// Display message showing thread creation
	fflush(NULL);
	printf(GREEN);
   	printf("Pharmaceutical company %d now exists!\n", id);
	fflush(NULL);

	// While there are still students don't return
	while(num_stud)
	{
		// Random number of batches created
		pharma_comp[id].batches = randint(1, 5);
		pharma_comp[id].unconsumed = pharma_comp[id].batches;

		// Display message 
		fflush(NULL);
		printf(MAGENTA);
		printf("Company %d is preparing %d batches of vaccines each with probability %f of succeeding!\n", id, pharma_comp[id].batches, pharma_comp[id].prob_succ);
		fflush(NULL);

		// Generate random number indicating vaccine production
		int vacc_time = randint(2, 5);
		sleep(vacc_time);

		// Display ready message
		fflush(NULL);
		printf(YELLOW);
		printf("Company %d has made %d batches of vaccines each with probability %f of succeeding! Waiting for them to be used to restart production.\n", id, pharma_comp[id].batches, pharma_comp[id].prob_succ);
		fflush(NULL);

		// Send the vaccines to the zones
		dispatch_vaccines(id);
	}	

	return NULL;
}

void dispatch_vaccines(int id)
{
	// While the batches haven't been dispatched and students still exist
	while(pharma_comp[id].batches && num_stud)
	{
		// Iterate over all the valid zones
		for(int i = 0; i<m; i++)
		{
			// Check if any vaccination zone is free
			if(vacc_zone[i].has_vaccines == 0)
			{
				// If it is check if its locked
				int check_lock = pthread_mutex_trylock(&vacc_zone[i].lock);
				if(check_lock == 0)
				{
					// If its not locked recheck flag in case another company sent
					if(vacc_zone[i].has_vaccines == 0)
					{
						// Display message
						fflush(NULL);
						printf(BLUE);
						printf("Company %d is sending vaccines to zone %d with success probability %f\n", id, i, pharma_comp[id].prob_succ);
						fflush(NULL);

						// Set the pharma id of the vaccination zone
						vacc_zone[i].pharma_id = id;
						// Set the flag to true so the zone can execute
						vacc_zone[i].has_vaccines = 1;
						//Decrement the counter
						
						// Unlock as the vacc_zone can't unlock
						pthread_mutex_unlock(&vacc_zone[i].lock);

						// Decrement the number of batches left
						pharma_comp[id].batches -= 1;
						
						// Return if there are no more batches
						if(pharma_comp[id].batches == 0)
						{
							// CHeck if all the batches are consumed or if there are no students left
							while(pharma_comp[id].unconsumed && num_stud);
								
							// If all the students have been taken care of return
							if(num_stud == 0)
								return;

							// Otherwise make vaccines again
							fflush(NULL);
							printf(RED);
							printf("Vaccines made by company %d have been consumed, resuming production\n", id);
							fflush(NULL);
							return;
						}
					}
					else
						pthread_mutex_unlock(&vacc_zone[i].lock);
				}
			}
		}
	}
}


void *create_zone(void *args)
{
	// Get the id of the zone
	vaccination *temp = (vaccination*)args;
	int id = temp->id;
	
	// Display message
	fflush(NULL);
	printf(GREEN);
	printf("Vaccination zone %d has been created!\n", id);
	fflush(NULL);

	// Exit when there are no more students
	while(num_stud)
	{
		// Wait until vaccines are received
		while(vacc_zone[id].has_vaccines == 0 && num_stud)
			sleep(1);

		// If there are no students left, join the thread
		if(num_stud == 0)
			return NULL;

		// Otherwise try and acquire the lock
		while(pthread_mutex_trylock(&vacc_zone[id].lock))
			sleep(1);

		// Assign the number of vaccines sent 
		vacc_zone[id].num_vacc = randint(10, 20);

		// Pring message 
		fflush(NULL);
		printf(BLUE);
		printf("Vaccination zone %d has received %d vaccines from company %d\n", id, vacc_zone[id].num_vacc, vacc_zone[id].pharma_id);
		fflush(NULL);

		// Small delay
		sleep(1);

		// While we still have vaccines in this zone, proceed to vaccinate
		while(num_stud && vacc_zone[id].num_vacc)
		{
			// Display message
			fflush(NULL);
			printf(BLUE);
			printf("Vaccination zone %d now entering vaccination phase\n", id);
			fflush(NULL);

			// Small delay
			sleep(1);

			// Start vaccinating students
			vaccinate_students(id);
		}

		// If there are no vaccines left, display message and change number of unconsumed batches in company
		if(vacc_zone[id].num_vacc == 0)
		{
			fflush(NULL);
			printf(RED);
			printf("Vaccination zone %d has run out of vaccines\n", id);
			fflush(NULL);

			pharma_comp[vacc_zone[id].pharma_id].unconsumed -= 1;	
		}

		// The zone doesn't have vaccines anymore
		vacc_zone[id].has_vaccines = 0;

		// Unlock this zone so it can receive vaccines again
		pthread_mutex_unlock(&vacc_zone[id].lock);
	}

	return NULL;
}


void *create_student(void *args)
{
	// Extract data from the arguments
	students *temp = (students*)args;
	int id = temp->id;

	// Infinte loop to ensure that the student is always taken care of
	while(1)
	{
		// IIIT students don't come on time :(
		int delay = randint(2, 5);
		sleep(delay);

		// Display message
		fflush(NULL);
		printf(BLUE);
		printf("Student %d has arrived for their vaccination attempt %d!\n", id, student[id].failures+1);
		fflush(NULL);	

		// Small delay
		sleep(1);

		// Display next message
		fflush(NULL);
		printf(BLUE);
		printf("Student %d is waiting for their slot!\n", id);
		fflush(NULL);	

		// Set the status to waiting so the zone can allow student to visit 
		student[id].status = WAITING;

		// Wait till the student gets allocated
		while(student[id].status == WAITING)
			sleep(1);

		// Wait till the student acquires its own lock
		while(pthread_mutex_trylock(&student[id].lock))
			sleep(1);

		// Small delay
		sleep(1);

		// Display message
		fflush(NULL);
		printf(YELLOW);
		printf("Student %d has been vaccinated and is waiting for antibody test\n", id);
		fflush(NULL);
		sleep(1);

		// Math to compute probability
		float check = ((float)randint(0, 10000000))/10000000;
		
		// If vaccine succeeded, then join thread
		if(check <= student[id].succ_prob)
		{
			// Decrement student counter
			num_stud -= 1;

			// Display message
			fflush(NULL);
			printf(GREEN);
			printf("Student %d has been vaccinated successfully at zone %d!\n", id, student[id].vacc_zone_num);
			fflush(NULL);

			// Student should no longer be in waitinf state
			student[id].status = INVALID;
			return NULL;
		}

		// Otherwise the vaccination attempt failed
		student[id].failures += 1;

		// Display message
		fflush(NULL);
		printf(RED);
		printf("Student %d failed to be vaccinated at zone %d :(\n", id, student[id].vacc_zone_num);
		fflush(NULL);

		// If the student has been vaccinated 3 times, and has failed all three times join thread
		if(student[id].failures == 3)
		{
			// Decrement number of students
			num_stud -= 1;
			fflush(NULL);
			printf(RED);
			printf("Student %d failed to be vaccinated 3 times, they can't be let into college :(\n", id);
			fflush(NULL);

			// Invalidate the student
			student[id].status = INVALID;
			return NULL;
		}

		// Unlock the thread so that the zones can invite the student
		pthread_mutex_unlock(&student[id].lock);
	}
	return NULL;
}


void vaccinate_students(int id)
{
	// While there are still students to be vaccinated
	while(num_stud)
	{
		// Set the number of students allowed to be vaccinated
		int allowed_stud = randint(1, 8);
		if(allowed_stud > vacc_zone[id].num_vacc)
			allowed_stud = vacc_zone[id].num_vacc;

		// Display message
		fflush(NULL);
		printf(YELLOW);
		printf("Vaccination zone %d is ready to vaccinate %d students\n", id, allowed_stud);
		fflush(NULL);

		// While there are still students to be vaccinated and the zone still has space for students
		while(num_stud && allowed_stud)
		{
			// Check all the students
			for(int i = 0; i<o; i++)
			{
				// If they are waiting
				if(student[i].status == WAITING)
				{
					// Try to acquire their lock
					int check_lock = pthread_mutex_trylock(&student[i].lock);

					// If it is possible
					if(check_lock == 0)
					{
						// Check status again in case student was invalidated
						if(student[i].status == WAITING)
						{
							// Display message
							fflush(NULL);
							printf(MAGENTA);
							printf("Student %d has been allocated a slot at zone %d, waiting to be vaccinated\n", i, id);
							fflush(NULL);

							// Change the data of the student accordingly
							student[i].vacc_zone_num = id;
							student[i].succ_prob = pharma_comp[vacc_zone[id].pharma_id].prob_succ;
							student[i].status = ASSIGNED;

							// Unlock the student
							pthread_mutex_unlock(&student[i].lock);

							// Change the number of vaccines and slots
							vacc_zone[id].num_vacc -= 1;
							allowed_stud -= 1;

							// If this round is over, or there are no students left return
							if(allowed_stud == 0 || num_stud == 0)
							{
								fflush(NULL);
								printf(RED);
								printf("Zone %d has finished this round of vaccination!\n", id);
								//printf(BASE);
								fflush(NULL);
								return;
							}
						}
						else
							pthread_mutex_unlock(&student[i].lock);
					}
				}
			}
		}
	}
}