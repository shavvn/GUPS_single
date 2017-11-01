#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define POLY 0x0000000000000007UL
#define PERIOD 1317624576693539401L

using namespace std;

/*
 * Simple single thread Gig Updates Per Second benchmark in C++
 * use g++ gups.cc -o gups to compile
 * Use argv to pass the memory size in MBs
 * e.g. ./gups 2048
 * If no memory size specified, use 2GB by default
 **/

// Copy-pasted from HPCC code base
uint64_t RNG(int64_t n) {
  int i, j;
  uint64_t m2[64];
  uint64_t temp, ran;

  while (n < 0) n += PERIOD;
  while (n > PERIOD) n -= PERIOD;
  if (n == 0) return 0x1;

  temp = 0x1;
  for (i=0; i<64; i++) {
    m2[i] = temp;
    temp = (temp << 1) ^ ((int64_t) temp < 0 ? POLY : 0);
    temp = (temp << 1) ^ ((int64_t) temp < 0 ? POLY : 0);
  }

  for (i=62; i>=0; i--)
    if ((n >> i) & 1)
      break;

  ran = 0x2;
  while (i > 0) {
    temp = 0;
    for (j=0; j<64; j++)
      if ((ran >> j) & 1)
        temp ^= m2[j];
    ran = temp;
    i -= 1;
    if ((n >> i) & 1)
      ran = (ran << 1) ^ ((int64_t) ran < 0 ? POLY : 0);
  }

  return ran;
}

// Actual table updates
void RandomAccessUpdate(uint64_t table_size, uint64_t *table) {
    uint64_t ran[128];
    uint64_t num_updates = 4 * table_size;
    
    for (int j = 0; j < 128; j++) {
        ran[j] = RNG( (num_updates/128) * j);
    }

    for (uint64_t i = 0; i < num_updates / 128; i++) {
        if (i % (1 << 20) == 0) {
            printf("updated %lu entries\n", i);
        }
        for (int j = 0; j < 128; j++) {
            ran[j] = (ran[j] << 1) ^ ((int64_t) ran[j] < 0 ? POLY : 0);
            table[ran[j] & (table_size - 1)] ^= ran[j];
        }
    }
}

int main(int argc, char *argv[]) {
    int mem_size_mb = 2048;
    if (argc > 1) {
        sscanf(argv[1], "%d", &mem_size_mb);
    }
   
    printf("Memory size %d MB\n", mem_size_mb);
    // shift 30 bits to convert MB -> B 
    uint64_t mem_size = ((uint64_t)mem_size_mb) << 20;
    mem_size /= sizeof(uint64_t);
    uint64_t table_size;

    // make sure table size is power of 2
    for (mem_size >>= 1, table_size = 1; 
         mem_size >= 1; 
         mem_size >>= 1, table_size <<= 1)
        ;

    printf("Table size: %lu, allocating table...\n", table_size);

    uint64_t *table = (uint64_t*)malloc(sizeof(uint64_t) * table_size);
    if (!table) {
        printf("Failed to allocated memory of size %lu!\n", table_size);
        return 1;
    }
    
    // initialize table
    printf("Initializing table...\n");
    for (uint64_t i = 0; i < table_size; i++) {
        table[i] = i;
    }

    printf("starting GUPs...\n");
    clock_t begin = clock();
    RandomAccessUpdate(table_size, table);
    printf("finished GUPs!\n");
    clock_t end = clock();
    
    double time_secs = (double)(end - begin) / CLOCKS_PER_SEC;
    double GUPs = 1e-9 * 4 * table_size / time_secs;

    printf("Time used = %.4f seconds\n", time_secs);
    printf("%.9f GUP/s\n", GUPs);

    return 0;
}

