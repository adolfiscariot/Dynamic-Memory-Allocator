#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEAP_CAPACITY 256

typedef struct mem_block{
	int block_size;
	int is_free;
	struct mem_block *next_block;
	char data[];
}mem_block;

typedef struct free_list{
	
};

//Create & initialize mem block
struct mem_block *create_mem_block(int is_free){
	struct mem_block *new_mem_block = calloc(1, HEAP_CAPACITY);
	if (new_mem_block == NULL){
		perror("Failed to allocate new mem block\n");
		exit(1);
	}
	new_mem_block->block_size = HEAP_CAPACITY;
	new_mem_block->is_free = is_free;
	new_mem_block->next_block = NULL;
	return new_mem_block;
}

int main(void){
	int is_free = 1;
	struct mem_block* new_mem_block = create_mem_block(is_free);
	printf("Is free: %d\n", new_mem_block->is_free);
	printf("block size: %d\n", new_mem_block->block_size);
	printf("Data is: %c\n", new_mem_block->data[0]);
}
