partiallock
===========
A generic partial lock project.

**Partial lock** enables locking on a partial region of a shared data. One thread can grab a lock on exclusive regions while the other locks are still active. This locking method efficiently reduces the locking overhead from the serialization problem, when the shared data is large and concurrent threads modify only the small region of the data.


author
======
Jung-Sang Ahn <jungsang.ahn@gmail.com>


build
=====
make


how to use
==========
see tests/partiallock_test.cc or tests/partiallock_bench.cc
