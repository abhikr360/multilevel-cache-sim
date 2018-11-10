/*********** Author: Mainak Chaudhuri **************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
   unsigned long long tag;
   unsigned long long lru;
} CacheTag;

typedef struct 
{
   // CacheTag** L1InstCache;
   CacheTag** L1DataCache;
   // CacheTag** L2Cache;
} Core;

#define INVALID_TAG 0xfffffffffffffffULL

#define L1_NUMSET 64
#define LOG_L1_NUMSET 6
#define L1_ASSOC 8
#define L1_BLOCK_SIZE 64
#define LOG_L1_BLOCK_SIZE 6

// #define L2_NUMSET 256
// #define LOG_L2_NUMSET 8
// #define L2_ASSOC 8
// #define L2_BLOCK_SIZE 64
// #define LOG_L2_BLOCK_SIZE 6

#define NUM_CORES 8


CacheTag** create_cache (int numset, int assoc) {
/* Creates a cache with (number of sets = numset) and (associativity = assoc)
*/
   int i, j;
   CacheTag ** cache = (CacheTag**)malloc(numset*sizeof(CacheTag*));
   assert(cache != NULL);
   for (i=0; i<numset; i++) {
      cache[i] = (CacheTag*)malloc(assoc*sizeof(CacheTag));
      assert(cache[i] != NULL);
      for ( j=0; j<assoc; j++) {
         cache[i][j].tag = INVALID_TAG;
      }
   }
   return cache;
}


Core* create_cpu (int num_cores) {
/* Creates num_cores number of cores and each core has 2 caches
*/
   Core * cpu = (Core*)malloc(num_cores*sizeof(Core));
   int i;
   for (i=0; i<num_cores; i++) {
      // cpu[i].L1InstCache = create_cache(L1_NUMSET, L1_ASSOC);
      cpu[i].L1DataCache = create_cache(L1_NUMSET, L1_ASSOC);
      // cpu[i].L2Cache = create_cache(L2_NUMSET, L2_ASSOC);
   }
   return cpu;
}


int main (int argc, char **argv)
{
   int j, L1setid, maxindex, tid;
   unsigned long long block_addr, max;
   char output_name[256], input_name[256];
   FILE * fp_in;
   FILE * fp_out;
   int l1way;

   Core* cpu;

   cpu = create_cpu(NUM_CORES);

   if (argc != 3) {
      printf("Need two arguments: input file. Aborting...\n");
      exit (1);
   }


   sprintf(input_name, "%s", argv[1]);
   // sprintf(input_name, "../data/temp");
   sprintf(output_name, "%s", argv[2]);
   // sprintf(output_name, "../data/temp.out");
   fp_in = fopen(input_name, "r");
   fp_out = fopen(output_name, "w");
   assert(fp_in != NULL);
   assert(fp_out != NULL);
   while (!feof(fp_in)) {
      fscanf(fp_in, "%d %llu", &tid, &block_addr);
      tid = tid % NUM_CORES;

      L1setid = block_addr % L1_NUMSET ;
      // L2setid = block_addr % L2_NUMSET;
      /* L1 cache lookup */
      for (l1way=0; l1way<L1_ASSOC; l1way++) {
         if (cpu[tid].L1DataCache[L1setid][l1way].tag == block_addr) {
            break;
         }
      }
      if (l1way==L1_ASSOC) {  
      /*  L1 cache miss; */
         fprintf(fp_out, "%d %llu\n", tid, block_addr);
         /* Now fill in L1 cache */
         /* find the way that is invalid in L1 cache */
         for (l1way=0; l1way<L1_ASSOC; l1way++) {
            if (cpu[tid].L1DataCache[L1setid][l1way].tag == INVALID_TAG) break;
         }
         if (l1way==L1_ASSOC) {
            /* no way is invalid; find LRU */
            max = 0;
            for (l1way=0; l1way<L1_ASSOC; l1way++) {
               if (cpu[tid].L1DataCache[L1setid][l1way].lru >= max) {
                  max = cpu[tid].L1DataCache[L1setid][l1way].lru;
                  maxindex = l1way;
               }
            }
            l1way = maxindex;
         }
         /* l1way is the victim way */
         assert(l1way < L1_ASSOC);
         cpu[tid].L1DataCache[L1setid][l1way].tag = block_addr;
      }/* L1 cache now contains the required block in l1way */

      assert(l1way < L1_ASSOC);
      /* update LRU values of L1 cache */
      for (j=0; j<L1_ASSOC; j++) {
         cpu[tid].L1DataCache[L1setid][j].lru++;
      }
      cpu[tid].L1DataCache[L1setid][l1way].lru = 0;
   }
   printf("*** Done reading file! ***\n");

   fclose(fp_in);
   fclose(fp_out);
   return 0;
}
