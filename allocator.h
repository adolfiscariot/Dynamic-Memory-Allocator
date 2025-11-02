#ifndef ALLOCATOR_H
#define ALLOCATOR_H

struct mem_block;
typedef struct free_list {
    struct mem_block *head_block;
} free_list;

void set_boundary_tag(struct mem_block *block);
struct mem_block *create_mem_block(free_list *free_list);
struct mem_block *alloc_mem_block(free_list *free_list, int size_to_allocate);
int free_mem_block(free_list *free_list, struct mem_block *mem_block_to_free);
int traverse_free_list(free_list *free_list);

#endif
