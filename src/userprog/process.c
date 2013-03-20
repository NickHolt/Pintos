#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#ifdef VM
#include "vm/frame.h"
#include "vm/page.h"
#include "userprog/syscall.h"
#endif

static thread_func start_process NO_RETURN;
static bool load (const char *cmdline, void (**eip) (void), void **esp);
static unsigned sup_pt_hash_func (const struct hash_elem *elem, void *aux);
static bool sup_pt_less_func (const struct hash_elem *a,
                              const struct hash_elem *b, void *aux);

/* Starts a new thread running a user program loaded from
   FILENAME.  The new thread may be scheduled (and may even exit)
   before process_execute() returns.  Returns the new process's
   thread id, or TID_ERROR if the thread cannot be created. */
tid_t
process_execute (const char *file_name)
{
  char *fn_copy;
  tid_t tid;

  /* Make a copy of FILE_NAME.
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page (0);
  if (fn_copy == NULL)
    return TID_ERROR;
  strlcpy (fn_copy, file_name, PGSIZE);

  int i = 0;
  char *sep = " ";
  char *last;
  char **args = calloc ((strlen (fn_copy) / 2) + 1, sizeof (char *));

  for (args[i] = strtok_r (fn_copy, sep, &last); i < MAXARGS && args[i];
       args[++i] = strtok_r (NULL, sep, &last))
    {
      char *string = calloc (strlen(args[i]), sizeof(char));
      strlcpy (string, args[i], strlen (args[i]) + 1);
      args[i] = string;
    }

  palloc_free_page (fn_copy);

  /* Create a new thread to execute FILE_NAME. */
  tid = thread_create (args[0], PRI_DEFAULT, start_process, args);
  if (tid == TID_ERROR)
    {
      struct thread *curr = thread_current ();
      curr->child_status = FAILED;

      lock_acquire (&curr->cond_lock);
      cond_signal (&curr->child_waiter, &curr->cond_lock);
      lock_release (&curr->cond_lock);
    }
  else
    {
      /* Create child_info associated with t */
      struct child_info *t_info;
      t_info = calloc (sizeof *t_info, 1);
      if (t_info == NULL)
        PANIC ("Failed to allocate memory for thread child information");

      t_info->id = tid;
      t_info->has_exited = false;
      t_info->has_waited = false;

      list_push_back (&thread_current ()->children, &t_info->infoelem);
    }

  return tid;
}

/* TODO: move these somewhere more logical. */
static unsigned
mapid_hash (const struct hash_elem *m_, void *aux UNUSED)
{
  struct mapid_node *m = hash_entry (m_, struct mapid_node, elem);
  return hash_bytes (&m->addr, sizeof m->addr);
}

static bool
mapid_less (const struct hash_elem *a_ UNUSED,
            const struct hash_elem *b_ UNUSED,
            void *aux UNUSED)
{
  struct mapid_node *a = hash_entry (a_, struct mapid_node, elem);
  struct mapid_node *b = hash_entry (b_, struct mapid_node, elem);

  ASSERT (a != NULL);
  ASSERT (b != NULL);

  return a->addr < b->addr;
}

void mapid_destroy (struct hash_elem *m_, void *aux UNUSED)
{
  struct mapid_node *m = hash_entry (m_, struct mapid_node, elem);
  ASSERT (m != NULL);
  free (m);
}

/* A thread function that loads a user process and starts it
   running. */
static void
start_process (void *args_)
{
  char **args = (char **) args_;

  struct intr_frame if_;
  bool success;

  hash_init (&thread_current ()->supp_pt, sup_pt_hash_func, sup_pt_less_func,
             NULL);

  hash_init (&thread_current ()->file_map, mapid_hash, mapid_less, NULL);
  thread_current()->next_mapid = 0;

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;
  success = load (args[0], &if_.eip, &if_.esp);

  /* Set the load_status of the threads parent */
  struct thread *current = thread_current ();
  if (current->parent != NULL)
    {
      current->parent->child_status = (success) ? LOADED : FAILED;

      lock_acquire (&current->parent->cond_lock);
      cond_signal (&current->parent->child_waiter, &current->parent->cond_lock);
      lock_release (&current->parent->cond_lock);
    }

  /* If it didn't work, we get out */
  if (!success)
    thread_exit ();

  struct file *f = filesys_open (args[0]);
  if (f != NULL)
    {
      current->executable = f;
      file_deny_write (f);
    }

  int i;

  /* Tokenise arguments */
  char *arg_address[MAXARGS];

  /* Copy the arguments onto the stack and saves their addresses */
  for (i = 0; i < MAXARGS && args[i]; ++i)
    {
      if_.esp -= sizeof (char) * (strlen (args[i]) + 1);

      arg_address[i] = (char *) if_.esp;

      strlcpy (arg_address[i], args[i], strlen (args[i]) + 1);
    }

  /* Align to the next word */
  while (! (((uint32_t) if_.esp % 4) == 0))
    {
      uint8_t *align = --if_.esp;
      *align = 0;
    }

  int argc = i;
  /* null for end of array. */
  if_.esp -= sizeof (char **);
  char **end = if_.esp;
  *end = NULL;

  /* We put in the extra null pointer, so now we need to decrement
     our counter */
  --i;

  /* push argv addresses in reverse. */
  for (; i >= 0; --i)
    {
      if_.esp -= sizeof (char *);
      char **pntr = if_.esp;
      *pntr = arg_address[i];
    }

  /* Pointer to the start of argv */
  if_.esp -= sizeof (char **);
  char ***argv = if_.esp;
  *argv = if_.esp + sizeof (char **);

  /* number or args. */
  if_.esp -= sizeof (int *);
  int *argc_ptr = if_.esp;
  *argc_ptr = argc;

  /* void return. */
  if_.esp -= sizeof (int *);
  int *void_pntr = if_.esp;
  *void_pntr = 0;

  /* Freedom!! */
  for (i = 0; i < MAXARGS && args[i]; ++i)
    free(args[i]);
  free (args);

  /* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer (%esp) to our stack frame
     and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

#ifdef VM

/* Hash function for supplemental page table */
static unsigned
sup_pt_hash_func (const struct hash_elem *elem, void *aux UNUSED)
{
  struct sup_page *p = hash_entry (elem, struct sup_page, pt_elem);
  return hash_bytes (&p->user_addr, sizeof (p->user_addr));
}

/* Comparsion function for supplemental page table */
static bool
sup_pt_less_func (const struct hash_elem *a, const struct hash_elem *b,
                  void *aux UNUSED)
{
  struct sup_page *page_a = hash_entry (a, struct sup_page, pt_elem);
  struct sup_page *page_b = hash_entry (b, struct sup_page, pt_elem);

  ASSERT (page_a != NULL);
  ASSERT (page_b != NULL);

  return (page_a->user_addr - page_b->user_addr) < 0;
}

#endif


/* Waits for thread TID to die and returns its exit status.  If
   it was terminated by the kernel (i.e. killed due to an
   exception), returns -1.  If TID is invalid or if it was not a
   child of the calling process, or if process_wait() has already
   been successfully called for the given TID, returns -1
   immediately, without waiting.

   This function will be implemented in problem 2-2.  For now, it
   does nothing. */
int
process_wait (tid_t child_tid)
{
  /* Check if the thread was not created sucessfully */
  if (child_tid != TID_ERROR)
    {
      struct thread *current = thread_current ();
      struct child_info *child = NULL;

      struct list_elem *e = list_tail (&current->children);
       while ((e = list_prev (e)) != list_head (&current->children))
         {
           child = list_entry(e, struct child_info, infoelem);
           if (child->id == child_tid)
             break;
         }

      /* The thread with tid CHILD_TID is not a direct child
         of the current thread */
      if (child == NULL)
          return -1;
      else
        {
          /* current has already called wait on this child. */
          if (child->has_waited)
            return -1;

          /* If the thread is not dying, then wait until it has.
             The lock is aquired as that is a pre-condition of
             cond_wait */

          lock_acquire (&current->cond_lock);

          while (get_thread (child_tid) != NULL)
            cond_wait (&current->child_waiter, &current->cond_lock);

          lock_release (&current->cond_lock);

          /* Killed by the kernel, not by the conventional means */
          if (!child->has_exited)
            return -1;
          else
            {
              child->has_waited = true;
              return child->return_status;
            }
        }
    }
  else
    return TID_ERROR;
}

/* Free the current process's resources. */
void
process_exit (void)
{

  struct thread *cur = thread_current ();

  uint32_t *pd;

  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  pd = cur->pagedir;
  if (pd != NULL)
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }

  /* Destory the thread's suplementary page table */
  reclaim_pages (&cur->supp_pt);

  hash_destroy (&cur->file_map, NULL);

  /* Destory and free the list of child threads. Keep a temporaty pointer
     to the next item in the list, because we're killing list items as we
     loop */
  struct list_elem *e;
  struct list_elem *temp;
  for (e = list_begin (&cur->children);
       e != list_end(&cur->children); e = temp)
    {
      temp = list_next (e);
      struct child_info *info = list_entry (e, struct child_info, infoelem);
      list_remove (e);
      free (info);
    }

  if (cur->executable != NULL)
    {
      file_allow_write (cur->executable);
      file_close (cur->executable);
    }

  /* Signal the parent that the child is done. */
  struct thread *parent = cur->parent;
  if (parent != NULL)
    {
      lock_acquire (&parent->cond_lock);
      cond_signal (&parent->child_waiter, &parent->cond_lock);
      lock_release (&parent->cond_lock);
    }
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}

/* We load ELF binaries.  The following definitions are taken
   from the ELF specification, [ELF1], more-or-less verbatim.  */

/* ELF types.  See [ELF1] 1-2. */
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* For use with ELF types in printf(). */
#define PE32Wx PRIx32   /* Print Elf32_Word in hexadecimal. */
#define PE32Ax PRIx32   /* Print Elf32_Addr in hexadecimal. */
#define PE32Ox PRIx32   /* Print Elf32_Off in hexadecimal. */
#define PE32Hx PRIx16   /* Print Elf32_Half in hexadecimal. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
   This appears at the very beginning of an ELF binary. */
struct Elf32_Ehdr
  {
    unsigned char e_ident[16];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
  };

/* Program header.  See [ELF1] 2-2 to 2-4.
   There are e_phnum of these, starting at file offset e_phoff
   (see [ELF1] 1-6). */
struct Elf32_Phdr
  {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
  };

/* Values for p_type.  See [ELF1] 2-3. */
#define PT_NULL    0            /* Ignore. */
#define PT_LOAD    1            /* Loadable segment. */
#define PT_DYNAMIC 2            /* Dynamic linking info. */
#define PT_INTERP  3            /* Name of dynamic loader. */
#define PT_NOTE    4            /* Auxiliary info. */
#define PT_SHLIB   5            /* Reserved. */
#define PT_PHDR    6            /* Program header table. */
#define PT_STACK   0x6474e551   /* Stack segment. */

/* Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
#define PF_X 1          /* Executable. */
#define PF_W 2          /* Writable. */
#define PF_R 4          /* Readable. */

static bool setup_stack (void **esp);
static bool validate_segment (const struct Elf32_Phdr *, struct file *);
static bool lazy_load (struct file *file, off_t ofs, uint8_t *upage,
                          uint32_t read_bytes, uint32_t zero_bytes,
                          bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
   Stores the executable's entry point into *EIP
   and its initial stack pointer into *ESP.
   Returns true if successful, false otherwise. */
bool
load (const char *file_name, void (**eip) (void), void **esp)
{
  struct thread *t = thread_current ();
  struct Elf32_Ehdr ehdr;
  struct file *file = NULL;
  off_t file_ofs;
  bool success = false;
  int i;

  /* Allocate and activate page directory. */
  t->pagedir = pagedir_create ();
  if (t->pagedir == NULL)
    goto done;
  process_activate ();

  /* Open executable file. */
  file = filesys_open (file_name);
  if (file == NULL)
    {
      printf ("load: %s: open failed\n", file_name);
      goto done;
    }

  /* Read and verify executable header. */
  if (file_read (file, &ehdr, sizeof ehdr) != sizeof ehdr
      || memcmp (ehdr.e_ident, "\177ELF\1\1\1", 7)
      || ehdr.e_type != 2
      || ehdr.e_machine != 3
      || ehdr.e_version != 1
      || ehdr.e_phentsize != sizeof (struct Elf32_Phdr)
      || ehdr.e_phnum > 1024)
    {
      printf ("load: %s: error loading executable\n", file_name);
      goto done;
    }

  /* Read program headers. */
  file_ofs = ehdr.e_phoff;
  for (i = 0; i < ehdr.e_phnum; i++)
    {
      struct Elf32_Phdr phdr;

      if (file_ofs < 0 || file_ofs > file_length (file))
        goto done;
      file_seek (file, file_ofs);

      if (file_read (file, &phdr, sizeof phdr) != sizeof phdr)
        goto done;
      file_ofs += sizeof phdr;
      switch (phdr.p_type)
        {
        case PT_NULL:
        case PT_NOTE:
        case PT_PHDR:
        case PT_STACK:
        default:
          /* Ignore this segment. */
          break;
        case PT_DYNAMIC:
        case PT_INTERP:
        case PT_SHLIB:
          goto done;
        case PT_LOAD:
          if (validate_segment (&phdr, file))
            {
              bool writable = (phdr.p_flags & PF_W) != 0;
              uint32_t file_page = phdr.p_offset & ~PGMASK;
              uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
              uint32_t page_offset = phdr.p_vaddr & PGMASK;
              uint32_t read_bytes, zero_bytes;
              if (phdr.p_filesz > 0)
                {
                  /* Normal segment.
                     Read initial part from disk and zero the rest. */
                  read_bytes = page_offset + phdr.p_filesz;
                  zero_bytes = (ROUND_UP (page_offset + phdr.p_memsz, PGSIZE)
                                - read_bytes);
                }
              else
                {
                  /* Entirely zero.
                     Don't read anything from disk. */
                  read_bytes = 0;
                  zero_bytes = ROUND_UP (page_offset + phdr.p_memsz, PGSIZE);
                }
              if (!lazy_load (file, file_page, (void *) mem_page,
                                 read_bytes, zero_bytes, writable))
                goto done;
            }
          else
            goto done;
          break;
        }
    }

  /* Set up stack. */
  if (!setup_stack (esp))
    goto done;

  /* Start address. */
  *eip = (void (*) (void)) ehdr.e_entry;

  success = true;

 done:
  /* We arrive here whether the load is successful or not. */
  file_close (file);
  return success;
}

/* load() helpers. */

static bool install_page (void *upage, void *kpage, bool writable);

/* Checks whether PHDR describes a valid, loadable segment in
   FILE and returns true if so, false otherwise. */
static bool
validate_segment (const struct Elf32_Phdr *phdr, struct file *file)
{
  /* p_offset and p_vaddr must have the same page offset. */
  if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK))
    return false;

  /* p_offset must point within FILE. */
  if (phdr->p_offset > (Elf32_Off) file_length (file))
    return false;

  /* p_memsz must be at least as big as p_filesz. */
  if (phdr->p_memsz < phdr->p_filesz)
    return false;

  /* The segment must not be empty. */
  if (phdr->p_memsz == 0)
    return false;

  /* The virtual memory region must both start and end within the
     user address space range. */
  if (!is_user_vaddr ((void *) phdr->p_vaddr))
    return false;
  if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
    return false;

  /* The region cannot "wrap around" across the kernel virtual
     address space. */
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
    return false;

  /* Disallow mapping page 0.
     Not only is it a bad idea to map page 0, but if we allowed
     it then user code that passed a null pointer to system calls
     could quite likely panic the kernel by way of null pointer
     assertions in memcpy(), etc. */
  if (phdr->p_vaddr < PGSIZE)
    return false;

  /* It's okay. */
  return true;
}

/* Loads a segment starting at offset OFS in FILE at address
   UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
   memory are initialized, as follows:

        - READ_BYTES bytes at UPAGE must be read from FILE
          starting at offset OFS.

        - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.

   The pages initialized by this function must be writable by the
   user process if WRITABLE is true, read-only otherwise.

   Return true if successful, false if a memory allocation error
   or disk read error occurs. */
static bool
lazy_load (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

  file_seek (file, ofs);
  while (read_bytes > 0 || zero_bytes > 0)
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      struct sup_page *new;
      /* Either demand the full page from the file, create a zero page, or
         create a partially filled page from the file */
      if (page_read_bytes == PGSIZE)
          new = create_full_page (file, ofs, writable, upage);
      else if (page_zero_bytes == PGSIZE)
          new = create_zero_page (upage);
      else
          new = create_partial_page (file, ofs, zero_bytes, writable, upage, read_bytes);

      if (!add_sup_page (new))
        return false;

      /* Advance. */
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      upage += PGSIZE;

      ofs += page_read_bytes;
    }
  return true;
}

/* Create a minimal stack by mapping a zeroed page at the top of
   user virtual memory. */
static bool
setup_stack (void **esp)
{
  uint8_t *kpage;
  bool success = false;

#ifdef VM
  kpage = allocate_frame (PAL_USER | PAL_ZERO);
#else
  kpage = palloc_get_page (PAL_USER | PAL_ZERO);
#endif
  if (kpage != NULL)
    {
      success = install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage, true);
      if (success)
        *esp = PHYS_BASE;
      else
#ifdef VM
        free_frame (kpage);
#else
        palloc_free_page (kpage);
#endif
    }
  return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}
