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
