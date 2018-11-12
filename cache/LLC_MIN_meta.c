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
#include "headers/hash_table.h"
#include "headers/cache.h"


int main (int argc, char **argv)
{
	MetaData *metadata;

	int shared_reuse_flag, private_reuse_flag;
	int temp=0;

	int  i, LLCsetid, maxindex, tid;
	unsigned long long block_addr, max, uniqueId; 

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

	metadata = (MetaData*)malloc(sizeof(MetaData));
	LLCcache = (CacheTag**)create_cache(LLC_NUMSET, LLC_ASSOC);
	ht = (HashTableEntry*)create_hash_table(SIZE);

	/* The following counter is used to find the sequence number for an access to a set;
	This sequence number acts as the timestamp for the access */

	/* Build the hash table of accesses */
	sprintf(input_name, "%s", argv[1]);
	sprintf(output_name, "%s", argv[2]);
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



	
	fp_out = fopen(output_name, "w");
	assert(fp_out != NULL);
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
	fprint_sharing_history(ht, fp_history);


	fclose(fp_history);
	return 0;
}
