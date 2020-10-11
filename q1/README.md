# Merge Sort
How does the merge sort algorithm perform when multiple processes/threads are used?
To find the answer to this question, I coded the merge sort algorithm normally, using multiple processes and multiple threads(all of them use selection sort when there are 5 or less numbers to be sorted). The timer was started right before the MergeSort() function call, and stopped right after. To generate an array of random numbers to be sorted, I used the shuf function. The functions were timed using the ```clock_gettime()``` function which returns wall clock time precise to the nanosecond.
#### To generate 1000 random numbers
```
echo 1000 > data
shuf -i 1-100000 -n 1000 >> data
```
This generates 1000 random numbers between 1 and 100000 and appends them to data from which we will read for our program.

### To run the program
To run the program we can either enter n numbers into the terminal, or simply redirect from the text file we created. To test I did the following:
```
gcc q1.c -lpthread
./a.out < data
```
Each of the following tests were run multiple times. The outputs are from one of the tests.

#### N = 1
When n = 1, there shouldn't be any process creation, and it should take roughly the same time as the normal merge sort. The threaded algorithm however must create a seperate thread to call the function that adds some overhead.
```
Normal merge sort took : 0.000003 seconds
Multiprocess merge sort took : 0.000002 seconds
Multithreaded merge sort took : 0.000354 seconds

```
We can clearly see that the normal and process merge sorts are comparable, but the threaded merge sort takes significantly more time which is a result of the process creation time.  

#### N = 10
When n = 10, there should be only 2 processes/threads created (one for left, and one for right). The time taken depends on how long process/thread creation takes.
```
Normal merge sort took : 0.000006 seconds
Multiprocess merge sort took : 0.000752 seconds
Multithreaded merge sort took : 0.001479 seconds
```
The multiprocess version performed worse than its normal counterpart due to the fact that process creation takes too much time for its benefits to be seen at this scale.
The thread version also suffers similarly, and it performs a lot worse than the process merge sort. This indicates that thread creation is a very time expensive process (atleast local to my system).

#### N = 100
```
Normal merge sort took : 0.000050 seconds
Multiprocess merge sort took : 0.006752 seconds
Multithreaded merge sort took : 0.002885 seconds
```
We see that the difference in the times taken is only growing. However at this point the multithreaded merge sort performs much better than it's multiprocess counterpart. It appears that the time overhead of creating and switching between threads/processes is not balanced by the gain in parallelism (multiple computaions running in parallel) and therefore the normal merge sort still outperforms both the concurrent merge sorts.

#### N = 1000
```
Normal merge sort took : 0.000497 seconds
Multiprocess merge sort took : 0.025717 seconds
Multithreaded merge sort took : 0.010974 seconds
```
Again we see that a similar result to the previous test. It is highly plausible that the creation of a process or thread takes more time than it does to actually carry out the functionality of the merge sort. It is also possible that requests for memory in the merge function may take time due to the multiple processes requesting the memory at the same level, while in a normal merge sort only one merge is carried out at any point in time.  

#### N = 10000
```
Normal merge sort took : 0.005702 seconds
Multiprocess merge sort took : 0.239475 seconds
Multithreaded merge sort took : 0.180805 seconds
```
Here we can see that the multithreaded merge sort is not much better than the multiprocess mergesort, and the ratio of their times has increased substantially. The number of context switches required to run all the threads and the memory requests have detracted from the computation, and hence there is a slowdown in the multithreaded merge sort.  


#### N = 100000
```
Normal merge sort took : 0.022478 seconds
fork: Resource temporarily unavailable
Multiprocess merge sort took : 1.365130 seconds
Segmentation fault (core dumped)

```
At this point the number of forks is too high, and the program displays these error messages. The program also fails to sort the array as the forking fails, and the function simply returns without sorting the array.  
The threaded merge sort results in a segmentation fault, as there is not enough memory allocated to the program to support the number of threads required and hence the array remains unsorted.  

### Conclusions
Overall sticking to the normal merge sort is the best option due to its range and decent overall performance.  
If we want a low variance in the time taken to sort any number(of the range 10<exp>5</exp>) of integers passed to our program, the multithreaded merge sort would be the optimum choice.  
The multiprocess merge sort failed to outperform either of its counterparts at any stage in testing. This indicates that the time required to spawn a new process simply is not balanced by the gain in parallel computation, and using multiple processes to sort an array is a redundancy.  
The multithreaded merge sort performed better than the multiprocess merge sort, but couldn't outperform the normal merge sort. The time taken to create the threads and to swap between them is not balanced out by the gain in parallel computing power. However a viable way to use threads would be to create a certain number of threads at inception (ideally equivalent to the number of CPU threads (virtual cores), 8 in my laptops case), and then merge the parts of the arrray sorted by those individual threads. This would ensure that the thread creation overhead would not be reflected in our run time, as we have a constant number of threads created, and the array would be sorted x times as fast where x is the number of threads. This would also ensure that our program would not need to swap between the threads (assuming your program has access to all the cores in your system). However most programs are allowed to use only one CPU thread (CPU6 in my laptops case), and this means that the creation of multiple processes and threads is completely redundant, as at any point in time only one user thread/process will be running. 
