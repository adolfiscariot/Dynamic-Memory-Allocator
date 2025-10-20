#include <stdio.h>
#include <stdlib.h>
#include "malloc.h"  

// Helper macro
#define RUN_TEST(name) printf("\n=== Running %s ===\n", #name); name()

// Global free list instance for all tests
static free_list global_free_list;

// Forward declarations of tests
void test_create_mem_block();
void test_alloc_small_block();
void test_alloc_and_free_block();
void test_coalescing_behavior();
void test_traverse_free_list();

int main(void) {
    printf("=== Running allocator tests ===\n");

    RUN_TEST(test_create_mem_block);
    RUN_TEST(test_alloc_small_block);
    RUN_TEST(test_alloc_and_free_block);
    RUN_TEST(test_coalescing_behavior);
    RUN_TEST(test_traverse_free_list);

    printf("\n=== All tests completed ===\n");
    return 0;
}

// =============================================================
// Test 1: Initialization
// =============================================================
void test_create_mem_block() {
    free_list fl = {0};
    void *payload = create_mem_block(&fl);

    if (payload == NULL) {
        printf("❌ Failed: create_mem_block returned NULL\n");
        return;
    }
    if (fl.head_block == NULL) {
        printf("❌ Failed: free_list head_block is NULL\n");
        return;
    }

    printf("✅ create_mem_block created heap at %p, payload at %p\n",
           (void *)fl.head_block, payload);
}

// =============================================================
// Test 2: Allocate small block
// =============================================================
void test_alloc_small_block() {
    free_list fl = {0};
    create_mem_block(&fl);

    struct mem_block *block = alloc_mem_block(&fl, 64);
    if (!block) {
        printf("❌ alloc_mem_block failed to allocate 64 bytes\n");
        return;
    }

    printf("✅ Allocated small block at %p\n", (void *)block);
    traverse_free_list(&fl);
}

// =============================================================
// Test 3: Allocate and free block
// =============================================================
void test_alloc_and_free_block() {
    free_list fl = {0};
    create_mem_block(&fl);

    struct mem_block *block = alloc_mem_block(&fl, 128);
    if (!block) {
        printf("❌ Allocation failed\n");
        return;
    }

    printf("Allocated block at %p\n", (void *)block);

    int res = free_mem_block(&fl, block);
    if (res != 0) {
        printf("❌ free_mem_block failed (code %d)\n", res);
    } else {
        printf("✅ Freed block successfully, checking list:\n");
        traverse_free_list(&fl);
    }
}

// =============================================================
// Test 4: Coalescing behavior
// =============================================================
void test_coalescing_behavior() {
    free_list fl = {0};
    create_mem_block(&fl);

    struct mem_block *block1 = alloc_mem_block(&fl, 128);
    struct mem_block *block2 = alloc_mem_block(&fl, 256);

    printf("Allocated two blocks: %p and %p\n", (void *)block1, (void *)block2);

    free_mem_block(&fl, block1);
    free_mem_block(&fl, block2);

    printf("✅ Freed both blocks. Expected: coalesced into one large block.\n");
    traverse_free_list(&fl);
}

// =============================================================
// Test 5: Free list traversal (sanity)
// =============================================================
void test_traverse_free_list() {
    free_list fl = {0};
    create_mem_block(&fl);
    alloc_mem_block(&fl, 256);
    traverse_free_list(&fl);
}

