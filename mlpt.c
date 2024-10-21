#include "mlpt.h"
#include "config.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>
#include <malloc.h>
#include <stdlib.h>

size_t ptbr = 0;

#define MAX_LEVELS 6
#define MIN_POBITS 4
#define MAX_POBITS 18

#if LEVELS < 1 || LEVELS > MAX_LEVELS
#error "LEVELS must be between 1 and 6 inclusive."
#endif

#if POBITS < MIN_POBITS || POBITS > MAX_POBITS
#error "POBITS must be between 4 and 18 inclusive."
#endif

#define PAGE_SIZE (1UL << POBITS)
#define PTE_COUNT (PAGE_SIZE / sizeof(size_t))
#define VALID_BIT 0x1
#define PAGE_OFFSET_MASK (PAGE_SIZE - 1)
#define ADDRESS_BITS 64
#define VPN_BITS (ADDRESS_BITS - POBITS)

static size_t level_bits[LEVELS];
static int initialized = 0;

static void init_level_bits(void)
{
    size_t total_vpn_bits = VPN_BITS;
    size_t base_bits_per_level = total_vpn_bits / LEVELS;
    size_t remainder_bits = total_vpn_bits % LEVELS;

    for (size_t i = 0; i < LEVELS; i++)
    {
        level_bits[i] = base_bits_per_level;
        if (i < remainder_bits)
        {
            level_bits[i]++;
        }
    }
    initialized = 1;
}

static size_t get_index(size_t vpn, size_t level)
{
    size_t shift = 0;
    for (size_t i = level + 1; i < LEVELS; i++)
    {
        shift += level_bits[i];
    }
    size_t mask = (1UL << level_bits[level]) - 1;
    return (vpn >> shift) & mask;
}

size_t translate(size_t va)
{
    if (!initialized)
    {
        init_level_bits();
    }

    if (ptbr == 0)
    {
        return ~0UL;
    }

    size_t offset = va & PAGE_OFFSET_MASK;
    size_t vpn = va >> POBITS;
    size_t *page_table = (size_t *)ptbr;

    for (size_t level = 0; level < LEVELS; level++)
    {
        size_t index = get_index(vpn, level);
        size_t pte = page_table[index];

        if ((pte & VALID_BIT) == 0)
        {
            return ~0UL;
        }

        size_t next_level_address = (pte >> POBITS) << POBITS;

        if (level == LEVELS - 1)
        {
            return next_level_address | offset;
        }
        else
        {
            page_table = (size_t *)next_level_address;
        }
    }

    return ~0UL;
}

void page_allocate(size_t va)
{
    if (!initialized)
    {
        init_level_bits();
    }

    if (ptbr == 0)
    {
        if (posix_memalign((void **)&ptbr, PAGE_SIZE, PAGE_SIZE) != 0)
        {
            exit(EXIT_FAILURE);
        }
        memset((void *)ptbr, 0, PAGE_SIZE);
    }

    size_t vpn = va >> POBITS;
    size_t *page_table = (size_t *)ptbr;

    for (size_t level = 0; level < LEVELS; level++)
    {
        size_t index = get_index(vpn, level);
        size_t pte = page_table[index];

        if ((pte & VALID_BIT) == 0)
        {
            void *next_level = NULL;

            if (level == LEVELS - 1)
            {
                if (posix_memalign(&next_level, PAGE_SIZE, PAGE_SIZE) != 0)
                {
                    exit(EXIT_FAILURE);
                }
                memset(next_level, 0, PAGE_SIZE);
            }
            else
            {
                if (posix_memalign(&next_level, PAGE_SIZE, PAGE_SIZE) != 0)
                {
                    exit(EXIT_FAILURE);
                }
                memset(next_level, 0, PAGE_SIZE);
            }

            size_t next_level_pn = ((size_t)next_level) >> POBITS;
            pte = (next_level_pn << POBITS) | VALID_BIT;
            page_table[index] = pte;
        }

        size_t next_level_address = (pte >> POBITS) << POBITS;

        if (level == LEVELS - 1)
        {
            break;
        }
        else
        {
            page_table = (size_t *)next_level_address;
        }
    }
}
