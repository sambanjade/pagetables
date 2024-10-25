#include "mlpt.h"
#include "config.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>
#include <malloc.h>

size_t ptbr = 0;
//defined by the config file
#define MAX_LEVELS 6
#define MIN_POBITS 4
#define MAX_POBITS 18
//error checking for the levels and bits
#if LEVELS < 1 || LEVELS > MAX_LEVELS
#error "LEVELS must be between 1 and 6 inclusive."
#endif
//error checking for the bits
#if POBITS < MIN_POBITS || POBITS > MAX_POBITS
#error "POBITS must be between 4 and 18 inclusive."
#endif
//defining the page size and the number of page table entries
#define PAGE_SIZE (1UL << POBITS)
#define PTE_COUNT (PAGE_SIZE / sizeof(size_t))
#define VALID_BIT 0x1
#define PAGE_OFFSET_MASK (PAGE_SIZE - 1)
#define ADDRESS_BITS 64
#define VPN_BITS (ADDRESS_BITS - POBITS)

static size_t level_bits[LEVELS];
static int initialized = 0;
//helper methods
static size_t* allocate_page_table(void);
static void init_level_bits(void);
static size_t calculate_index(size_t va, size_t level);

size_t translate(size_t va)
{
    if (!initialized) {
        init_level_bits(); //always initalize if its not initalized to start off 
    }

    if (ptbr == 0) { //if page table is not  allocated return -1 for invalid address
        return ~0UL;
    }

    size_t offset = va & PAGE_OFFSET_MASK;
    size_t vpn = va >> POBITS;
    size_t *current_table = (size_t *)ptbr;

    for (size_t level = 0; level < LEVELS - 1; level++) {
        size_t index = calculate_index(va, level);

        if ((current_table[index] & VALID_BIT) == 0) {
            return ~0UL;
        }

        current_table = (size_t *)(current_table[index] & ~VALID_BIT);
    }

    size_t index = calculate_index(va, LEVELS - 1);
    if ((current_table[index] & VALID_BIT) == 0) {
        return ~0UL;
    }

    size_t physical_page_number = current_table[index] >> POBITS;
    return (physical_page_number << POBITS) | offset;
}

void page_allocate(size_t va)
{
    if (!initialized) {
        init_level_bits();
    }

    if (ptbr == 0) {
        ptbr = (size_t)allocate_page_table();
        if (ptbr == 0) {
            exit(EXIT_FAILURE);
        }
    }

    size_t vpn = va >> POBITS;
    size_t *current_table = (size_t *)ptbr;

    for (size_t level = 0; level < LEVELS - 1; level++) {
        size_t index = calculate_index(va, level);

        if ((current_table[index] & VALID_BIT) == 0) {
            size_t *new_table = allocate_page_table();
            if (new_table == NULL) {
                return;
            }
            current_table[index] = (size_t)new_table | VALID_BIT;
        }

        current_table = (size_t *)(current_table[index] & ~VALID_BIT);
    }

    size_t index = calculate_index(va, LEVELS - 1);
    if ((current_table[index] & VALID_BIT) == 0) {
        current_table[index] = (size_t)allocate_page_table() | VALID_BIT;
    }
}

static void init_level_bits(void) //helper  method for initizligin the bits of the table
{
    size_t total_vpn_bits = VPN_BITS;
    size_t base_bits_per_level = total_vpn_bits / LEVELS;
    size_t remainder_bits = total_vpn_bits % LEVELS;

    for (size_t i = 0; i < LEVELS; i++) {
        level_bits[i] = base_bits_per_level;
        if (i < remainder_bits) {
            level_bits[i] += 1;
        }
    }
    initialized = 1;
}

static size_t* allocate_page_table(void)
{
    void* new_page;
    if (posix_memalign(&new_page, PAGE_SIZE, PAGE_SIZE) != 0) { //allocate mem for page table 
        perror("posix_memalign failed");
        return NULL;
    }
    memset(new_page, 0, PAGE_SIZE);
    return (size_t*)new_page;
}

static size_t calculate_index(size_t va, size_t level) //helper method for caluclating indices of the page table
{
    size_t shift_amount = POBITS + (LEVELS - 1 - level) * (POBITS - 3);
    size_t index_mask = (1 << (POBITS - 3)) - 1;
    return (va >> shift_amount) & index_mask;
}


#include <stdlib.h>


void page_deallocate(size_t va) {
    \
    size_t *current_table = (size_t *)ptbr;
    if (current_table == NULL)
     {
        return; // No tables allocated, nothing to deallocate
    }

    for (int level = 0; level < LEVELS; ++level) 
    {
        size_t index = (va >> (POBITS * (LEVELS - level - 1))) & ((1 << (POBITS - 3)) - 1);
        size_t entry = current_table[index];

        if ((entry & 1) == 0) //basic check if the entry is valid 
        {
            return; 
        }

        if (level == LEVELS - 1) 
        {
            
            free((void *)(entry & ~(size_t)0xFFF));
            current_table[index] = 0; 
        } else 
        {
            
            current_table = (size_t *)(entry & ~(size_t)0xFFF);
        }
    }
}
