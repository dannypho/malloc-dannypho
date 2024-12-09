# Custom malloc Implementation in C
## Description
This project implements a custom version of the malloc and free functions for dynamic memory management in C. The goal is to create a memory allocator that interacts with the operating system to manage heap space efficiently, supporting different allocation strategies such as First Fit, Best Fit, Next Fit, and Worst Fit. Additionally, it features splitting and coalescing of free blocks to optimize memory reuse.

## Features
Memory Allocation Strategies:

1. First Fit
2. Best Fit
3. Next Fit
4. Worst Fit

Dynamic Memory Management Operations:

Splitting: When a free block is larger than the requested size, the block is split into smaller segments.

Coalescing: Adjacent free blocks are merged to reduce fragmentation.

realloc and calloc: Implementations of realloc and calloc are included for completeness.

Statistics Tracking: The implementation tracks various statistics to analyze the performance of the allocator

Example output of statistics upon program exit:
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

## Building the Project
When running the project for the first time:
```
mkdir lib
```
To build the project and compile the shared libraries and test programs:
```
make
```

## Running Tests
Use the LD_PRELOAD environment variable to override the default malloc with your custom allocator. Here's an example of running a test with the First Fit allocator:
```
env LD_PRELOAD=lib/libmalloc-ff.so tests/ffnf
```
Replace libmalloc-ff.so with the appropriate library for other allocation strategies:

* Best Fit: libmalloc-bf.so
* Next Fit: libmalloc-nf.so
* Worst Fit: libmalloc-wf.so

