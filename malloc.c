#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define HEAP_CAPACITY 65536 /*16 bit memory*/
#define FREE_BLOCK 0x1
#define USED_BLOCK 0x0

/*
 ========================
 * MEMORY BLOCK STRUCTURE
 ========================
 */

//Arrange from largest to smallest for good memory alignment
typedef struct mem_block{
	struct mem_block *next_block;
	int block_size;
	uint8_t is_free; //Just a byte
	char data[];
}mem_block;

/*
 =====================
 * FREE LIST STRUCTURE
 =====================
 */
typedef struct{
	struct mem_block *head_block;
}free_list;

/*
 ===============================
 * MEMORY MANIPULATION FUNCTIONS
 ===============================
 */

//Initialize mem block using calloc to ensure bytes are 0's
struct mem_block *create_mem_block(free_list *free_list){
	struct mem_block *new_mem_block = calloc(1, HEAP_CAPACITY);
	if (new_mem_block == NULL){
		perror("Failed to create new mem block\n");
		exit(1);
	}
	new_mem_block->block_size = HEAP_CAPACITY;
	new_mem_block->is_free = FREE_BLOCK;
	new_mem_block->next_block = NULL;
	
	//Make this the head mem block of the free list
	free_list->head_block = new_mem_block;

	return new_mem_block;
}

//Allocate memory from mem block
/*
 *  Assign a memory block to the user. This code uses the first fit placement policy
 *  where it finds the first memory block that's the required size.
 */
struct mem_block *alloc_mem_block(free_list *free_list, int size_to_allocate){
	
	//I AM NOT ALLOCATING FROM CREATED_MEM_BLOCK. WHY???


	/*
	 * We loop through the free list looking for the first block thats
	 * free and of the required size. We then remove it from the free list
	 * and change is_free to USED_BLOCK
	 */

	struct mem_block *preceding_mem_block = NULL;
	struct mem_block *current_mem_block = free_list->head_block;
	printf("Head block location: %p\n", current_mem_block);
	printf("Head block size: %d\n", current_mem_block->block_size);

	//Find first block
	while (current_mem_block && !(current_mem_block->block_size >= size_to_allocate && 
		current_mem_block->is_free == FREE_BLOCK)) {

		preceding_mem_block = current_mem_block;
		current_mem_block = current_mem_block->next_block;
	}

	//Check if free list is empty
	if (current_mem_block == NULL){
		fprintf(stderr, "Failed to find ideal memory block\n");
		return NULL;
	}
	
	//If the current mem block is the head block...
	if (preceding_mem_block == NULL){ 
		int remaining = current_mem_block->block_size - size_to_allocate - sizeof(struct mem_block);
		if (remaining > sizeof(struct mem_block)){
			struct mem_block *new_head_block = (struct mem_block *)((char *)current_mem_block + sizeof(struct mem_block) + size_to_allocate);

			new_head_block->is_free = FREE_BLOCK;
			new_head_block->block_size = current_mem_block->block_size - size_to_allocate;
			new_head_block->next_block = NULL;
			free_list->head_block = new_head_block;
		} else{
			free_list->head_block = current_mem_block->next_block;
		}

	}

	else {
		preceding_mem_block->next_block = current_mem_block->next_block;
		printf("Preceding mem block size: %d\n", preceding_mem_block->block_size);
	}

	current_mem_block->next_block = NULL;
	current_mem_block->is_free = USED_BLOCK;
	current_mem_block->block_size = size_to_allocate;
	printf("Current mem block size: %d\n", current_mem_block->block_size);
	return current_mem_block;
}

/*
 * Insert mem block back to free list.
 * This operation is called when a mem block is freed
 */

int free_mem_block(struct mem_block *mem_block_to_free, free_list *free_list){
	/* 
	 * Traverse the free list and find the mem block that 
	 * precedes mem_block_to_free. If its absent our block
	 * is the first one. If it exists point it to mem_block
	 * to free and point mem_block_to_free to the preceding
	 * block's previous next_block.
	 */

	struct mem_block *next_mem_block =  mem_block_to_free->next_block;

	//Loop through the list to find previous mem block
	struct mem_block *preceding_mem_block = NULL;
	struct mem_block *current_mem_block = free_list->head_block;
	while(current_mem_block && current_mem_block != mem_block_to_free){
		preceding_mem_block = current_mem_block;
		current_mem_block = current_mem_block->next_block;
	}

	if (current_mem_block == NULL){
		fprintf(stderr, "Mem block not found in free list\n");
		return 1;
	}

	if (preceding_mem_block == NULL){
		mem_block_to_free->next_block = free_list->head_block;
		free_list->head_block = mem_block_to_free;
	}
	else {
		preceding_mem_block->next_block = mem_block_to_free;
		mem_block_to_free->next_block = preceding_mem_block->next_block;
	}
	mem_block_to_free->is_free = FREE_BLOCK;
	
	printf("Freed memory block back into free list\n");
	return 0;
}

/*
 ====================
 *FREE LIST FUNCTIONS
 ====================
 */

/*
 * Traverse list
 * This operation traverses the linked list
 * Returns 0 if successful, 1 if failed
 */
int traverse_free_list(free_list *free_list){
	if (free_list->head_block->next_block == NULL){
		fprintf(stderr, "Free list is empty\n");
		return 1;
	}
	
	struct mem_block *current_mem_block = free_list->head_block;
	
	while (current_mem_block->next_block != NULL){
		current_mem_block = current_mem_block->next_block;
	}
	
	printf("End of free list\n");
	return 0;

}

int main(void){
	/*
	 * We must initialize the free list on the heap. Doing it on the stack ensures
	 * its lifetime will be tied to a specific function which is not what we want. 
	 * We want this list to exist for the entire allocator's lifetime.
	 */

	free_list *free_list = malloc(sizeof(free_list));

	struct mem_block *new_mem_block = create_mem_block(free_list);
	printf("OG Mem Block Size: %d\n", new_mem_block->block_size);

	struct mem_block *allocated_mem_block = alloc_mem_block(free_list, 4096);
	struct mem_block *second_allocated_mem_block = alloc_mem_block(free_list, 8192);
}



