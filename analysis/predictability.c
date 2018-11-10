#define SIZE 4194304
#define INVALID_TAG 0xfffffffffffffffULL

// #define LLC_NUMSET 8192 /* 8 MB LLC: 8 X 1024 */
// #define LLC_ASSOC 16
// #define NUM_PROC 8


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "headers/structs.h"
#include "headers/list.h"
#include "headers/hash_table.h"
#include "headers/pattern.h"
#include "headers/predictability.h"













int main (int argc, char **argv) {
	/* Declarations */
	ListEntry *head;
	unsigned long long id, block_addr, prev_block, next_block;
	int i,j, shared, hash_index, sh_len, barrel;
	HashTableEntry *ht, *prev, *ptr;
	
	char output_name[256], input_name[256];
	List *addr_list;
	Pattern **patterns;
	int index, *curr_seq;

	FILE *fp_in, *fp_out;

	int LLC_NUMSET, LLC_ASSOC;
	int num_bits, num_patterns, NUM_BITS, PREV_NBR, NEXT_NBR, 
		PREV_NBR_NUM_BITS=0, NEXT_NBR_NUM_BITS=0, 
		PREV_NBR_NUM_PATTERNS=0, NEXT_NBR_NUM_PATTERNS=0;

	ListEntry *list_head;


	/* input the params */
	/*
	argv[1] = name of input file
	argv[2] = name of output file
	argv[3] = NUM_BITS
	argv[4] = PREV_NBR
	argv[5] = PREV_NBR_NUM_BITS
	argv[6] = NEXT_NBR
	argv[7] = NEXT_NBR_NUM_BITS
	*/
	sprintf(input_name, "%s", argv[1]);
	sprintf(output_name, "%s", argv[2]);
	NUM_BITS 	= atoi(argv[3]);
	PREV_NBR 	= atoi(argv[4]);
	PREV_NBR_NUM_BITS = atoi(argv[5]);
	NEXT_NBR 	= atoi(argv[6]);
	NEXT_NBR_NUM_BITS = atoi(argv[7]);


	/* compute other params */
	PREV_NBR_NUM_PATTERNS = (1 << PREV_NBR_NUM_BITS);
	NEXT_NBR_NUM_PATTERNS = (1 << NEXT_NBR_NUM_BITS);
	num_bits = NUM_BITS;
	if (PREV_NBR) {
		num_bits += PREV_NBR_NUM_BITS;
	}
	if (NEXT_NBR) {
		num_bits += NEXT_NBR_NUM_BITS;
	}
	num_patterns = (1 << num_bits);

	patterns = (Pattern**)init_patterns(num_bits);
	addr_list = (List*)create_list();
	ht = (HashTableEntry*)create_hash_table(SIZE);

	/* display patterns */
	display_patterns(patterns, num_bits);

	fp_in = fopen(input_name, "r");
	assert(fp_in != NULL);
	
	build_hash_table(ht, fp_in, addr_list);

	fclose(fp_in);
	
	/* compute the predictability */
	fp_out = fopen(output_name, "w");

	fclose(fp_out);
	return 0;
}