/*********** Author: Mainak Chaudhuri; Manish Kumar Bera **************/

#define SIZE 4194304
#define INVALID_TAG 0xfffffffffffffffULL

// #define LLC_NUMSET 8192 /* 8 MB LLC: 8 X 1024 */
// #define LLC_NUMSET 4096 // 4 MB LLC
#define LLC_ASSOC 16
#define NUM_PROC 8

// #define LLC_BLOCK_SIZE 64
// #define LOG_LLC_BLOCK_SIZE 6
int LLC_NUMSET;

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "headers/hash_table.h"
#include "headers/cache.h"


int main (int argc, char **argv)
{
	// declarations
	MetaData *metadata;
	char output_name[256], input_name[256], history_file_name[256];
	FILE *fp_in; FILE *fp_out; FILE *fp_history;
	HashTableEntry *ht, *prev, *ptr;
	CacheTag** LLCcache;

	int i;

	if (argc != 5) {
		printf("Ne1ed 3 arguments: input file. Aborting...\n");
		exit (1);
	}

	LLC_NUMSET = atoi(argv[4]);
	metadata = (MetaData*)create_metadata();
	LLCcache = (CacheTag**)create_cache(LLC_NUMSET, LLC_ASSOC);
	ht = (HashTableEntry*)create_hash_table(SIZE);

	/* Build the hash table of accesses */
	sprintf(input_name, "%s", argv[1]);
	fp_in = fopen(input_name, "r");
	assert(fp_in != NULL);

	build_hash_table(ht, fp_in);

	fclose(fp_in);
	printf("Done reading file!\n");

	printf("Access list prepared.\nStarting simulation...\n"); fflush(stdout);

	// Simulate
	fp_in = fopen(input_name, "r");
	assert(fp_in != NULL);
	simulate(LLCcache, ht, fp_in, metadata);
	fclose(fp_in);

	printf("Done reading file!\n");

	sprintf(output_name, "%s", argv[2]);
	fp_out = fopen(output_name, "w");
	assert(fp_out != NULL);
	fprintf(fp_out, "%llu\n", 
		metadata->total_miss_count);
	fprintf(fp_out, "%llu %llu %llu\n", 
		metadata->count_shared_reuse_fills, 
		metadata->count_private_reuse_fills, 
		metadata->count_no_reuse_fills);
	fprintf(fp_out, "%llu %llu\n", 
		metadata->count_shared_hits, 
		metadata->count_private_hits);
	fprintf(fp_out, "%llu %llu\n", 
		metadata->count_shared_reuse, 
		metadata->count_private_reuse);
	for (i=0; i<NUM_PROC; i++) {
		fprintf(fp_out, "%llu ", 
			metadata->count_sharers[i]);
	} fprintf(fp_out, "\n" );
	fclose(fp_out);

	sprintf(history_file_name, "%s", argv[3]);
	fp_history = fopen(history_file_name, "w");
	assert(fp_history != NULL);
	fprint_sharing_history(ht, fp_history);
	fclose(fp_history);
	return 0;
}
