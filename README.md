# CS 764 Project
A simple implementation of database sorting system has been implemented, which supports external merge sorting of massive data. The current version of the program is complete and has no bugs.

## Introduction
Our project contains four modules.
| Module Name | Function |
| -- | -- |
| Scan | Generate the input records, save them in the input directory and scan them |
| Filter (Not required but still implemented)| Add filter conditions to generated data to select specific data |
| In-memory Sort | Quick sort the input records, and write these sorted data groups into memory, then read data from memory in groups, and use loser tree to reorder these groups of data into a large ordered array. which is divided into Run Generation and spill datas to SSD and HDD in batches |
| External Merge Sort | From the file generated in the previous step, read the groups of data into memory and use the loser tree to sort, and write the final sorted results back to HDD|
| Verify | Verify the output file to ensure the sort is successful and valid |
### Run Generation
In order to make full use of the CPU Cache, quicksort is first used to generate cache-size mini runs. Until the memory is exhausted, use the loser tree to merge cache-size runs into memory-size runs and save them into SSD. Here we design a shared buffer based on the Producer-Consumer model so that when cache-size run merging produces sorted results, SSD can consume the sorted data simultaneously by asynchronous IO without interrupting the CPU, which optimizes performance.  
Based on the premise that the bandwidth of SSD and HDD are the same and the idea from the paper [AlphaSort: A Cache-Sensitive Parallel External Sort](https://sci-hub.se/10.1007/bf01354877), we treat HDD as an extension of SSD, that is the second SSD, and write the SSD and HDD at the same time to double the overall bandwidth of IO.
### External Merge Sort
Since there are memory-size runs on SSD and HDD, the first part of each run is read into memory to merge using a loser tree, and each run is continuously refilled until the merge ends. The total fan-in equals to: $size_{input}/size_{memory}$ . Sharedbuffer is used again to achieve memory-size runs merging and final result output simultaneously.   
 

## Run
### 1. Enter the project folder:
```shell
cd ${workspaceFolder}
```
### 2. Build the project with makefile:
```shell
make
```
### 3. Run 120GB test:
```shell
./Test -c 120000000 -s 1000 -o trace0.txt
```
Where "-c" gives the total number of records, "-s" is the individual record size, and "-o" is the trace of the program run. The size of the input data can be adjusted by changing the command line parameters.

## Features
- [x] 1. Quicksort
- [x] 2. Tournament trees [5]
- [ ] 3. Replacement selection
- [ ] 4. Run size > memory size
- [x] 5. Offset-value coding [5]
- [x] 6. Variable-size records
- [ ] 7. Compression
- [ ] 8. Prefix truncation
- [x] 9. Minimum count of row & column comparisons [5]
- [x] 10. Cache-size mini runs [5]
- [x] 11. Device-optimized page sizes [5]
- [x] 12. Spilling memory-to-SSD [5]
- [x] 13. Spilling from SSD to disk [5]
- [x] 14. Graceful degradation
- [x] a. into merging [5]
- [x] b. beyond one merge step [5]
- [x] 15. Optimized merge patterns [5]
- [x] 16. Verifying
- [x] a. sets of rows & values [5]
- [x] b. sort order [5]

## Implemented Techniques & Instructions
### Quicksort [Sort.cpp line 151]
For a sequence with $N$ distinct key values, the theoretical time complexity limit is $\log_2(N) \approx N*log_2(N/e)$ based on Stirling Formula. Because Quicksort has an average time complexity of $O(n\log_2n)$, which is usually the most efficient sorting algorithm, and its sequential and localized memory references properties work well with a cache, Quicksort is an ideal in-memory sorting algorithm in our project. 
### Tournament Trees [LoserTree.cpp]
The tournament tree is a data structure commonly used in sorting algorithms to quickly find the minimum or maximum value. Its main advantage is the ability to find the smallest or largest element in a faster time and retain information from previous comparisons to efficiently search for subsequent elements. A loser tree is used in this project to maximize the performance of internal and external merges.
### Offset-Value Coding [LoserTree.cpp & ovc.cpp]
Offset-value coding is an encoding method for sort keywords, which reduces comparison overhead in sorting by avoiding full-string comparison of keywords.
### Variable-Size Records [Scan.cpp line 62]
The size of the record is controlled by the input.  
Additionally, one record has three columns and each column is of type character array to control record size easily:
| Field Name | Index | Type  |        Size       |
| ---------- | ----- | ----- | ----------------- |
|    Incl    |   0   | char* | $size_{record}/3$ |
|    Mem     |   1   | char* | $size_{record}/3$ |
|    Mgmt    |   2   | char* | $size_{record}-2*size_{record}/3$ |
### Minimum Count of Row & Column Comparisons [LoserTree.cpp & ovc.cpp]
The Loser-Tree and Offset-value coding work together to minimize the count of comparisons between record rows and columns. The loser tree minimizes the number of comparisons of data rows to $O(M*logN)$ by retaining previous comparison information, which is close to the limit, while offset-value encoding quickly compares two strings through only one ovc comparison, minimizing the number of column comparisons.
### Cache-Size Mini Runs [Sort.cpp line 36]
Modern computers have multiple levels of storage structures: Registers, Cache, RAM, and Hard Disk (SSD, HDD). Among them, the Cache is located between the CPU and RAM. It is faster than RAM but has a smaller capacity.
In order to maximize the usage of CPU cache, it is necessary to ensure that all data to be sorted is in the cache, so mini cache size runs (1MB) are generated first at the beginning of sorting.
### Device-Optimized Page Sizes [defs.h line 46-49]
In this project, SSD and HDD have different latencies and bandwidths. According to the calculation formula of data I/O time:  
$$
I/O\ time = 
\begin{cases} 
latency, & data \leq (latency*bandwidth) \\
latency*\lceil data/(latency*bandwidth) \rceil, & data > (latency*bandwidth)
\end{cases}
$$  
, the optimal device-based I/O page size is calculated (SSD 10KB, HDD 1MB) and applied in the project.  
### Spilling Memory-to-SSD [SharedBuffer.cpp line 96]
Because the data to be sorted is much larger than the size of RAM, after generating memory-size runs, the data will spill into the SSD, freeing up memory space to other data for internal sorting.
### Spilling from SSD to Disk [SharedBuffer.cpp line 130]
Because the capacity of the SSD is not enough to accommodate all the data, when the SSD capacity is full, the data will spill to the HDD to ensure that all data will be processed.
### Graceful Degradation [Sort.cpp line 92 & 118 & 181 DiskScan.cpp line 238]
Graceful degradation means that when an input is just a little too large to be sorted in memory, there is no need to spill the entire input to disk. A better policy is to spill only as much as absolutely necessary so as to make space for extra input records, to minimize the total I/O cost. 
#### a. into merging
In the project, it is possible that the data to be sorted is a little larger than the storage capacity (Cache, RAM, SSD), so graceful degradation is necessary.  
Taking the generation of cache size run as an example, the solution is to use a fixed-size circular queue. When encountering data slightly larger than the queue length, the excess data will only cover part of the data at the head of the queue, which means that only a small amount of data spills, and the remaining data remains in the queue, meeting the conditions for graceful degradation.  
In addition, taking loser tree merging as an example, when the data to be merged is slightly larger than the memory size, graceful degradation is also implemented using a circular queue. Only the extra data overwrites the memory, while most of the rest of the data remains in memory. In our project, when the in-memory data group corresponding to a leaf node is exhausted, we immediately read the next batch of data from the corresponding group in the disk instead of waiting for all the data in the memory to be consumed. This method takes advantage of graceful degredation.
#### b. beyond one merge step
This project contains multiple merge steps. In each step, we use the circular queue method mentioned above to achieve graceful degradation.
### Verifying [Verify.cpp]
In the verification phase, the order of output records and the consistency of input and output sets are two aspects that need to be checked. The main challenge is how to load data that is much larger than the memory size into memory for verification. The method used here is partition hashing.
- Step 0: Calculate the number of buckets (hash value space) based on the size of the memory and input and output files to ensure that a single bucket can be read into the memory.  
- Step 1: Read the output files into memory in batches.
- Step 2: Hash every record in the current batch of data, distribute the records into different buckets based on the hash results, and flush the bucket to the disk at last.
- Step 3: Repeat steps 1 and 2 until the whole output file is processed. Get the hash result (bucket on disk) of the output file.
- Step 4: Do step 1,2,3 for the input file. Get the hash bucket result of the input file.
#### a. sets of rows & values
Load the two bucket files with the same hash value of the input file and the output file into the memory, and determine whether the data on both sides match. Because the hash values are the same, they should correspond to the same block of original data. If there is a data mismatch, it means that the output is inconsistent with the input.
#### b. sort order
The verification of the order can be done during scanning of the output file. Just use a variable to store the previous record and determine whether the current variable is not less than the previous variable.

### Group Members & Contributions
| Name | Contributions |
|---|---|
| Kefan Zheng | Tournament trees, Shared Buffer to spill data from memory to disks, Verifying, Readme|
| Tianyu Huang | Scan SSD&HDD and do external merge sort, Write data back to HDD, Trace|
| Chuan Tian | Offset-value coding, Readme|
| Ethan Fang | Base structure, In-memory Scan&Filter&Quick Sort, Whole process debug |
