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

/*
 * Insert mem block back in free list.
 * Insertions are always done at the front of the free list and coalesce the next
 * block if free
 */
int insert_into_free_list(free_list *free_list, struct mem_block *mem_block_to_insert){
	if (!free_list && !mem_block_to_insert) return 1;

	//If free list is empty
	struct mem_block *head_block = free_list->head_block;

	if (head_block == NULL){
		free_list->head_block = mem_block_to_insert;
		mem_block_to_insert->next_block = NULL;
	} else {
		mem_block_to_insert->next_block = free_list->head_block;
		free_list->head_block = mem_block_to_insert;
	}

	struct mem_block *new_block = mem_block_to_insert;

	//Coalesce with next block if free
	unsigned int mem_block_to_insert_size = GET_SIZE(mem_block_to_insert);
	struct mem_block *next_block = (struct mem_block *)((char *)mem_block_to_insert + sizeof(struct mem_block) + mem_block_to_insert_size + sizeof(unsigned int));
	unsigned int next_block_is_free = IS_FREE(next_block);

	//If next block is free merge it with the block we're inserting
	if (next_block_is_free){
		unsigned int next_block_size = GET_SIZE(next_block);
		unsigned int new_block_size = mem_block_to_insert_size + sizeof(unsigned int) + sizeof(struct mem_block) + next_block_size;


		SET_SIZE(new_block, new_block_size);
		SET_FREE(new_block, FREE_BLOCK);

		unsigned int *new_block_footer = (unsigned int *)((char *)new_block + sizeof(struct mem_block) + new_block_size);
		*new_block_footer = new_block->size_and_flags;
	}

	printf("The freed block's location is: %p\n", new_block);

	return 0;
}


/*
 ===============================
 * MEMORY MANIPULATION FUNCTIONS
 ===============================
 */

//Create boundary tag for each created block
void set_boundary_tag(struct mem_block *block){
	unsigned int size = GET_SIZE(block);
	unsigned int *footer = (unsigned int *)((char *)block + sizeof(struct mem_block) + size);
	*footer = block->size_and_flags;
}

//Initialize mem block (use calloc to ensure bytes are 0's)
struct mem_block *create_mem_block(free_list *free_list){
	struct mem_block *new_mem_block = calloc(1, sizeof(struct mem_block) + HEAP_CAPACITY);
	if (new_mem_block == NULL){
		perror("Failed to create new mem block\n");
		exit(1);
	}

	SET_SIZE(new_mem_block, HEAP_CAPACITY);
	SET_FREE(new_mem_block, FREE_BLOCK);

	set_boundary_tag(new_mem_block);
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
	 * free and of the required size. We then split it from the free block
	 * and remove it from the free list. If the desired block is the head
	 * block in our list, we shift the block pointer up by the allocated
	 * size + sizeof(mem_block) and that becomes the beginning of our new 
	 * head block.
	 */

	struct mem_block *preceding_mem_block = NULL;
	struct mem_block *current_mem_block = free_list->head_block;
	printf("New Head block location: %p\n", current_mem_block);
	printf("New Head block size: %d\n", GET_SIZE(current_mem_block));

	//Find first block
	while (current_mem_block && !(GET_SIZE(current_mem_block) >= size_to_allocate && 
		IS_FREE(current_mem_block))) {

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
		int remaining_size = GET_SIZE(current_mem_block) - total_space_used;

		if (remaining_size > sizeof(struct mem_block)){
			struct mem_block *new_head_block = (struct mem_block *)((char *)current_mem_block + total_space_used);

			SET_FREE(new_head_block, FREE_BLOCK);
			SET_SIZE(new_head_block, remaining_size);
			new_head_block->next_block = current_mem_block->next_block;
			free_list->head_block = new_head_block;
			SET_SIZE(current_mem_block, size_to_allocate);
		} else{
			free_list->head_block = current_mem_block->next_block;
		}
	} else {
		preceding_mem_block->next_block = current_mem_block->next_block;
		printf("Preceding mem block size: %d\n", GET_SIZE(preceding_mem_block));
	}

	current_mem_block->next_block = NULL;
	SET_FREE(current_mem_block, USED_BLOCK);
	SET_SIZE(current_mem_block, size_to_allocate);
	printf("Allocated mem block location: %p\n", current_mem_block);
	printf("Allocated mem block size: %d\n", GET_SIZE(current_mem_block));
	
	//Return just the data section after the the metadata
	return (struct mem_block *)((char *)current_mem_block + sizeof(struct mem_block));
}


//Free an allocated memory block
/*
 * This function will free a memory block and apply coalescing to reduce memory
 * fragmentation. This will be done using the help of each allocated block's boundary
 * tag. mem_block_to_free points to the block's data and not header section. Returns 1
 * if there's an error and 0 if everything goes well i.e. memory block is freed, coalesced
 * if possible and inserted into the free list.
 */
int free_mem_block(free_list *free_list, struct mem_block *mem_block_to_free){
	if (!mem_block_to_free){
		return 1;
	}

	// 1. Get block header
	struct mem_block *mem_block = (struct mem_block *)((char *)mem_block_to_free - sizeof(struct mem_block));
	SET_FREE(mem_block, FREE_BLOCK);

	struct mem_block *new_block = mem_block;
	unsigned int new_block_size = GET_SIZE(new_block);


	// 2. If previous block exists and is free, coalesce
	unsigned int *preceding_block_footer = (unsigned int *)((char *)mem_block - sizeof(unsigned int));
	unsigned int preceding_block_size_and_flags = *preceding_block_footer; //To avoid manipulating the actual pointer
	unsigned int preceding_block_size = preceding_block_size_and_flags & BLOCK_SIZE_MASK;
	unsigned int preceding_block_is_free = preceding_block_size_and_flags & BLOCK_FREE_MASK;

	if (preceding_block_is_free == 1){
		struct mem_block *start_of_preceding_block = (struct mem_block *)((char *)preceding_block_footer - preceding_block_size - (char *)sizeof(struct mem_block));
		
		//new_block is now the preceding block and not the one to free
		new_block = start_of_preceding_block;
		new_block_size += preceding_block_size + sizeof(unsigned int) + sizeof(struct mem_block);

		SET_SIZE(new_block, new_block_size);
		SET_FREE(new_block, FREE_BLOCK);

		//Set new boundary tag
		unsigned int *new_block_footer = (unsigned int *)((char *)sizeof(struct mem_block) + new_block_size);
		*new_block_footer = new_block->size_and_flags;
	}

	// 3. If next block exists and is free, coalesce too.
	struct mem_block *next_block = (struct mem_block *)((char *)new_block + sizeof(struct mem_block) + new_block_size + sizeof(unsigned int));
	unsigned int next_block_is_free = next_block->size_and_flags & BLOCK_FREE_MASK; 

	if (next_block_is_free){
		unsigned int next_block_size = GET_SIZE(next_block);

		new_block_size += sizeof(struct mem_block) + next_block_size + sizeof(unsigned int);

		SET_SIZE(new_block, new_block_size);
		SET_FREE(new_block, FREE_BLOCK);

		//Set new boundary tag
		unsigned int *new_block_footer = (unsigned int *)((char *)new_block + sizeof(struct mem_block) + new_block_size);
		*new_block_footer = new_block->size_and_flags;
	}

	// 4. Insert new_block in free list
	insert_into_free_list(free_list, new_block);
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

	int result = free_mem_block(free_list, allocated_mem_block);
	if (result == 0) printf("Freeing worked\n");
	else printf("Freeing failed\n");
}



