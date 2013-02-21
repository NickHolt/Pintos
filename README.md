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
FAIL tests/userprog/open-normal  
pass tests/userprog/open-missing  
FAIL tests/userprog/open-boundary  
pass tests/userprog/open-empty  
pass tests/userprog/open-null  
FAIL tests/userprog/open-bad-ptr  
FAIL tests/userprog/open-twice  
FAIL tests/userprog/close-normal  
FAIL tests/userprog/close-twice  
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
FAIL tests/userprog/write-zero  
pass tests/userprog/write-stdin  
pass tests/userprog/write-bad-fd  
pass tests/userprog/exec-once  
pass tests/userprog/exec-arg  
pass tests/userprog/exec-multiple  
pass tests/userprog/exec-missing  
pass tests/userprog/exec-bad-ptr  
pass tests/userprog/wait-simple  
pass tests/userprog/wait-twice  
pass tests/userprog/wait-killed  
pass tests/userprog/wait-bad-pid  
pass tests/userprog/multi-recurse  
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
37 of 76 tests failed.
