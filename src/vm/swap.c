#include "vm/swap.h"
#include "threads/vaddr.h"

void
init_swap_structures (void)
{
    /* Initilaise the BLOCK_SWAP device */
    block_device = block_get_role (BLOCK_SWAP);

    /* Calculate how big we need to make the bitmap */
    size_t size = block_size (block_device) / (PGSIZE / BLOCK_SECTOR_SIZE);
    if ((swap_slot_map = bitmap_create (size)) == NULL)
        PANIC ("Could not allocate memory for swap table");
}

void
pick_slot (void)
{

}

void
free_slot (void)
{

}