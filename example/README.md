## GccLibcPolyPatch Example

**This project is highly experimental and is being tested on Ubuntu 24.04 x86_64.**

```bash
cp /lib/x86_64-linux-gnu/libc.so .
./GccLibcPolyPatch   'libc.so.6'
nm -D libc.so.6.patched
...
00000000000ba870 T memcpy@@GLIBC_2.2.5
00000000000b1720 i memcpy@GLIBC_2.14
000000000002a200 T __libc_start_main@GLIBC_2.34
000000000002a200 T __libc_start_main@@GLIBC_2.2.5
...

gcc --version
gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
Copyright (C) 2023 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

gcc test.cpp  libc.so.6.patched

nm a.out  | grep start_main
                 U __libc_start_main@GLIBC_2.2.5

```
Copy the compiled product to CentOS 5.11, which is a system with glibc from 2006 years.

```bash
ldd --version
ldd (GNU libc) 2.5
Copyright (C) 2006 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
Written by Roland McGrath and Ulrich Drepper.

./a.out 
Hello World from ubuntu 24.04's gcc 13.
```
If you don't use this method, it will be:
```bash
./a.out 
./a.out: /lib64/libc.so.6: version `GLIBC_2.34' not found (required by ./a.out)
```

