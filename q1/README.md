# Merge Sort
How does the merge sort algorithm perform when multiple processes are used?
To find the answer to this question, I coded the merge sort algorithm both normally and using multiple processes (the multiprocess mergesort uses selection sort when there are 5 or less numbers to be sorted to prevent unnecessary process creation). The timer was started right before the MergeSort() function call, and stopped right after. To generate random numbers to be sorted, I used the shuf function. The timer was created using the ```clock()``` function from the ```time.h``` library.
#### To generate 1000 random numbers
```
echo 1000 > data
shuf -i 1-100000 -n 1000 >> data
```
This generates 1000 random numbers between 1 and 10000 and appends them to data from which we will read for our program.

### To run the program
To run the program we can either enter n numbers into the terminal, or simply redirect from the text file we created. To test I did the following:
```
./a.out < data
```
Each of the following tests were run multiple times. The outputs are from one of the tests.

#### N = 1
When n = 1, there shouldn't be any process creation, and both the algorithms should take approximately the same amount of time. The threaded algorithm however must create a seperate thread to call the function that adds some overhead.
```
Normal merge sort took : 0.000004 seconds
Multiprocess merge sort took : 0.000002 seconds
Multithreaded merge sort took : 0.000238 seconds
```
We can clearly see that the normal and process merge sorts are comparable, but the threaded merge sort takes significantly more time

#### N = 10
When n = 10, there should be only 2 processes/threads created (one for left, and one for right). The time taken depends on how long process/thread creation takes.
```
Normal merge sort took : 0.000007 seconds
Multiprocess merge sort took : 0.000373 seconds
Multithreaded merge sort took : 0.001865 seconds
```
The multiprocess version performed worse than its normal counterpart ie the process creation takes too much time for its benefits to be seen at this scale.
The thread version also suffers similarly, and it performs a lot worse than the process merge sort. This indicates that thread creation is a very time expensive process.

#### N = 100
```
Normal merge sort took : 0.000047 seconds
Multiprocess merge sort took : 0.000334 seconds
Multithreaded merge sort took : 0.005699 seconds
```
We see that the difference is slowly being bridged, and that the increment in the time taken by the process merge sort is similar to that of the normal merge sort. Hence we can conclude that the time taken by the process merge sort is mainly due to process creation.  
However we see that the threaded merge sort is steadily growing in its time taken. Adding multiple threads at each level seems to have no noticeable benefit.

#### N = 1000
```
Normal merge sort took : 0.000497 seconds
Multiprocess merge sort took : 0.000540 seconds
Multithreaded merge sort took : 0.037252 seconds
```
Here we see that the multiprocess merge sort outperforms the normal merge sort (marginally). The gain in the speed finally balanced the process creation time.
The threaded merge sort is much slower than both couterparts. We can conclude that thread creation at every level does not speed up our merge sort algorithm.

#### N = 10000
```
Normal merge sort took : 0.005739 seconds
Multiprocess merge sort took : 0.000358 seconds
Multithreaded merge sort took : 0.616806 seconds
```
Here we can see that the gain from using multiple processes finally has a significant advantage over its normal counterpart.  
We also observe that the threaded merge sort has a massive time.

#### N = 100000
```
Normal merge sort took : 0.011852 seconds
fork: Resource temporarily unavailable
...
Multiprocess merge sort took : 0.000920 seconds
Segmentation fault (core dumped)
```
At this point the number of forks is too high, and the program displays these error messages. The program also fails to sort the array as the forking fails.
The threaded merge sort also results in a segmentation fault here, as there is not enough memory allocated to the program to support the number of threads it requires.

### Conclusions
Overall sticking to the normal merge sort is the best option due to its range and decent overall performance.  
If we want a low variance in the time taken to sort any number(of the range 1o<exp>5</exp>) of integers passed to our program, the process merge sort would be the optimum choice.  
The threaded merge sort failed to outperform either of its counterparts. However a viable way to use threads would be to create a certain number of threads at inception, and then merge the parts of the arrray sorted by those individual threads. This would ensure that the thread creation overhead would not be reflected in our run time, as we have a constant number of threads created, and the array would be sorted x times as fast where x is the number of threads.