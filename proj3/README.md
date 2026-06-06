# Project 3

CS415 Operating Systems

Suyash Gupta

Oliver Boorstein, 952043159

## Compile and run
```bash
cd part3
make
./park -n 30 -c 4 -p 2 -w 3 -r 2 -t 60 -j 10
```

Parts 1 and 2 compile and run the same way from their own directories. Part 1 blocks config for number of passengers and car capacity.

## Fast mode
I've also added an additional `-f` flag to enable a faster testing mode in part 3.
```bash
./park -f -n 30 -c 4 -p 2 -w 3 -r 2 -t 60 -j 10
```
