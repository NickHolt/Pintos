#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

/* Program ID type. */
typedef int pid_t;

void syscall_init (void);

void exit (int status);

#endif /* userprog/syscall.h */
