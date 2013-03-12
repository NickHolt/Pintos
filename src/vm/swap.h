#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "devices/block.h"
#include <bitmap.h>

struct block *block_device;
struct bitmap *swap_slot_map;

void init_swap_structures (void);
void pick_slot (void);
void free_slot (void);

#endif