typedef struct pattern_s {
	int *seq;
	int private_count;
	int shared_count;

} Pattern;


Pattern** init_patterns (int num_bits) {
/* allocates memory and initializes*/
	int i, j, num_patterns, barrel;
	Pattern **patterns;

	num_patterns = (1 << num_bits);
	patterns = (Pattern**)malloc(num_patterns*sizeof(Pattern*));
	for (i=0; i<num_patterns; i++) {
		patterns[i] = (Pattern*)malloc(sizeof(Pattern));
		patterns[i]->seq = (int*)malloc(num_bits*sizeof(int));
		barrel = i;
		for (j=0; j<num_bits; j++) {
			patterns[i]->seq[j] = barrel % 2;
			barrel = barrel >> 1;
		}
		patterns[i]->private_count = 0;
		patterns[i]->shared_count = 0;
	}
	return patterns;
}

void update_patterns (Pattern **patterns, int num_bits, int* seq, int shared) {
/* checks which pattern matches the sequence in seq;
then updates the count accordingly*/
	int num_patterns, i, j;
// printf("MARK\n");
	assert(shared == 0 || shared == 1);
	num_patterns = (1 << num_bits);
	for (i=0; i<num_patterns; i++) {
		for (j=0; j<num_bits; j++) {
			if (seq[j] != patterns[i]->seq[j]) break;
		}
		if (j == num_bits) {
			if (shared == 0)
				patterns[i]->private_count += 1;
			else
				patterns[i]->shared_count += 1;
			break;
		}
	}
	assert(i < num_patterns);
}

void clean_patterns (Pattern **patterns, int num_bits) {
/* reinitializes the counts in patterns */
	int i, num_patterns;
	num_patterns = (1 << num_bits);
	for (i=0; i<num_patterns; i++) {
		patterns[i]->private_count = 0;
		patterns[i]->shared_count = 0;
	}
}


void display_patterns (Pattern** patterns, int num_bits) {
/* displays the sequences in patterns */
	int i, j;
	int num_patterns = (1 << num_bits);
	for (i=0; i<num_patterns; i++) {
		for (j=0; j<num_bits; j++) {
			printf("%d ", patterns[i]->seq[j]);
		}
		printf(" -- %d %d\n", 
			patterns[i]->private_count, patterns[i]->shared_count);
	}
}

void free_patterns (Pattern **patterns, int num_bits) {
	int num_patterns, i;

	num_patterns = (1 << num_bits);
	for (i=0; i<num_patterns; i++) {
		free(patterns[i]->seq);
		free(patterns[i]);
	}
	free(patterns);
}