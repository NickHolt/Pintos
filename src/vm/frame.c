#include "vm/frame.h"
#include <debug.h>
#include <hash.h>
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

struct frame {
  struct hash_elem elem; /* Hash table element. */
  void *page;            /* Page occupying this frame. */
  struct thread *thread; /* Owner of this frame. */
};

static struct lock frame_lock;
static struct hash frame_table;

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

  if (page != NULL)
    {
      struct frame *f = malloc (sizeof (struct frame));
      if (f == NULL)
        PANIC ("Failed to allocate memory for frame.");

      f->page = page;
      f->thread = thread_current ();
      hash_insert (&frame_table, &f->elem);

      lock_release (&frame_lock);

      return page;
    }
  else
    {
      /* Eventually, eviction and swapping will take place here. */
      PANIC ("Out of frames.");
    }
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
