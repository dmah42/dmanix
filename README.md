dmanix
=====
It's an i386 OS.

One time setup:
$ sudo mkdir /mnt/dmaos\_floppy

To build:
$ make [debug|release]
$ scripts/update.sh
$ scripts/run.sh

debug build defines DEBUG, release build defines NDEBUG

To run the tests:
$ test/test_all

Before submitting:
$ scripts/presubmit.sh

Allowed c++ headers:
 <ciso646>
 <cstddef>
 <cfloat>
 <limits>
 <climits>
 <cstdint>
 <cstdlib>
 <new>
 <typeinfo>
 <initializer_list>
 <cstdalign>
 <cstdarg>
 <cstdbool>
 <type_traits>
 <atomic>
