# CS764
A simple implementation of database sorting system.

There are three columns:

| Field | Type     |
| ----- | -------- |
| incl  | uint32_t |
| mem   | uint32_t |
| Mgmt  | uint32_t |

We can choose any of them to compare and sorting.

All the sorting algorithms must be implemented by ourselves.


# Initialization

Because the project has only one core, so the whole process is synchronous.
We don't need to allocate big queue to each step. But the buffer queue must be big enough to reduce the comparison number of external merge sort.

And always try to encode according to [the principle of locality](https://en.wikipedia.org/wiki/Locality_of_reference).

TODO: considering the use of asynchronous IO, in this way, cpu can still be used when IO operation is executing.

Size of three Queues:

- The Size of Scan Queue: 2 MB

- The Size of Filter Queue: 1 MB

- The Size of Sort Queue: 90 MB :

  > (In order to maximize the usage of cpu cache, and because space complexity of quicksort is log(n), so maybe need (90 + log2(90)) ~= 96.5M)
  > (temporarily sorting by first field, and using quicksort, because quicksort's sequential and localized memory references work well with a cache)

After sorting, asynchronously writing buffer into SSD or HDD.

Each group member's names and student IDs:
Kefan Zheng 
Tianyu Huang
Yixiang Fang
Chuan Tian

The techniques (see Grading) implemented by your submission and the corresponding source files and lines
1. Quicksort?: Sort.cpp (Line 138 - 163)
2. Tournament trees [5]: LoserTree.cpp + LoserTree.h
3. Replacement selection
4. Run size > memory size
5. Offset-value coding: ovc.cpp + ovc.h + LoserTree.cpp(Line )
6. Variable-size records??
7. Compression?
8. Prefix truncation?
9. Minimum count of row & column comparisons [5]: Sort.cpp (Line 165 - 215)
10. Cache-size mini runs [5]: Sort.cpp (Line 22 - 53)
11. Device-optimized page sizes [5]: Test.cpp ( Line 55 )
12. Spilling memory-to-SSD [5]: DiskScan.cpp
13. Spilling from SSD to disk [5]: DiskScan.cpp
14. Graceful degradation
  a. into merging [5]
  b. beyond one merge step [5]
15. Optimized merge patterns [5]
16. Verifying (Verify.cpp + Verify.h)
  a. sets of rows & values [5]
  b. sort order [5]

A brief writeup of 
(1) the reasons you chose to implement the specific subset of techniques, this is not expected to be a lengthy document, just to assist our understanding of your work.
We use the same block size, 50MB, in SSD and HDD. The number of runs as leaf nodes is equal to the number of each 50MB block. The size of each runs in memory is calculated by 100MB (memory size) / N (numbers of 50MB-block). We chose this way because the bandwidth of SSD and HDD is the same and the latency of SSD and HDD is a 1:100 ratio. We utilized this metric, for every 100 consumptions by the SSD, a consumption operation is triggered once for the HDD. We sorted each 100MB in memory and wrote it back to SSD and HDD. Then we got a partition from each 100MB from HDD and sorted again.

(2) the project's state (complete or have what kinds of bugs) 
Complete, the record is sorted. 

(3) how to run your programs if they are different from the default specifications above. 

Each group member's contributions to the project:
Kefan Fan: Tournament trees, Shared Buffer to spilling, Verifying
Tianyu Huang: DiskScan file to write records back to HDD.
Chuan Tian: Offset-value coding
Ethan Fang: Quicksort
