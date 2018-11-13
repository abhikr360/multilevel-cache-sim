typedef struct {
	unsigned long long tag;   
	HashTableEntry *htPtr;       // A pointer to the corresponding hash table entry
										  // Each block address gets a unique hash table entry
	int use_count[NUM_PROC]; // use_count[i] denotes how many times processor i used the LLC fill
} CacheTag;


typedef struct {
	/* declarations */
	// total numebr of misses
	unsigned long long total_miss_count;
	/* number of fills that are shared reused, private reused, not reused */
	unsigned long long count_shared_reuse_fills, count_private_reuse_fills, count_no_reuse_fills;
	/* number of hits to shared blocks, private blocks */
	unsigned long long count_shared_hits, count_private_hits;
	/* number of reuse of shared blocks, private blocks */
	unsigned long long count_shared_reuse, count_private_reuse;
	/* number of sharers: count_sharers[i] denotes the number of fills that were shared by i number of users */
	unsigned long long count_sharers[NUM_PROC];

} MetaData;


MetaData* create_metadata () {
	MetaData *metadata;
	int i;
	metadata = (MetaData*)malloc(sizeof(MetaData));
	metadata->total_miss_count = 0;
	metadata->count_shared_reuse_fills = 0;
	metadata->count_private_reuse_fills = 0;
	metadata->count_no_reuse_fills = 0;
	metadata->count_shared_hits = 0;
	metadata->count_private_hits = 0;
	metadata->count_shared_reuse = 0;
	metadata->count_private_reuse = 0;

	for (i=0; i<NUM_PROC; i++) {
		metadata->count_sharers[i] = 0;
	}

	return metadata;
}


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
			cache[i][j].htPtr = NULL;
			for (k=0; k<NUM_PROC; k++){
				cache[i][j].use_count[k] = 0;
			}
		}
	}
	return cache;
}


void flush_cache (CacheTag **LLCcache, HashTableEntry *ht, MetaData *metadata, unsigned long long uniqueId) {
	HashTableEntry *ptr;
	int hash_index, i, shared_reuse_flag, LLCsetid, llcway, private_reuse_flag;
	unsigned long long block_addr;
	
	for (LLCsetid=0; LLCsetid<LLC_NUMSET; LLCsetid++) {
		for (llcway=0; llcway<LLC_ASSOC; llcway++) {
			block_addr = LLCcache[LLCsetid][llcway].tag;
			if (block_addr == INVALID_TAG) continue;

			shared_reuse_flag = 0; private_reuse_flag = 0;
			for (i=0; i<NUM_PROC; i++) {
				if (LLCcache[LLCsetid][llcway].use_count[i] > 0) shared_reuse_flag++;
				if (LLCcache[LLCsetid][llcway].use_count[i] > private_reuse_flag) 
					private_reuse_flag = LLCcache[LLCsetid][llcway].use_count[i];
			}

			metadata->count_sharers[shared_reuse_flag-1]++;

			assert(shared_reuse_flag > 0);
			assert(private_reuse_flag > 0);
			
			hash_index = block_addr % SIZE;
			ptr = &ht[hash_index];
			while (ptr != NULL) {
				if (ptr->block_addr == block_addr) 	break;
				ptr = ptr->next;
			}
			assert(ptr != NULL);
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

			if (shared_reuse_flag > 1) {
				metadata->count_shared_reuse_fills++;
				for (i=0; i<NUM_PROC; i++) {
					metadata->count_shared_hits += LLCcache[LLCsetid][llcway].use_count[i];
					metadata->count_shared_reuse += LLCcache[LLCsetid][llcway].use_count[i];
				}
				metadata->count_shared_reuse--;
			}
			else {
				if (private_reuse_flag > 1) {
					metadata->count_private_reuse_fills++;
				}
				else {
					metadata->count_no_reuse_fills++;
				}  
				for (i=0; i<NUM_PROC; i++) {
					metadata->count_private_hits += LLCcache[LLCsetid][llcway].use_count[i];
					metadata->count_private_reuse += LLCcache[LLCsetid][llcway].use_count[i];
				}
				metadata->count_private_reuse--;
			}
		}
	}
}


void simulate (CacheTag **LLCcache, HashTableEntry *ht, FILE *fp_in, MetaData *metadata) {
	// declarations
	unsigned long long uniqueId = 0, block_addr, max, victim_block_addr;
	int tid, LLCsetid, llcway, hash_index, maxindex, victim_hash_index;
	HashTableEntry *ptr, *victim_ptr;
	int shared_reuse_flag, private_reuse_flag, i;


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
			metadata->total_miss_count++;

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

				metadata->count_sharers[shared_reuse_flag-1]++;

				assert(shared_reuse_flag > 0);
				assert(private_reuse_flag > 0);

				if (victim_ptr->sharing_history_head == NULL) {
					victim_ptr->sharing_history_head = (SharingList*)malloc(sizeof(SharingList));
					assert(victim_ptr->sharing_history_head != NULL);
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
					metadata->count_shared_reuse_fills++;
					for (i=0; i<NUM_PROC; i++) {
						metadata->count_shared_hits += LLCcache[LLCsetid][llcway].use_count[i];
						metadata->count_shared_reuse += LLCcache[LLCsetid][llcway].use_count[i];
					}
					metadata->count_shared_reuse--;
				}
				else {
					if (private_reuse_flag > 1) {
						metadata->count_private_reuse_fills++;
					}
					else {
						metadata->count_no_reuse_fills++;
					}  
					for (i=0; i<NUM_PROC; i++) {
						metadata->count_private_hits += LLCcache[LLCsetid][llcway].use_count[i];
						metadata->count_private_reuse += LLCcache[LLCsetid][llcway].use_count[i];
					}
					metadata->count_private_reuse--;
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

	uniqueId++;
	flush_cache(LLCcache, ht, metadata, uniqueId);
}



