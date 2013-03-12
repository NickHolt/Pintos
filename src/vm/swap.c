#include "vm/swap.h"
#include "threads/vaddr.h"

/* This is always 2, but for neatness we define it here */
#define SECTORS_PER_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)

void
init_swap_structures (void)
{
  /* Initilaise the BLOCK_SWAP device */
  block_device = block_get_role (BLOCK_SWAP);

  /* Calculate how big we need to make the bitmap. Everything is initialised 
     to false in the bitmap, which is the default behaviour in the underlying 
     ADT */
  size_t size = block_size (block_device) / SECTORS_PER_PAGE;
  if ((swap_slot_map = bitmap_create (size)) == NULL)
      PANIC ("Could not allocate memory for swap table");
}

size_t
pick_slot_and_swap (void *page)
{
  size_t index = bitmap_scan_and_flip (swap_slot_map, 0, 1, false);

  /* Write the ith BLOCK_SECTOR_SIZE bytes of the page to the block device. We 
     have a mapping of SECTORS_PER_PAGE block sectors for every bit in the 
     bitmap, so we can just multiply the index we got back by SECTORS_PER_PAGE 
     to find the correct block sector to write to, and then increment this 
     block sector index in each loop iteration */
  int i = 0;
  for (; i < SECTORS_PER_PAGE; ++i)
    {
      block_write (block_device, (index * SECTORS_PER_PAGE) + i, 
                   page + (BLOCK_SECTOR_SIZE * i));
    }

  return index;
}

void
free_slot (void)
{

}