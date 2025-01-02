# Matrix-Vector-Multiplication

## Blocked Matrix Vector Multiplication
Blocked multiplication is a lot slower. After running the code a few times using block size of:

`int SM = sysconf (_SC_LEVEL1_DCACHE_LINESIZE) / sizeof(double);`
I got values around this:
```
Regular: 1803921554 nanoseconds
Regular: 1.80392 seconds
Block Multiplication: 3579152105 nanoseconds
Block Multiplication: 3.57915 seconds
```

After trying SM of 256, 1024, and 2048, the block multiplication approaches the regular multiplication time from above.

With a block size of `constexpr int SM = 2048;`
```
Regular: 1416801075 nanoseconds
Regular: 1.4168 seconds
Block Multiplication: 1502472357 nanoseconds
Block Multiplication: 1.50247 seconds
Result vectors are the same.
```
