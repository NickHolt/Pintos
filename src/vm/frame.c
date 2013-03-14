#include "vm/frame.h"
#include <debug.h>
#include <hash.h>
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "vm/swap.h"
#include "userprog/pagedir.h"
#include "vm/page.h"

struct frame {
  struct hash_elem elem; /* Hash table element. */
  void *page;            /* Page occupying this frame. */
  tid_t thread;          /* TID of owner of this frame. */
  uint8_t *user_addr;    /* Stored to associate frames and sup_pt entries */
  uint8_t *pt_entry;     /* The page table entry for this frame */
};

static struct lock frame_lock;
static struct hash frame_table;
static struct frame* evict_frame (void);
static struct frame* select_frame_to_evict (void);

static unsigned
frame_hash (const struct hash_elem *f_, void *aux UNUSED)
{
  struct frame *f = hash_entry (f_, struct frame, elem);
  return hash_bytes (&f->page, sizeof f->page);
}

static bool
frame_less (const struct hash_elem *a_, const struct hash_elem *b_,
            void *aux UNUSED)
{
  struct frame *a = hash_entry (a_, struct frame, elem);
  struct frame *b = hash_entry (b_, struct frame, elem);
  return a->page < b->page;
}

static void
frame_destroy (struct hash_elem *f_, void *aux UNUSED)
{
  struct frame *f = hash_entry (f_, struct frame, elem);
  free (f);
}

void
frame_init (void)
{
  lock_init (&frame_lock);
  hash_init (&frame_table, frame_hash, frame_less, NULL);
}

void
frame_done (void)
{
  hash_destroy (&frame_table, frame_destroy);
}

void *
allocate_frame (enum palloc_flags flags)
{
  lock_acquire (&frame_lock);
  void *page = palloc_get_page (flags);
  struct frame *f;

  if (page != NULL)
    {
      f = malloc (sizeof (struct frame));
      if (f == NULL)
        PANIC ("Failed to allocate memory for frame.");

      f->page = page;
      f->thread = thread_current ()->tid;
      hash_insert (&frame_table, &f->elem);

      lock_release (&frame_lock);
      return page;
    }
  else
    {
      f = evict_frame ();
      ASSERT (f != NULL);

      lock_release (&frame_lock);
      return f->page;
    }
}

/* Set the page table entry as PT_ENTRY and the user page as USER_ADDR on the
   frame with page PAGE. */
void
set_page_table_entry (void* page, uint8_t *user_addr, uint8_t *pt_entry)
{
  struct frame temp;
  struct hash_elem *e;

  temp.page = page;
  e = hash_find (&frame_table, &temp.elem);
  struct frame *res = hash_entry (e, struct frame, elem);

  res->user_addr = user_addr;
  res->pt_entry = pt_entry;
}

static struct frame*
evict_frame (void)
{
  /* Choose a frame to evict and clear it from the page directory of its owner
     thread */
  struct frame *choice = select_frame_to_evict ();
  struct thread *owner = get_thread (choice->thread);
  ASSERT (owner != NULL);

  /* Get the supplemental page table entry associated with the chosen frame */
  struct sup_page *sp = get_sup_page (&owner->supp_pt, choice->user_addr);

  if (sp->writable && pagedir_is_dirty (owner->pagedir, choice->user_addr))
    {
      size_t index = pick_slot_and_swap (choice->page);
      sp->is_swapped = true;
      sp->swap_index = index;
    }

  pagedir_clear_page (owner->pagedir, choice->user_addr);

  return choice;
}

static struct frame*
select_frame_to_evict (void)
{
  /* For now, we just get the first entry in the frame table */
  struct hash_iterator i;
  hash_first (&i, &frame_table);
  hash_next (&i);

  struct frame *choice = hash_entry (hash_cur (&i), struct frame, elem);
  return choice;
}

void
free_frame (void *page)
{
  lock_acquire (&frame_lock);

  palloc_free_page (page);

  struct frame f;
  f.page = page;
  hash_find (&frame_table, &f.elem);
  struct hash_elem *e = hash_delete (&frame_table, &f.elem);
  frame_destroy (e, NULL);

  lock_release (&frame_lock);
}
