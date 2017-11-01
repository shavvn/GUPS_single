# GUPS_single
Single core, single threaded GUPS benchmark stripped from HPCC benchmark suite.

## Build

```shell
g++ -O2 gups.cc -o gups
```

## Run
The only argument of the program is the memory size in MBs.
The official guideline is to use half of your total memory.
e.g. If your system has 2GB memory, then use

```
./gups 1024
```

The GUPs number will be reported at the end of the program.
