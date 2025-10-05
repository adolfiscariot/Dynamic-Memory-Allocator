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
	uint8_t is_free;
	char data[];
}mem_block;

/*
 ===========
 * FREE LIST
 ===========
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
struct mem_block *create_mem_block(uint8_t is_free, free_list *free_list){
	struct mem_block *new_mem_block = calloc(1, HEAP_CAPACITY);
	if (new_mem_block == NULL){
		perror("Failed to create new mem block\n");
		exit(1);
	}
	new_mem_block->block_size = HEAP_CAPACITY;
	new_mem_block->is_free = is_free;
	new_mem_block->next_block = NULL;
	
	//Make this the head mem block of the free list
	free_list->head_block = new_mem_block;

	return new_mem_block;
}
//Allocate memory from mem block
int allocate_mem(struct mem_block *mem_block){
	printf("Allocating memory from the mem block...\n");
	//Traverse free list until we find free block
}
//Check if mem block size > mem to allocate


/*
 =====================
 *FREE LIST OPERATIONS
 =====================
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

/*
 * Insert back to free list.
 * This operation is called when a mem block is freed

 */

int free_mem_block(struct mem_block *mem_block_to_free, free_list *free_list){
	/* 
	 * Get the next mem block. If it exists, use it to 
	 * get the block that precedes it. If not, our block
	 * is the last one. If preceding block exists, point
	 * it to freed block. If not, our block is the first
	 * one.
	 */

	struct mem_block *next_mem_block =  mem_block_to_free->next_block;

	//Loop through the list to find previous mem block
	struct mem_block *preceding_mem_block = NULL;
	struct mem_block *current_mem_block = free_list->head_block;
	while(current_mem_block && current_mem_block != mem_block_to_free){
		preceding_mem_block = current_mem_block;
		current_mem_block = current_mem_block->next_block;
	}


	//TO BE CONTINUED FROM HERE
	if (next_mem_block == NULL){

	}
	

}

int main(void){
	free_list *free_list;
	struct mem_block *mem_block = create_mem_block(FREE_BLOCK, free_list);
	traverse_free_list(free_list);
}

