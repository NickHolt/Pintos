#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "devices/block.h"
#include <bitmap.h>

struct block *block_device;
struct bitmap *swap_slot_map;

void init_swap_structures (void);
size_t pick_slot_and_swap (void *page);
void free_slot (void);

#endif