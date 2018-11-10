typedef struct sharingHistoryEntry_s {
   int shared;
   unsigned long long id;
} SharingHistoryEntry;


typedef struct hashTableEntry_s {
   unsigned long long block_addr;
   int sh_len;
   SharingHistoryEntry *sh; // sharing history

   double pred;
   double entropy;

   struct hashTableEntry_s *next;
} HashTableEntry;


void copy_sharing_history (SharingHistoryEntry *src, SharingHistoryEntry *dst, int sh_len) {
	int i;
	assert(dst == NULL);
	dst = (SharingHistoryEntry*)malloc(sh_len* sizeof(SharingHistoryEntry));
	for (i=0; i<sh_len; i++) {
		dst[i].shared = src[i].shared;
		dst[i].id = src[i].shared;
	}
}

HashTableEntry* create_hash_table(int size) {
/* create a hash table of (size=size) */
   int j;
   HashTableEntry* ht;
   ht = (HashTableEntry*)malloc(SIZE*sizeof(HashTableEntry));
   assert(ht != NULL);
   for (j=0; j<SIZE; j++) {
      ht[j].block_addr = INVALID_TAG;
      ht[j].sh_len = 0;
      ht[j].next = NULL;
   }
}


void build_hash_table (HashTableEntry* ht, FILE *fp_in, List *addr_list) {
	unsigned long long block_addr, id;
	int sh_len, hash_index, i, shared;
	HashTableEntry *ptr, *prev;
	assert(ht != NULL); 
	assert(fp_in != NULL); 
	assert(addr_list != NULL); 

	while (!feof(fp_in)) {
		fscanf(fp_in, "%llu", &block_addr);
		fscanf(fp_in, "%d", &sh_len);

		ListInsert(addr_list, block_addr);

		hash_index = block_addr % SIZE;
			
		if (ht[hash_index].block_addr == INVALID_TAG) {
			ht[hash_index].block_addr = block_addr;
			ptr = &ht[hash_index];
		}
		else {
			prev = NULL;
			ptr = &ht[hash_index];
			while (ptr != NULL) {
				if (ptr->block_addr == block_addr) {
					assert(0);
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
				ptr->next = NULL;
				prev->next = ptr;
			}
		}
		assert(ptr != NULL);

		ptr->sh = (SharingHistoryEntry*)malloc(sh_len*sizeof(SharingHistoryEntry));

		for (i=0; i<sh_len; i++) {
			fscanf(fp_in, "%d %llu", &shared, &id);
			ptr->sh[i].shared = shared;
			ptr->sh[i].id = id;
		}
	}
}
