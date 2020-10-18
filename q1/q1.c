#define _POSIX_C_SOURCE 199309L

#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<time.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>


#define SUCC 0
#define FAIL 1
#define NUM_THREADS 2
int num_perT;
int offset;

typedef struct ForT
{
	int l, r;
	int * arr;
}Array;


void SelSort(int *arr, int l, int r)
{
	for(int i = l; i<=r; i++)
	{
		int min_ind = i;
		for(int j = i+1; j<=r; j++)
		{
			if(arr[j] < arr[min_ind])
				min_ind = j;
		}
		int temp = arr[min_ind];
		arr[min_ind] = arr[i];
		arr[i] = temp;
	}
}

void Merge(int *arr, int l, int m, int r)
{
	int i, j, k;
	int nl = m-l+1;
	int nr = r-m;

	int *left = (int*)malloc(nl*sizeof(int));
	int *right = (int*)malloc(nr*sizeof(int));

	for(i = 0; i<nl; i++)
		left[i] = arr[l+i];
	for(i = 0; i<nr; i++)
		right[i] = arr[m+1+i];

	i = 0;
	j = 0;
	k = l;

	while(i<nl && j<nr)
	{
		if(left[i] < right[j])
			arr[k++] = left[i++];
		else
			arr[k++] = right[j++];
	}
	while(i<nl)
		arr[k++] = left[i++];
	while(j<nr)
		arr[k++] = right[j++];

	free(left);
	free(right);
}

void MergeSort(int *arr, int l, int r)
{
	if(r-l+1 < 5)
	{
		SelSort(arr, l, r);
		return;
	}

	if(l >= r)
		return;
	int m = l + (r-l)/2;
	MergeSort(arr, l, m);
	MergeSort(arr, m+1, r);
	Merge(arr, l, m, r);
}


void MergeSortP(int *arr, int l, int r)
{
	if(r-l+1 <= 5)
	{
		SelSort(arr, l, r);
		return;
	}

	pid_t l_child, r_child;
	int m = l + (r-l)/2;

	l_child = fork();

	if(l_child < 0)
	{
		perror("fork");
		exit(FAIL);
	}

	else if(l_child == 0)
	{
		MergeSortP(arr, l, m);
		exit(SUCC);
	}

	else
	{
		r_child = fork();
		if(r_child < 0)
		{
			perror("fork");
			exit(FAIL);
		}
		else if(r_child == 0)
		{
			MergeSortP(arr, m+1, r);
			exit(SUCC);
		}
	}

	waitpid(l_child, NULL, 0);
	waitpid(r_child, NULL, 0);

	Merge(arr, l, m, r);
}

void *MergeSortT(void* a)
{
	Array *args = (Array*) a;

	int l = args->l;
	int r = args->r;
	int *arr = args->arr;

	if(l > r)
		return NULL;
	if(r-l+1 < 5)
	{
		SelSort(arr, l, r);
		return NULL;
	}
	
	int m = l + (r-l)/2;
	Array al;
	al.l = l;
	al.r = m;
	al.arr = arr;
	pthread_t tidl;
	pthread_create(&tidl, NULL, MergeSortT, &al);

	Array ar;
	ar.l = m+1;
	ar.r = r;
	ar.arr = arr;
	pthread_t tidr;
	pthread_create(&tidr, NULL, MergeSortT, &ar);

	pthread_join(tidl, NULL);
	pthread_join(tidr, NULL);

	Merge(arr, l, m, r);
	return NULL;
}


int main()
{
	struct timespec ts;

	int n;
	scanf("%d", &n);
	int *arr = (int*)malloc(n*sizeof(int));
	int *temparr = (int*)malloc(n*sizeof(int));

	clock_t timer;
	long double time_taken;

	for(int i = 0; i<n; i++)
		scanf("%d", &arr[i]);

	// For the normal mergesort
	for(int i = 0; i<n; i++)
		temparr[i] = arr[i];

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double start_ts = ts.tv_nsec/(1e9) + ts.tv_sec;
	
	MergeSort(temparr, 0, n-1);

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double end_ts = ts.tv_nsec/(1e9) + ts.tv_sec;
    time_taken = end_ts - start_ts;

	if(time_taken < 0)
		time_taken *= -1;

	for(int i = 0; i<n; i++)
		printf("%d ", temparr[i]);
	printf("\n");

	printf("Normal merge sort took : %Lf seconds\n", time_taken);

	// For the child process merge sort,
	// We need to create shared memory

	int shared_id;
	key_t key = IPC_PRIVATE;
	int *shared_array;

	size_t shared_size = n*sizeof(int);

	if((shared_id = shmget(key, shared_size, IPC_CREAT | 0666)) < 0)
	{
		perror("shared memory");
		exit(FAIL);
	}

	if((shared_array = (int*)shmat(shared_id, NULL, 0)) == (int*)-1)
	{
		perror("shared memory");
		exit(FAIL);
	}

	for(int i = 0; i<n; i++)
		shared_array[i] = arr[i];

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    start_ts = ts.tv_nsec/(1e9) + ts.tv_sec;

	MergeSortP(shared_array, 0, n-1);
	
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    end_ts = ts.tv_nsec/(1e9) + ts.tv_sec;
    time_taken = end_ts - start_ts;

	if(time_taken < 0)
		time_taken *= -1;

	for(int i = 0; i<n; i++)
		printf("%d ", shared_array[i]);
	printf("\n");

	printf("Multiprocess merge sort took : %Lf seconds\n", time_taken);

	// For multithreaded quicksort we use a seperate thread to call mergesort
	pthread_t tid;
	Array a;
	a.l = 0;
	a.r = n-1;

	for(int i = 0; i<n; i++)
		temparr[i] = arr[i];

	a.arr = temparr;

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    start_ts = ts.tv_nsec/(1e9) + ts.tv_sec;

	pthread_create(&tid, NULL, MergeSortT, &a);
	pthread_join(tid, NULL);

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    end_ts = ts.tv_nsec/(1e9) + ts.tv_sec;
    time_taken = end_ts - start_ts;

	if(time_taken < 0)
		time_taken *= -1;

	for(int i = 0; i<n; i++)
		printf("%d ", temparr[i]);
	printf("\n");

	printf("Multithreaded merge sort took : %Lf seconds\n", time_taken);
}
