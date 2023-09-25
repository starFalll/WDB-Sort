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