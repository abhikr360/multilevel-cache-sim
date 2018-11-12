typedef struct integerList_s {
/* A list of integers
 Used to maintain the list of access timestamps for a particular block */
	unsigned long long id;
	struct integerList_s *next;
} IntegerListEntry;


typedef struct sharingList_s {
	int shared;
	unsigned long long id;
	struct sharingList_s *next;
} SharingList;


typedef struct hashTableEntry_s {
/* This hash table is used by the optimal policy for maintaining the next access stamp
 Each hash table entry has a block address and a list of access timestamps to that block */
	unsigned long long block_addr;     // Block address
	IntegerListEntry *ilhead;    // Head of the access timestamp list
	IntegerListEntry *tail;      // Tail of the access timestamp list
	IntegerListEntry *currentPtr;// Pointer to the current position in the access list during simulation

	SharingList *sharing_history_head;
	SharingList *sharing_history_tail;
	struct hashTableEntry_s *next;
} HashTableEntry;


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
}


void build_hash_table (HashTableEntry *ht, FILE *fp_in) {
	/* declarations */
	int tid, hash_index, LLCsetid;
	unsigned long long block_addr, uniqueId;
	HashTableEntry *prev, *ptr;

	/* time stamp */
	uniqueId = 0;

	while (!feof(fp_in)) {
		fscanf(fp_in, "%d %llu", &tid, &block_addr);
		hash_index = block_addr  % SIZE;
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
	}
}