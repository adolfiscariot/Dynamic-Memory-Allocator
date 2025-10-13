#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define HEAP_CAPACITY 65536 /*16 bit memory*/

#define FREE_BLOCK 0x1
#define USED_BLOCK 0x0

#define BLOCK_SIZE_MASK 0xFFFFFFFE
#define BLOCK_FREE_MASK 0x1

#define GET_SIZE(block) ((block)->size_and_flags & BLOCK_SIZE_MASK)
#define IS_FREE(block) ((block)->size_and_flags & BLOCK_FREE_MASK)

#define SET_SIZE(block, size) ((block)->size_and_flags = ((size) & BLOCK_SIZE_MASK) | ((block)->size_and_flags & BLOCK_FREE_MASK))
#define SET_FREE(block, free) ((block)->size_and_flags = ((block)->size_and_flags & BLOCK_SIZE_MASK) | ((free) & BLOCK_FREE_MASK))

/*
 ========================
 * MEMORY BLOCK STRUCTURE
 ========================
 */

//Arrange from largest to smallest for good memory alignment
typedef struct mem_block{
	struct mem_block *next_block;
	unsigned int size_and_flags;
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
	struct mem_block *new_mem_block = calloc(1, sizeof(struct mem_block) + HEAP_CAPACITY);
	if (new_mem_block == NULL){
		perror("Failed to create new mem block\n");
		exit(1);
	}

	SET_SIZE(new_mem_block, HEAP_CAPACITY);
	SET_FREE(new_mem_block, FREE_BLOCK)

	new_mem_block->next_block = NULL;
	free_list->head_block = new_mem_block;

	return new_mem_block;
}

//Allocate memory from mem block
/*
 *  Assign a memory block to the user. This code uses the first fit placement policy
 *  where it finds the first memory block that's the required size.
 */
struct mem_block *alloc_mem_block(free_list *free_list, int size_to_allocate){
	
	/*
	 * We loop through the free list looking for the first block thats
	 * free and of the required size. We then remove it from the free list
	 * and change is_free to USED_BLOCK. If the desired block is the head
	 * block in our list, we shift the block pointer up by the allocated
	 * size + sizeof(mem_block) and that becomes the beginning of our new 
	 * head block.
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
		int total_space_used = size_to_allocate + sizeof(struct mem_block);
		int remaining_size = current_mem_block->block_size - total_space_used;

		if (remaining_size > sizeof(struct mem_block)){
			struct mem_block *new_head_block = (struct mem_block *)((char *)current_mem_block + total_space_used;

			new_head_block->is_free = FREE_BLOCK;
			new_head_block->block_size = remaining_size;
			new_head_block->next_block = current_mem_block->next_block;
			free_list->head_block = new_head_block;
			current_mem_block->block_size = size_to_allocate;
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
	
	//Return just the data section after the the metadata
	return (struct mem_block *)((char *)current_mem_block + sizeof(struct mem_block));
}

/*
 * Insert mem block back to free list.
 * This operation is called when a mem block is freed
 */

int free_mem_block(struct mem_block *mem_block_to_free, free_list *free_list){
	/*
	 * Take the memory to be freed, 
	 */
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
	if (!free_list->head_block){
		printf("Free list is empty\n");
		return 0;
	}
	
	struct mem_block *current_mem_block = free_list->head_block;
	
	int num_of_free_blocks = 1; //We passed the above check so a free block exists
	while (current_mem_block->next_block != NULL){
		num_of_free_blocks ++;
		current_mem_block = current_mem_block->next_block;
	}
	
	printf("Num of free blocks: %d\n", num_of_free_blocks);
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

	struct mem_block *allocated_mem_block = alloc_mem_block(free_list, 4096);
	struct mem_block *second_allocated_mem_block = alloc_mem_block(free_list, 8192);

	traverse_free_list(free_list);
}



