
void _compute_predictability_alone (
	HashTableEntry *ht, 
	List *addr_list, 
	FILE* fp_out,
	int NUM_BITS) {

	Pattern **patterns;
	int *seq;
	SharingHistoryEntry *sh;
	int i, hash_index, block_addr, j, shared;
	ListEntry *list_head;
	HashTableEntry *ptr;

	patterns = (Pattern**)init_patterns(NUM_BITS);
	seq = (int*)malloc(NUM_BITS*sizeof(int));

	list_head = addr_list->head;
	while (list_head != NULL) {
		block_addr = list_head->block_addr;

		hash_index = block_addr % SIZE;
		ptr = &ht[hash_index];
		while (ptr != NULL) {
			if (ptr->block_addr == block_addr) break;
			ptr = ptr->next;
		}
		assert(ptr != NULL);

		for (j=0; j<NUM_BITS; j++) {
			seq[j] = 0;
		}

		for (i=0; i+NUM_BITS<ptr->sh_len; i++) {
			for (j=0; j<NUM_BITS; j++) {
				seq[j] = ptr->sh[i+j].shared;
			}
			shared = ptr->sh[i+NUM_BITS].shared;
			update_patterns(patterns, NUM_BITS, seq, shared);
		}

		list_head = list_head->next;
	}	

	assert(list_head == NULL);

	free_patterns(patterns, NUM_BITS);
	free(seq);
}


void _compute_predictability_together () {

}


void compute_predictability (
	HashTableEntry *ht, 
	List *addr_list, 
	FILE* fp_out,
	int NUM_BITS,
	int PREV_NBR,
	int PREV_NBR_NUM_BITS,
	int NEXT_NBR,
	int NEXT_NBR_NUM_BITS) {
	
	
	if (PREV_NBR == 0 && NEXT_NBR == 0) 
		_compute_predictability_alone(ht, addr_list, fp_out, NUM_BITS);
	else 
		assert(0);
	
	SharingHistoryEntry *sh;
	int i, hash_index, block_addr;
	ListEntry *list_head;
	HashTableEntry *ptr;

	list_head = addr_list->head;
	while (list_head != NULL) {
		block_addr = list_head->block_addr;

		hash_index = block_addr % SIZE;
		ptr = &ht[hash_index];
		while (ptr != NULL) {
			if (ptr->block_addr == block_addr) break;
			ptr = ptr->next;
		}
		assert(ptr != NULL);


		list_head = list_head->next;
	}	

	assert(list_head == NULL);
}