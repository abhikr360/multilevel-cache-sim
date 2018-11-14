double logarithm (double x, double b) {
	return (log(x) / log(b));
}


int max (int a, int b) {
	if (a > b) return a;
	else return b;
}


int fprintf_private_predictability (FILE *fp, HashTableEntry *ht, List *addr_list) {
	ListEntry * list_head;
	unsigned long long block_addr;
	int hash_index;
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
		fprintf(fp, "%f %f\n", ptr->predictability_index, ptr->entropy);
		list_head = list_head->next;
	}
}


double get_entropy (Pattern **patterns, int num_bits) {
	int num_patterns, i, total_patterns_count, numerator, denominator;
	double prob_x_and_y, prob_x_given_y, total;

	total = 0.0;
	num_patterns = (1 << num_bits);
	total_patterns_count = 0;
	for (i=0; i<num_patterns; i++) {
		total_patterns_count += patterns[i]->shared_count + patterns[i]->private_count;
	}

	for (i=0; i<num_patterns; i++) {
		/* private */
		numerator = patterns[i]->private_count;
		denominator = total_patterns_count;
		prob_x_and_y = ( numerator* 1.0) / denominator;

		if (numerator != 0) {
			// numerator = patterns[i]->private_count;
			denominator = patterns[i]->shared_count + patterns[i]->private_count;
			prob_x_given_y = (numerator * 1.0) / denominator;
			total = total - prob_x_and_y * logarithm(prob_x_given_y, LOG_BASE);
		}

		/* shared */
		numerator = patterns[i]->shared_count;
		denominator = total_patterns_count;
		prob_x_and_y = ( numerator* 1.0) / denominator;

		if (numerator != 0) {
			// numerator = patterns[i]->shared_count;
			denominator = patterns[i]->shared_count + patterns[i]->private_count;
			prob_x_given_y = (numerator * 1.0) / denominator;
			total = total - prob_x_and_y * logarithm(prob_x_given_y, LOG_BASE);
		}
	}

	return total;
}


double get_predictability_index (Pattern **patterns, int num_bits) {
	int num_patterns, i;
	int numerator, denominator; 
	double total;

	total = 0.0;
	num_patterns = (1 << num_bits);
	for (i=0; i<num_patterns; i++) {
		numerator = max(patterns[i]->shared_count, patterns[i]->private_count);
		denominator = patterns[i]->shared_count + patterns[i]->private_count;
		if (numerator != 0) total += (numerator * 1.0) / denominator;
	}
	total = total / num_patterns;
	return total;
}


void _compute_predictability_alone (
	HashTableEntry *ht, 
	List *addr_list, 
	FILE* fp_out_global,
	int NUM_BITS) {

	Pattern **private_patterns, **global_patterns;
	int *seq;
	SharingHistoryEntry *sh;
	int i, hash_index, j, shared;
	unsigned long long block_addr;
	ListEntry *list_head;
	HashTableEntry *ptr;
	double global_entropy;

	private_patterns = (Pattern**)init_patterns(NUM_BITS);
	global_patterns = (Pattern**)init_patterns(NUM_BITS);

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

		for (j=0; j<NUM_BITS; j++) seq[j] = 0; 

		for (i=0; i+NUM_BITS<ptr->sh_len; i++) {
			for (j=0; j<NUM_BITS; j++) {
				seq[j] = ptr->sh[i+j].shared;
			}
			// for (j=0; j<NUM_BITS; j++) {
			// 	printf("%d ", seq[j]);
			// } printf("\n");
			shared = ptr->sh[i+NUM_BITS].shared;
			update_patterns(private_patterns, NUM_BITS, seq, shared);
			update_patterns(global_patterns, NUM_BITS, seq, shared);
		}
		ptr->predictability_index = get_predictability_index(private_patterns, NUM_BITS);
		ptr->entropy = get_entropy(private_patterns, NUM_BITS);
		clean_patterns(private_patterns, NUM_BITS);
		list_head = list_head->next;

	}	
	assert(list_head == NULL);
	// display_patterns(global_patterns, NUM_BITS);
	global_entropy = get_entropy(global_patterns, NUM_BITS);
	global_pred_index = get_predictability_index(global_patterns, NUM_BITS);

	fprintf(fp_out_global, "%f ", global_entropy);
	fprintf(fp_out_global, "%f\n", global_pred_index);

	free_patterns(private_patterns, NUM_BITS);
	free_patterns(global_patterns, NUM_BITS);
	free(seq);
}


void _compute_predictability_together (
	HashTableEntry *ht, 
	List *addr_list, 
	FILE *fp_out_private,
	FILE *fp_out_global,
	int NUM_BITS,
	int PREV_NBR,
	int PREV_NBR_NUM_BITS,
	int NEXT_NBR,
	int NEXT_NBR_NUM_BITS) {

	/* declarations*/
	int num_bits, num_patterns, hash_index, i, j, shared, index;
	Pattern **private_patterns, **global_patterns;
	int *seq;
	ListEntry *list_head;
	unsigned long long  prev_block, next_block, block_addr;
	HashTableEntry *ptr, *ptr_prev, *ptr_next;
	double global_entropy, global_pred_index;
	

	assert(!(PREV_NBR==0 && NEXT_NBR==0));
	num_bits = NUM_BITS;
	if (PREV_NBR) num_bits += PREV_NBR_NUM_BITS;
	if (NEXT_NBR) num_bits += NEXT_NBR_NUM_BITS;

	num_patterns = (1 << num_bits);

	private_patterns = (Pattern**)init_patterns(num_bits);
	global_patterns = (Pattern**)init_patterns(num_bits);

	seq = (int*)malloc(num_bits*sizeof(int));

	list_head = addr_list->head;
	while (list_head != NULL) {
		block_addr = list_head->block_addr;
		prev_block = block_addr - 1;
		next_block = block_addr + 1;

		hash_index = block_addr % SIZE;
		ptr = &ht[hash_index];
		while (ptr != NULL) {
			if (ptr->block_addr == block_addr) break;
			ptr = ptr->next;
		}
		assert(ptr != NULL);

		hash_index = prev_block % SIZE;
		ptr_prev = &ht[hash_index];
		while (ptr_prev != NULL) {
			if (ptr_prev->block_addr == prev_block) break;
			ptr_prev = ptr_prev->next;
		}
// if(ptr_prev == NULL) printf("MARK %llu\n", prev_block);
		// assert(ptr_prev != NULL);

		hash_index = next_block % SIZE;
		ptr_next = &ht[hash_index];
		while (ptr_next != NULL) {
			if (ptr_next->block_addr == next_block) break;
			ptr_next = ptr_next->next;
		}
		// assert(ptr_next != NULL);

		for (j=0; j<num_bits; j++) seq[j] = 0;

		for (i=0; i+NUM_BITS<ptr->sh_len; i++) {
			for (j=0; j<NUM_BITS; j++) {
				seq[j] = ptr->sh[i+j].shared;
			}
			shared = ptr->sh[i+NUM_BITS].shared;
			if (PREV_NBR == 1 && ptr_prev != NULL) {
				/* find the last index of ptr_prev->sh that has id geq ptr->sh[i+NUM_BITS].id */
				index = 0;
				while (index < ptr_prev->sh_len) {
					if (ptr_prev->sh[index].id >= ptr->sh[i+NUM_BITS].id) break;
					index++;
				}
				index--;
				j = PREV_NBR_NUM_BITS-1;
				while (index >= 0 && j >= 0) {
					seq[NUM_BITS+j] = ptr_prev->sh[index].shared;
					index--;
					j--;
				}
			}
			if (NEXT_NBR == 1 && ptr_next != NULL) {
				index = 0;
				while (index < ptr_next->sh_len) {
					if (ptr_next->sh[index].id >= ptr->sh[i+NUM_BITS].id) break;
					index++;
				}
				index--;
				j = NEXT_NBR_NUM_BITS-1;
				while (index >= 0 && j >= 0) {
					seq[NUM_BITS+PREV_NBR_NUM_BITS+j] = ptr_next->sh[index].shared;
					index--;
					j--;
				}
			}
			update_patterns(private_patterns, num_bits, seq, shared);
			update_patterns(global_patterns, num_bits, seq, shared);
		}
		ptr->predictability_index = get_predictability_index(private_patterns, num_bits);
		ptr->entropy = get_entropy(private_patterns, num_bits);
		clean_patterns(private_patterns, num_bits);
		list_head = list_head->next;
	}

	global_entropy = get_entropy(global_patterns, num_bits);
	global_pred_index = get_predictability_index(global_patterns, num_bits);
	fprintf(fp_out_global, "%f ", global_entropy);
	fprintf(fp_out_global, "%f\n", global_pred_index);

	free_patterns(private_patterns, num_bits);
	free_patterns(global_patterns, num_bits);
	free(seq);
}


void compute_predictability (
	HashTableEntry *ht, 
	List *addr_list, 
	FILE *fp_out_private,
	FILE *fp_out_global,
	int NUM_BITS,
	int PREV_NBR,
	int PREV_NBR_NUM_BITS,
	int NEXT_NBR,
	int NEXT_NBR_NUM_BITS) {
	
	
	if (PREV_NBR == 0 && NEXT_NBR == 0) 
		_compute_predictability_alone(ht, addr_list, fp_out_global, NUM_BITS);
	else 
		// assert(0);
		_compute_predictability_together(
			ht, addr_list, fp_out_private, fp_out_global,
			NUM_BITS, PREV_NBR, PREV_NBR_NUM_BITS,
			NEXT_NBR, NEXT_NBR_NUM_BITS);
	
	fprintf_private_predictability(fp_out_private, ht, addr_list);
}