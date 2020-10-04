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

#define SUCC 0
#define FAIL 1
#define NUM_THREADS 2
int num_perT;
int offset;

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
	if(l >= r)
		return;
	int m = l + (r-l)/2;
	MergeSort(arr, l, m);
	MergeSort(arr, m+1, r);
	Merge(arr, l, m, r);
}

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


int main()
{
	int n;
	scanf("%d", &n);
	int *arr = (int*)malloc(n*sizeof(int));
	int *temparr = (int*)malloc(n*sizeof(int));

	clock_t timer;
	double time_taken;

	for(int i = 0; i<n; i++)
		scanf("%d", &arr[i]);

	// For the normal mergesort
	for(int i = 0; i<n; i++)
		temparr[i] = arr[i];

	timer = clock();
	MergeSort(temparr, 0, n-1);
	timer = timer - clock();

	time_taken = (double)timer/CLOCKS_PER_SEC;
	if(time_taken < 0)
		time_taken *= -1;

	printf("Normal merge sort took : %f seconds\n", time_taken);
	for(int i = 0; i<n; i++)
		printf("%d ", temparr[i]);
	printf("\n");
	

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

	timer = clock();
	MergeSortP(shared_array, 0, n-1);
	timer = timer - clock();

	time_taken = ((double)timer)/CLOCKS_PER_SEC;
	if(time_taken < 0)
		time_taken *= -1;

	printf("Multiprocess merge sort took : %f seconds\n", time_taken);
	for(int i = 0; i<n; i++)
		printf("%d ", shared_array[i]);
	printf("\n");
}