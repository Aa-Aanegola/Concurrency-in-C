# Merge Sort
How does the merge sort algorithm perform when multiple processes are used?
To find the answer to this question, I coded the merge sort algorithm both normally and using multiple processes (the multiprocess mergesort uses selection sort when there are 5 or less numbers to be sorted to prevent unnecessary process creation). The timer was started right before the MergeSort() function call, and stopped right after. To generate random numbers to be sorted, I used the shuf function. The timer was created using the ```clock()``` function from the ```time.h``` library.
#### To generate 1000 random numbers
```
echo 1000 > data
shuf -i 1-10000 -n 1000 >> data
```
This generates 1000 random numbers between 1 and 10000 and appends them to data from which we will read for our program.

### To run the program
To run the program we can either enter n numbers into the terminal, or simply redirect from the text file we created. To test I did the following:
```
./a.out < data
```
Each of the following tests were run multiple times. The outputs are from one of the tests.
#### N = 1
When n = 1, there shouldn't be any process creation, and both the algorithms should take approximately the same amount of time.
```
Normal merge sort took : 0.000006 seconds
Multiprocess merge sort took : 0.000003 seconds
```


#### N = 10
When n = 10, there should be only 2 processes created (one for left, and one for right). The time taken depends on how long process creation takes.
```
Normal merge sort took : 0.000011 seconds
Multiprocess merge sort took : 0.000504 seconds
```
The multiprocess version performed worse than its normal counterpart ie the process creation takes too much time for its benefits to be seen at this scale.

#### N = 100
```
Normal merge sort took : 0.000057 seconds
Multiprocess merge sort took : 0.000547 seconds
```
We see that the difference is slowly being bridged, and that the increment in the time taken by the multithreaded merge sort is similar to that of the normal merge sort. Hence we can conclude that the time taken by the multithreaded version is mainly due to process creation.

#### N = 1000
```
Normal merge sort took : 0.000677 seconds
Multiprocess merge sort took : 0.000656 seconds
```
Here we see that the multiprocess merge sort outperforms the normal merge sort (marginally). The gain in the speed finally balanced the process creation time.

#### N = 10000
```
Normal merge sort took : 0.007048 seconds
Multiprocess merge sort took : 0.001027 seconds
```
Here we can see that the gain from using multiple processes finally has a significant advantage.

#### N = 100000
```
Normal merge sort took : 0.011852 seconds
fork: Resource temporarily unavailable
...
Multiprocess merge sort took : 0.000920 seconds
```
At this point the number of forks is too high, and the program displays these error messages. The program also fails to sort the array as the forking fails.