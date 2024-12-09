[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/kIMrm1Ns)
[![Open in Codespaces](https://classroom.github.com/assets/launch-codespace-2972f46106e565e64193e422d61a12cf1da4916b45550586e14ef0a7c637dd04.svg)](https://classroom.github.com/open-in-codespaces?assignment_repo_id=17075017)

# malloc Assignment

## Description

In this assignment you will build your own implementation of malloc and free. That is, you will need to implement a library that interacts with the operating system to perform heap management on behalf of a user process as demonstrated in class. This project must be completed, in C, on a course Codespace.

## Building and Running the Code

The code compiles into four shared libraries and six test programs. To build the code, change to your top level assignment directory and type:
```
make
```
Once you have the library, you can use it to override the existing malloc by using
LD_PRELOAD. The following example shows running the ffnf test using the First Fit shared library:
```
$ env LD_PRELOAD=lib/libmalloc-ff.so tests/ffnf
```

To run the other heap management schemes replace libmalloc-ff.so with the appropriate library:
```
Best-Fit: libmalloc-bf.so
First-Fit: libmalloc-ff.so
Next-Fit: libmalloc-nf.so
Worst-Fit: libmalloc-wf.so
```
## Program Requirements

Using the framework of malloc and free provided on the course github repository:
1. Implement splitting and coalescing of free blocks. If two free blocks are adjacent then
combine them. If a free block is larger than the requested size then split the block into two.
2. Implement three additional heap management strategies: Next Fit, Worst Fit, Best Fit (First
Fit has already been implemented for you).
3. Counters exist in the code for tracking of the following events:

* Number of times the user calls malloc successfully
* Number of times the user calls free successfully
* Number of times we reuse an existing block
* Number of times we request a new block
* Number of times we split a block
* Number of times we coalesce blocks
* Number blocks in free list
* Total amount of memory requested
* Maximum size of the heap

The code will print the statistics ( THESE STATS ARE FAKE) upon exit and should look like similar to:
```
mallocs: 8
frees: 8
reuses: 1
grows: 5
splits: 1
coalesces: 1
blocks: 5
requested: 7298
max heap: 4096
```

You will need to increment these counters where appropriate.

4. Eight test programs are provided to help debug your code. They are located in the tests
directory.
5. Implement realloc and calloc.
6. 
## Hint

You will see an extra malloc of 1024 bytes in your statistics. This is the system allocating memory for the printf statement.

## Debugging
While running the tests, you may encounter some segfaults and other memory errors. Because we are side-loading our own malloc implementation, you will need to do the following to debug a test application using gdb.
```
$ gdb ./tests/ffnf
...
(gdb) set exec-wrapper env LD_PRELOAD=./lib/libmalloc-ff.so
(gdb) run
...
(gdb) where
```
Basically, you want to first load gdb with the test program that is crashing. Next, you need to tell gdb to utilize the appropriate malloc library by creating an exec-wrapper that loads it into memory. Next, simply run the program. Once it segfaults, you can print a stack trace by using the where command. From there, you can explore the state of your program and see if you can determine what went wrong.
