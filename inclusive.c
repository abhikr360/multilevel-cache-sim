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
   CacheTag** L1InstCache;
   CacheTag** L1DataCache
   CacheTag** L2Cache;
} Core;

#define INVALID_TAG 0xfffffffffffffffULL

#define L1_NUMSET 64
#define L1_ASSOC 8
#define L1_BLOCK_SIZE 64
#define LOG_L1_BLOCK_SIZE 6

#define L2_NUMSET 256
#define L2_ASSOC 8
#define L2_BLOCK_SIZE 64
#define LOG_L2_BLOCK_SIZE 6

#define NUM_CORES 8


CacheTag** create_cache (int numset, int assoc) {
/* Creates a cache with (number of sets = numset) and (associativity = assoc)
*/
   cache = (CacheTag**)malloc(numset*sizeof(CacheTag*));
   assert(cache != NULL);
   for (i=0; i<numset; i++) {
      cache[i] = (CacheTag*)malloc(assoc*sizeof(CacheTag));
      assert(cache[i] != NULL);
      for (j=0; j<assoc; j++) {
         cache[i][j].tag = INVALID_TAG;
      }
   }
   return cache;
}


Core* create_cpu (int num_cores) {
/* Creates num_cores number of cores and each core has 2 caches
*/
   cpu = (Core*)malloc(num_cores*sizeof(Core))
   for (int i=0; i<num_cores; i++) {
      cpu[i].L1InstCache = create_cache(L1_NUMSET, L1_ASSOC);
      cpu[i].L1DataCache = create_cache(L1_NUMSET, L1_ASSOC);
      cpu[i].L2Cache = create_cache(L2_NUMSET, L2_ASSOC);
   }
   return cpu;
}


int main (int argc, char **argv)
{
   int i, j, k, m, L3setid, L2setid, INVsetid, maxindex, numtraces;
   unsigned long long addr, miss=0, max, l2_miss=0;
   char dest_file_name[256], input_name[256];
   char type, iord;
   FILE *fp;
   CacheTag **cache2, **cache3;
   unsigned pc;
   int l2way, l3way, updateway;

   Core* cpu;

   cpu = create_cpu(NUM_CORES);


   sprintf(input_name, "%s_%d", argv[1], k);
   fp = fopen(input_name, "rb");
   assert(fp != NULL);

   while (!feof(fp)) {
      fread(&iord, sizeof(char), 1, fp);
      fread(&type, sizeof(char), 1, fp);
      fread(&addr, sizeof(unsigned long long), 1, fp);
      fread(&pc, sizeof(unsigned), 1, fp);
      L2setid = (addr >> LOG_L2_BLOCK_SIZE) & (L2_NUMSET - 1);
      L3setid = (addr >> LOG_L3_BLOCK_SIZE) & (L3_NUMSET - 1);

      if (type != 0) {
      // L2 cache lookup
      for (l2way=0; l2way<L2_ASSOC; l2way++) {
         if (cache2[L2setid][l2way].tag == (addr >> LOG_L2_BLOCK_SIZE)) {
            break;
         }
      }
      if (l2way==L2_ASSOC) {  // L2 cache miss
         l2_miss++;
         // L3 lookup
         for (l3way=0; l3way<L3_ASSOC; l3way++) {
            if (cache3[L3setid][l3way].tag == (addr >> LOG_L3_BLOCK_SIZE)) {
               break;
            }
         }
         if (l3way==L3_ASSOC) {
            // Replace
            miss++;
            for (l3way=0; l3way<L3_ASSOC; l3way++) {
               if (cache3[L3setid][l3way].tag == INVALID_TAG) break;
            }
            if (l3way==L3_ASSOC) {
               // Find LRU
               max = 0;
               for (l3way=0; l3way<L3_ASSOC; l3way++) {
                  if (cache3[L3setid][l3way].lru >= max) {
                     max = cache3[L3setid][l3way].lru;
                     maxindex = l3way;
                  }
               }
               l3way = maxindex;
            }
            assert (l3way < L3_ASSOC);
            // Invalidate L3 tag in L2 cache
            if (cache3[L3setid][l3way].tag != INVALID_TAG) {
               INVsetid = ((cache3[L3setid][l3way].tag << LOG_L3_BLOCK_SIZE) >> LOG_L2_BLOCK_SIZE) & (L2_NUMSET - 1);
               for (updateway=0; updateway<L2_ASSOC; updateway++) {
                  if (cache2[INVsetid][updateway].tag == ((cache3[L3setid][l3way].tag << LOG_L3_BLOCK_SIZE) >> LOG_L2_BLOCK_SIZE)) {
                     cache2[INVsetid][updateway].tag = INVALID_TAG;
                     break;
                  }
               }
            }
            cache3[L3setid][l3way].tag = (addr >> LOG_L3_BLOCK_SIZE);
         }
         for (j=0; j<L3_ASSOC; j++) {
            cache3[L3setid][j].lru++;
         }
         cache3[L3setid][l3way].lru = 0;
         // Now fill in L2 cache
         for (l2way=0; l2way<L2_ASSOC; l2way++) {
            if (cache2[L2setid][l2way].tag == INVALID_TAG) break;
         }
         if (l2way==L2_ASSOC) {
            // Find LRU
            max = 0;
            for (l2way=0; l2way<L2_ASSOC; l2way++) {
               if (cache2[L2setid][l2way].lru >= max) {
                  max = cache2[L2setid][l2way].lru;
                  maxindex = l2way;
               }
            }
            l2way = maxindex;
         }
         assert(l2way < L2_ASSOC);
         cache2[L2setid][l2way].tag = (addr >> LOG_L2_BLOCK_SIZE);
      }
      assert(l2way < L2_ASSOC);
      for (j=0; j<L2_ASSOC; j++) {
         cache2[L2setid][j].lru++;
      }
      cache2[L2setid][l2way].lru = 0;
      }
   }
   fclose(fp);
   printf("Done reading file %d!\n", k);

   sprintf(dest_file_name, "%s.inclusion.L2%d", argv[1], L2_NUMSET/2);
   fp = fopen(dest_file_name, "w");
   assert(fp != NULL);
   fprintf(fp, "L3: %llu, L2: %llu\n", miss, l2_miss);
   fclose(fp);
   return 0;
}
