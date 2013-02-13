#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "userprog/process.h"

// This doesn't appear to be typedefed anyway else, and we need it for wait()
typedef int pid_t;

static void syscall_handler (struct intr_frame *);
static void halt (void);
static void exit (int status);
static int  write (int fd, const void *buffer, unsigned size);
static int  wait (pid_t pid);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Switch on the system call numbers defined in lib/syscall-nr.h, and call the
   appropriate system call. If the system call returns something, then put that
   value in f->eax */
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
        halt ();
        break;

      case SYS_EXIT:
        exit (*(stack_pointer + 1));
        break;

      case SYS_WRITE:
        f->eax = write (*(stack_pointer + 1), (void *) *(stack_pointer + 2),
                        *(stack_pointer + 3));
        break;

      case SYS_WAIT:
        f->eax = wait (*(stack_pointer + 1));
        break;
    }
}

/* The halt system call. Terminates Pintos */
static void
halt (void)
{
  // I think this is all we need here?
  shutdown_power_off ();
}

/* The exit system call. Terminate the current user program, sending the exit
   status to the kernel */
static void
exit (int status)
{
  struct thread *exiting_thread = thread_current();
  printf("%s: exit(%d)\n", exiting_thread->name, status);

  // I think there might need to be stuff to do with parents here, which will
  // involve adding a list of children to the thread struct probably

  thread_exit();
}

/* The write system call. Writes SIZE bytes to the file FD from BUFFER, or to
   the console if FD == 1. */
static int
write (int fd, const void *buffer, unsigned size)
{

  /* TODO: check if pointer to buffer is valid */

  if (fd == 1)
    {
      putbuf (buffer, size);
      return size;
    }
  else
    {
      // Write to the file. Need to lock the file system, open the file, write
      // to the file, and release the lock. We can probably use file_open and
      // file_write from filesys/file.h for this
    }

  return 0; // Needs to return number of bytes written to file
}

/* The wait system call is just defined in terms of process_wait in process.c,
   as recommended in the spec */
static int
wait (pid_t pid)
{
  return process_wait (pid);
}
