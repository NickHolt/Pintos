#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Switch on the system call numbers defined in lib/syscall-nr.h, and call
   the appropriate system call */
static void
syscall_handler (struct intr_frame *f)
{
  int *stack_pointer = f->esp;

  /* TODO: check if pointer is valid */

  int syscall_number = *stack_pointer;

  // Maybe function pointers would be neater here?
  switch (syscall_number)
    {
      case SYS_HALT:
        /* call halt() */
        break;

      case SYS_EXIT:
        /* call exit() */
        break;

      case SYS_WRITE:
        /* call write() */
        break;

      case SYS_WAIT:
        /* call wait() */
        break;
    }
}
