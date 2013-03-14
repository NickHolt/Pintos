#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

/* Program ID type. */
typedef int pid_t;

/* Map region identifier. */
typedef int mapid_t;

void syscall_init (void);
void syscall_done (void);
void exit (int status);
void lock_filesystem (void);
void release_filesystem (void);

#define MAX_PUTBUF 512

#endif /* userprog/syscall.h */
