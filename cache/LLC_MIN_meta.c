/*********** Author: Mainak Chaudhuri; Manish Kumar Bera **************/

#define SIZE 4194304
#define INVALID_TAG 0xfffffffffffffffULL

#define LLC_NUMSET 8192 /* 8 MB LLC: 8 X 1024 */
#define LLC_ASSOC 16
#define NUM_PROC 8

// #define LLC_BLOCK_SIZE 64
// #define LOG_LLC_BLOCK_SIZE 6

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// A list of integers
// Used to maintain the list of access timestamps for a particular block
typedef struct integerList_s {
   unsigned long long id;
   struct integerList_s *next;
} IntegerListEntry;

typedef struct sharingList_s {
   int shared;
   unsigned long long id;
   struct sharingList_s *next;
} SharingList;

// This hash table is used by the optimal policy for maintaining the next access stamp
// Each hash table entry has a block address and a list of access timestamps to that block
typedef struct hashTableEntry_s {
   unsigned long long block_addr;     // Block address
   IntegerListEntry *ilhead;    // Head of the access timestamp list
   IntegerListEntry *tail;      // Tail of the access timestamp list
   IntegerListEntry *currentPtr;// Pointer to the current position in the access list during simulation

   SharingList *sharing_history_head;
   SharingList *sharing_history_tail;
   struct hashTableEntry_s *next;
} HashTableEntry;

typedef struct {
   unsigned long long tag;   
   HashTableEntry *htPtr;       // A pointer to the corresponding hash table entry
                                // Each block address gets a unique hash table entry
   int use_count[NUM_PROC]; // this array denotes whether the a processor used the the LLC fill
} CacheTag;



CacheTag** create_cache (int numset, int assoc) {
/* Creates a cache with (number of sets = numset) and (associativity = assoc)
*/
   int i, j, k;
   CacheTag ** cache = (CacheTag**)malloc(numset*sizeof(CacheTag*));
   assert(cache != NULL);
   for (i=0; i<numset; i++) {
      cache[i] = (CacheTag*)malloc(assoc*sizeof(CacheTag));
      assert(cache[i] != NULL);
      for ( j=0; j<assoc; j++) {
         cache[i][j].tag = INVALID_TAG;
         for (k=0; k<NUM_PROC; k++){
            cache[i][j].use_count[k] = 0;
         }
      }
   }
   return cache;
}


HashTableEntry* create_hash_table(int size) {
/* create a hash table of (size=size) */
   int j;
   HashTableEntry* ht;
   ht = (HashTableEntry*)malloc(SIZE*sizeof(HashTableEntry));
   assert(ht != NULL);
   for (j=0; j<SIZE; j++) {
      ht[j].ilhead = NULL;
      ht[j].tail = NULL;
      ht[j].sharing_history_head = NULL;
      ht[j].sharing_history_tail = NULL;
   }
}

void fprint_sharing_history(HashTableEntry* ht, FILE* fp) {
   // int temp=0;
   int history_count=0;
   int i;
   HashTableEntry *ptr;
   SharingList *head;
   for (i=0; i<SIZE; i++) {
      if (ht[i].ilhead == NULL) continue;
      ptr = &ht[i];
      while (ptr != NULL) {
         fprintf(fp, "%llu\n", ptr->block_addr);
         head = ptr->sharing_history_head;
         history_count = 0;
         while (head != NULL) {
            history_count++;
            head = head->next;
         }
         fprintf(fp, "%d\n", history_count);
         head = ptr->sharing_history_head;
         while (head != NULL) {
            fprintf(fp, "%d %llu ", head->shared, head->id);
            head = head->next;
         }
         fprintf(fp, "\n");
         ptr = ptr->next;
     } 
   }
// printf("mark\n");
}

int main (int argc, char **argv)
{
   /* meta data counters */
   /* number of fills that are shared reused, private reused, not reused */
   unsigned long long count_shared_reuse_fills=0, count_private_reuse_fills=0, count_no_reuse_fills=0;
   /* number of hits to shared blocks, private blocks */
   unsigned long long count_shared_hits=0, count_private_hits=0;
   /* number of reuse of shared blocks, private blocks */
   unsigned long long count_shared_reuse, count_private_reuse;
   /* number of sharers */
   unsigned long long count_sharers[NUM_PROC];
   // ---------------------------------------------

   int shared_reuse_flag, private_reuse_flag;
   int temp=0;

   int  i, LLCsetid, maxindex, tid;
   unsigned long long block_addr, max; //*uniqueId;
   unsigned long long uniqueId;

   unsigned long long victim_block_addr;
   char output_name[256], input_name[256], history_file_name[256];
   FILE *fp_in; FILE *fp_out; FILE *fp_history;
   int llcway;
   HashTableEntry *ht, *prev, *ptr;
   HashTableEntry *victim_ptr;
   int hash_index, victim_hash_index;
   CacheTag** LLCcache;

   if (argc != 4) {
      printf("Need 3 arguments: input file. Aborting...\n");
      exit (1);
   }

   LLCcache = (CacheTag**)create_cache(LLC_NUMSET, LLC_ASSOC);
   ht = (HashTableEntry*)create_hash_table(SIZE);

   /* The following array is used to find the sequence number for an access to a set;
   This sequence number acts as the timestamp for the access */
   // uniqueId = (unsigned long long*)malloc(LLC_NUMSET*sizeof(unsigned long long));
   // assert(uniqueId != NULL);
   // for (i=0; i<LLC_NUMSET; i++) {
   //    uniqueId[i] = 0;
   // }
   uniqueId = 0;

   /* Build the hash table of accesses */
   sprintf(input_name, "%s", argv[1]);
   sprintf(output_name, "%s", argv[2]);
   fp_in = fopen(input_name, "r");
   fp_out = fopen(output_name, "w");
   assert(fp_in != NULL);
   assert(fp_out != NULL);


   while (!feof(fp_in)) {
      fscanf(fp_in, "%d %llu", &tid, &block_addr);
      hash_index = block_addr  % SIZE;
// printf("%d\n", hash_index);
      LLCsetid = block_addr % LLC_NUMSET;
      if (ht[hash_index].ilhead == NULL) {
         ht[hash_index].block_addr = block_addr;
         ht[hash_index].ilhead = (IntegerListEntry*)malloc(sizeof(IntegerListEntry));
         assert(ht[hash_index].ilhead != NULL);
         ht[hash_index].tail = ht[hash_index].ilhead;
         ht[hash_index].ilhead->id = uniqueId;
         ht[hash_index].ilhead->next = NULL;
         ht[hash_index].currentPtr = ht[hash_index].ilhead;  // Initialize to point to the beginning of the list
         ht[hash_index].next = NULL;
      }
      else {
         prev = NULL;
         ptr = &ht[hash_index];
         while (ptr != NULL) {
            if (ptr->block_addr == block_addr) {
               assert(ptr->ilhead != NULL);
               assert(ptr->tail->next == NULL);
               ptr->tail->next = (IntegerListEntry*)malloc(sizeof(IntegerListEntry));
               assert(ptr->tail->next != NULL);
               ptr->tail = ptr->tail->next;
               ptr->tail->id = uniqueId;
               ptr->tail->next = NULL;
               break;
            }
            prev = ptr;
            ptr = ptr->next;
         }
         if (ptr == NULL) {
            assert(prev->next == NULL);
            ptr = (HashTableEntry*)malloc(sizeof(HashTableEntry));
            assert(ptr != NULL);
            ptr->block_addr = block_addr;
            ptr->ilhead = (IntegerListEntry*)malloc(sizeof(IntegerListEntry));
            assert(ptr->ilhead != NULL);
            ptr->tail = ptr->ilhead;
            ptr->tail->id = uniqueId;
            ptr->tail->next = NULL;
            ptr->next = NULL;
            ptr->currentPtr = ptr->ilhead;
            prev->next = ptr;
            ptr->sharing_history_head = NULL;
            ptr->sharing_history_tail = NULL;
         }
      }
      uniqueId++;
// printf("%llu\n", block_addr);
   }
   fclose(fp_in);
   printf("Done reading file!\n");
// printf("%llu\n", ht[1].block_addr);

   // fprint_sharing_history(ht, stdout);

   printf("Access list prepared.\nStarting simulation...\n"); fflush(stdout);

   uniqueId = -1;
   // Simulate
   // sprintf(input_name, "%s", argv[1]);
   fp_in = fopen(input_name, "r");
   assert(fp_in != NULL);

   while (!feof(fp_in)) {
      fscanf(fp_in, "%d %llu", &tid, &block_addr); uniqueId++;
      hash_index = block_addr % SIZE;
      LLCsetid = block_addr % LLC_NUMSET;

      /* LLC cache lookup */
      for (llcway=0; llcway<LLC_ASSOC; llcway++) {
         if (LLCcache[LLCsetid][llcway].tag == block_addr) {
            /* LLC cache hit; Update access list */
            assert(LLCcache[LLCsetid][llcway].htPtr != NULL);
            assert(LLCcache[LLCsetid][llcway].htPtr->block_addr == block_addr);
            assert(LLCcache[LLCsetid][llcway].htPtr->currentPtr != NULL);
            LLCcache[LLCsetid][llcway].htPtr->currentPtr = LLCcache[LLCsetid][llcway].htPtr->currentPtr->next;
            break;
         }
      }
      if (llcway==LLC_ASSOC) {
         /* LLC cache miss */

         /* Access list pointer needs to be advanced; 
         Search the entry in hash table */
         ptr = &ht[hash_index];
         while (ptr != NULL) {
            if (ptr->block_addr == block_addr) break;
            ptr = ptr->next;
         }
         assert(ptr != NULL);
         assert(ptr->currentPtr != NULL);
         ptr->currentPtr = ptr->currentPtr->next; // Advance to point to the next access

         /* find victim block and replace it with current block */

         /* check if there is invalid way */
         for (llcway=0; llcway<LLC_ASSOC; llcway++) {
            if (LLCcache[LLCsetid][llcway].tag == INVALID_TAG) break;
         }

         if (llcway==LLC_ASSOC) {
            /* no invalid way; find MIN */ 
            max = 0;
            for (llcway=0; llcway<LLC_ASSOC; llcway++) {
               if ((LLCcache[llcway][llcway].htPtr == NULL) || (LLCcache[LLCsetid][llcway].htPtr->currentPtr == NULL)) {
               /* No future access (eternally dead) */
                  maxindex = llcway;
                  break;
               }
               if (LLCcache[LLCsetid][llcway].htPtr->currentPtr->id >= max) {
                  max = LLCcache[LLCsetid][llcway].htPtr->currentPtr->id;
                  maxindex = llcway;
               }
            }
            llcway = maxindex;

            victim_block_addr = LLCcache[LLCsetid][llcway].tag;
            victim_hash_index = victim_block_addr % SIZE;
            victim_ptr = &ht[victim_hash_index];
            while (victim_ptr != NULL) {
               if (victim_ptr->block_addr == victim_block_addr) break;
               victim_ptr = victim_ptr->next;
            }
            assert(victim_ptr != NULL);

            shared_reuse_flag = 0; private_reuse_flag = 0;
            for (i=0; i<NUM_PROC; i++) {
               if (LLCcache[LLCsetid][llcway].use_count[i] > 0) shared_reuse_flag++;
               if (LLCcache[LLCsetid][llcway].use_count[i] > private_reuse_flag) 
                  private_reuse_flag = LLCcache[LLCsetid][llcway].use_count[i];
            }

            count_sharers[shared_reuse_flag-1]++;

            if (victim_ptr->sharing_history_head == NULL) {
               victim_ptr->sharing_history_head = (SharingList*)malloc(sizeof(SharingList));
               assert(victim_ptr != NULL);
               victim_ptr->sharing_history_tail = victim_ptr->sharing_history_head;

               if (shared_reuse_flag > 1) victim_ptr->sharing_history_tail->shared = 1;
               else victim_ptr->sharing_history_tail->shared = 0;

               victim_ptr->sharing_history_tail->id = uniqueId;

               victim_ptr->sharing_history_tail->next = NULL;
            }
            else {
               assert(victim_ptr->sharing_history_tail != NULL);
               assert(victim_ptr->sharing_history_tail->next == NULL);
               victim_ptr->sharing_history_tail->next = (SharingList*)malloc(sizeof(SharingList));
               assert(victim_ptr->sharing_history_tail->next != NULL);
               victim_ptr->sharing_history_tail = victim_ptr->sharing_history_tail->next;

               if (shared_reuse_flag > 1) victim_ptr->sharing_history_tail->shared = 1;
               else victim_ptr->sharing_history_tail->shared = 0;

               victim_ptr->sharing_history_tail->id = uniqueId;

               victim_ptr->sharing_history_tail->next = NULL;
            }

            if (shared_reuse_flag > 1) {
               count_shared_reuse_fills++;
               for (i=0; i<NUM_PROC; i++) {
                  count_shared_hits += LLCcache[LLCsetid][llcway].use_count[i];
                  count_shared_reuse += LLCcache[LLCsetid][llcway].use_count[i];
               }
               count_shared_reuse--;
            }
            else {
               if (private_reuse_flag > 1) {
                  count_private_reuse_fills++;
               }
               else {
                  count_no_reuse_fills++;
               }  
               for (i=0; i<NUM_PROC; i++) {
                  count_private_hits += LLCcache[LLCsetid][llcway].use_count[i];
                  count_private_reuse += LLCcache[LLCsetid][llcway].use_count[i];
               }
               count_private_reuse--;
            }
         }
         assert (llcway < LLC_ASSOC);
         
         LLCcache[LLCsetid][llcway].tag = block_addr;
         LLCcache[LLCsetid][llcway].htPtr = ptr;    // Set up the hash table pointer
         for (i=0; i<NUM_PROC; i++) {
            LLCcache[LLCsetid][llcway].use_count[i] = 0;
         }
      }

      LLCcache[LLCsetid][llcway].use_count[tid]++;
      
   }
   fclose(fp_in);

   printf("Done reading file!\n");


   /* Sanity check terminal state
   All access lists must have been exhausted */
   for (i=0; i<SIZE; i++) {
      if (ht[i].ilhead != NULL) {
         ptr = &ht[i];
         while (ptr != NULL) {
            assert(ptr->currentPtr == NULL);
            ptr = ptr->next;
         }
      }
   }

   fprintf(fp_out, "%llu %llu %llu\n", count_shared_reuse_fills, count_private_reuse_fills, count_no_reuse_fills);
   fprintf(fp_out, "%llu %llu\n", count_shared_hits, count_private_hits);
   fprintf(fp_out, "%llu %llu\n", count_shared_reuse, count_private_reuse);
   for (i=0; i<NUM_PROC; i++) {
      fprintf(fp_out, "%llu", count_sharers[i]);
   } fprintf(fp_out, "\n" );
   fclose(fp_out);

   sprintf(history_file_name, "%s", argv[3]);
   fp_history = fopen(history_file_name, "w");

   for (LLCsetid=0; LLCsetid<LLC_NUMSET; LLCsetid++) {
      for (llcway=0; llcway<LLC_ASSOC; llcway++) {
         block_addr = LLCcache[LLCsetid][llcway].tag;
         if (block_addr == INVALID_TAG) continue;

         shared_reuse_flag = 0;
         for (i=0; i<NUM_PROC; i++) {
            if (LLCcache[LLCsetid][llcway].use_count[i] > 0) shared_reuse_flag++;
         }
         assert(shared_reuse_flag > 0);
         hash_index = block_addr % SIZE;
         ptr = &ht[hash_index];
         while (ptr != NULL) {
// // printf("mark %d\n", temp);temp++;
            if (ptr->block_addr == block_addr) {
               if (ptr->sharing_history_head == NULL) {
                  ptr->sharing_history_head = (SharingList*)malloc(sizeof(SharingList));
                  assert(ptr->sharing_history_head != NULL);
                  ptr->sharing_history_tail = ptr->sharing_history_head;

                  if (shared_reuse_flag > 1) ptr->sharing_history_tail->shared = 1;
                  else ptr->sharing_history_tail->shared = 0;

                  ptr->sharing_history_tail->id = uniqueId;

                  ptr->sharing_history_tail->next = NULL;
               }
               else {
                  assert(ptr->sharing_history_tail != NULL);
                  assert(ptr->sharing_history_tail->next == NULL);
                  ptr->sharing_history_tail->next = (SharingList*)malloc(sizeof(SharingList));
                  assert(ptr->sharing_history_tail->next != NULL);
                  ptr->sharing_history_tail = ptr->sharing_history_tail->next;

                  if (shared_reuse_flag > 1) ptr->sharing_history_tail->shared = 1;
                  else ptr->sharing_history_tail->shared = 0;

                  ptr->sharing_history_tail->id = uniqueId;

                  ptr->sharing_history_tail->next = NULL;
               }
               break;
            }
            ptr = ptr->next;
         }
         assert(ptr != NULL);
      }
   }

fprint_sharing_history(ht, fp_history);


   fclose(fp_history);
   return 0;
}
