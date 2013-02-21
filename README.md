Task 2 Plan
===========

Who will do what?
-----------------
* Owen:
    * ~~system call infrastructure~~
    * ~~`exit` system call~~
    * `write` system call
    * ~~`process_wait()`~~
* Dan:
    * ~~argument passing~~
* Charlie:
    * user memory access

Everyone needs to do system calls basically

Current Test Status
-------------------
pass tests/userprog/args-none  
pass tests/userprog/args-single  
pass tests/userprog/args-multiple  
pass tests/userprog/args-many  
pass tests/userprog/args-dbl-space  
FAIL tests/userprog/sc-bad-sp  
pass tests/userprog/sc-bad-arg  
pass tests/userprog/sc-boundary  
pass tests/userprog/sc-boundary-2  
pass tests/userprog/halt  
pass tests/userprog/exit  
pass tests/userprog/create-normal  
FAIL tests/userprog/create-empty  
FAIL tests/userprog/create-null  
FAIL tests/userprog/create-bad-ptr  
FAIL tests/userprog/create-long  
FAIL tests/userprog/create-exists  
pass tests/userprog/create-bound  
pass tests/userprog/open-normal  
FAIL tests/userprog/open-missing  
pass tests/userprog/open-boundary  
FAIL tests/userprog/open-empty  
pass tests/userprog/open-null  
FAIL tests/userprog/open-bad-ptr  
pass tests/userprog/open-twice  
pass tests/userprog/close-normal  
pass tests/userprog/close-twice  
pass tests/userprog/close-stdin  
pass tests/userprog/close-stdout  
pass tests/userprog/close-bad-fd  
FAIL tests/userprog/read-normal  
FAIL tests/userprog/read-bad-ptr  
FAIL tests/userprog/read-boundary  
FAIL tests/userprog/read-zero  
pass tests/userprog/read-stdout  
pass tests/userprog/read-bad-fd  
FAIL tests/userprog/write-normal  
FAIL tests/userprog/write-bad-ptr  
FAIL tests/userprog/write-boundary  
pass tests/userprog/write-zero  
pass tests/userprog/write-stdin  
pass tests/userprog/write-bad-fd  
FAIL tests/userprog/exec-once  
FAIL tests/userprog/exec-arg  
FAIL tests/userprog/exec-multiple  
FAIL tests/userprog/exec-missing  
pass tests/userprog/exec-bad-ptr  
FAIL tests/userprog/wait-simple  
FAIL tests/userprog/wait-twice  
FAIL tests/userprog/wait-killed  
FAIL tests/userprog/wait-bad-pid  
FAIL tests/userprog/multi-recurse  
FAIL tests/userprog/multi-child-fd  
FAIL tests/userprog/rox-simple  
FAIL tests/userprog/rox-child  
FAIL tests/userprog/rox-multichild  
pass tests/userprog/bad-read  
pass tests/userprog/bad-write  
pass tests/userprog/bad-read2  
pass tests/userprog/bad-write2  
pass tests/userprog/bad-jump  
pass tests/userprog/bad-jump2  
FAIL tests/userprog/no-vm/multi-oom  
FAIL tests/filesys/base/lg-create  
FAIL tests/filesys/base/lg-full  
FAIL tests/filesys/base/lg-random  
FAIL tests/filesys/base/lg-seq-block  
FAIL tests/filesys/base/lg-seq-random  
FAIL tests/filesys/base/sm-create  
FAIL tests/filesys/base/sm-full  
FAIL tests/filesys/base/sm-random  
FAIL tests/filesys/base/sm-seq-block  
FAIL tests/filesys/base/sm-seq-random  
FAIL tests/filesys/base/syn-read  
FAIL tests/filesys/base/syn-remove  
FAIL tests/filesys/base/syn-write  
43 of 76 tests failed.