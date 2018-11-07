#define SIZE 4194304
#define INVALID_TAG 0xfffffffffffffffULL

#define LLC_NUMSET 8192 /* 8 MB LLC: 8 X 1024 */
#define LLC_ASSOC 16
#define NUM_PROC 8

// #define W 2
// #define NUM_PATTERNS 4

#define W 4
#define NUM_PATTERNS 16

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


typedef struct listEntry_s {
	unsigned long long block_addr;
	struct listEntry_s *next;
} ListEntry;


typedef struct list_s {
	ListEntry *head;
	ListEntry *tail;
} List;


List* create_list () {
	List* l;
	l = (List*)malloc(sizeof(List));
	l->head = NULL;
	l->tail = NULL;

	return l;
}


void ListInsert (List *l, unsigned long long entry) {
	assert(l != NULL);
	if (l->head == NULL) {
		assert(l->tail == NULL);
		l->head = (ListEntry*)malloc(sizeof(ListEntry));
		l->tail = l->head;
		l->tail->block_addr = entry;
		l->tail->next = NULL;
	}
	else {
		assert(l->tail->next == NULL);
		l->tail->next = (ListEntry*)malloc(sizeof(ListEntry));
		l->tail = l->tail->next;
		l->tail->block_addr = entry;
		l->tail->next = NULL;
	}
}


typedef struct sharingList_s {
   int shared;
   unsigned long long id;
   struct sharingList_s *next;
} SharingList;


typedef struct hashTableEntry_s {
   unsigned long long block_addr;
   SharingList *sh_head;
   SharingList *sh_tail;
   double pred;
   double entropy;

   struct hashTableEntry_s *next;
} HashTableEntry;


HashTableEntry* create_hash_table(int size) {
/* create a hash table of (size=size) */
   int j;
   HashTableEntry* ht;
   ht = (HashTableEntry*)malloc(SIZE*sizeof(HashTableEntry));
   assert(ht != NULL);
   for (j=0; j<SIZE; j++) {
      ht[j].sh_head = NULL;
      ht[j].sh_tail = NULL;
      ht[j].block_addr = INVALID_TAG;
   }
}

int main (int argc, char **argv) {
	unsigned long long id, block_addr;
	int i,j, shared, hash_index, sh_len;
	HashTableEntry *ht, *prev, *ptr;
	ht = (HashTableEntry*)create_hash_table(SIZE);
	char output_name[256], input_name[256];
	List *addr_list;
	int **patterns;

	FILE *fp_in, *fp_out;

	patterns = (int**)malloc(NUM_PATTERNS*sizeof(int*));
	for (i=0; i<NUM_PATTERNS; i++) {
		patterns[i] = (int*)malloc(W*sizeof(int));
		for (j=0; j<W; j++) {
			patterns[i][j] = i % (2 << j);
		}
	}

	for (i=0; i<NUM_PATTERNS; i++) {
		for (j=0; j<NUM_PATTERNS; j++) {
			printf("%d ", patterns[i][j]);
		}
		printf("\n");
	}

	return 0;
	addr_list = (List*)create_list();

  	if (argc != 3) {
      printf("Need 2 arguments: input file. Aborting...\n");
      exit (1);
   	}

	sprintf(input_name, "%s", argv[1]);
	sprintf(output_name, "%s", argv[2]);

	fp_in = fopen(input_name, "r");
	assert(fp_in != NULL);

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
					assert(ptr->sh_head != NULL);
					assert(ptr->sh_tail->next == NULL);
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
				ptr->sh_head = NULL;
				ptr->sh_tail = NULL;
			}
		}

		for (i=0; i<sh_len; i++) {
			fscanf(fp_in, "%d %llu", &shared, &id);
			if (ptr->sh_head == NULL) {
				assert(ptr->sh_tail == NULL);
				ptr->sh_head = (SharingList*)malloc(sizeof(SharingList));
				ptr->sh_tail = ptr->sh_head;
				ptr->sh_tail->next = NULL;
				ptr->sh_tail->shared = shared;
				ptr->sh_tail->id = id;
			}
			else {
				assert(ptr->sh_tail != NULL);
				assert(ptr->sh_tail->next == NULL);
				ptr->sh_tail->next = (SharingList*)malloc(sizeof(SharingList));
				ptr->sh_tail = ptr->sh_tail->next;
				ptr->sh_tail->shared = shared;
				ptr->sh_tail->id = id;
				ptr->sh_tail->next = NULL;
			}
		}
	}


	/* compute the predictability */


	
	return 0;
}