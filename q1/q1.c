#include<stdio.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>

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

void MergeP(int *arr, int l, int m, int r)
{

}

void MergeSortP(int *arr, int l, int r)
{

}

int main()
{
	int n;
	scanf("%d", &n);
	int *arr = (int*)malloc(n*sizeof(int));
	int *temparr = (int*)malloc(n*sizeof(int));

	for(int i = 0; i<n; i++)
		scanf("%d", &arr[i]);

	for(int i = 0; i<n; i++)
		temparr[i] = arr[i];

	MergeSort(temparr, 0, n-1);
	for(int i = 0; i<n; i++)
		printf("%d ", temparr[i]);
}