# GccLibcPolyPatch


<div align="center">

![CodeQL](https://github.com/serge1/ELFIO/workflows/CodeQL/badge.svg)
![Viewer](https://komarev.com/ghpvc/?username=fgfxf-gcclibpolypatch)
![Static Badge](https://img.shields.io/badge/License-MPL2.0-orange)

</div>

Patch libc.so.6 to make gcc link to the lower version symbols by default.

This project only resolves symbol missing issues for libc.so.6 . For symbol missing issues in C++ (libstdc++\.so), please refer to my other project.

**This project is highly experimental and is being tested on Ubuntu 24.04 x86_64.**

This project is sponsored by "Qihoo 360" and "Henan University of Science and Technology".
<div align="center">
    <img src="./sponsor/360.png" width="128" height="128" />
    <img src="./sponsor/haust.png" width="128" height="128" />
</div>

For example:
```bash
cat /lib/x86_64-linux-gnu/libc.so
/* GNU ld script
   Use the shared library, but some functions are only in
   the static library, so try that secondarily.  */
OUTPUT_FORMAT(elf64-x86-64)
GROUP ( /lib/x86_64-linux-gnu/libc.so.6 /usr/lib/x86_64-linux-gnu/libc_nonshared.a  AS_NEEDED ( /lib64/ld-linux-x86-64.so.2 ) )

m -D /lib/x86_64-linux-gnu/libc.so.6  | grep start_main
000000000002a200 T __libc_start_main@@GLIBC_2.34
000000000002a200 T __libc_start_main@GLIBC_2.2.5
nm -D /lib/x86_64-linux-gnu/libc.so.6  | grep memcpy

00000000000ba870 T memcpy@GLIBC_2.2.5
00000000000b1720 i memcpy@@GLIBC_2.14
```

Two @@ indicate the **default** when gcc/g++ linking. 
This means that when gcc links elf files, it defaults to linking to __libc_start_main@@GLIBC_2.34 and memcpy@@GLIBC_2.14.
```bash
/lib64/libc.so.6: version `GLIBC_2.14' not found
/lib64/libc.so.6: version `GLIBC_2.38' not found

objdump  -T ./tool  | grep GLIBC_2.34
0000000000000000      DF *UND*	0000000000000000  GLIBC_2.34  __libc_start_main

objdump  -T ./tool  | grep GLIBC_2.14
0000000000000000      DF *UND*	0000000000000000  GLIBC_2.14  memcpy
```

On older systems without this symbol, the program cannot run.

Modify the path string of **/lib/x86_64-linux-gnu/libc.so.6** in **/lib/x86_64-linux-gnu/libc.so** to the patched path, and **DO NOT** modify the system's libc.so.6 (/lib/x86_64-linux-gnu/libc.so). Otherwise, it will cause the CPU soft fault.

If the minimum version of the symbols in libc.so.6 is lower than the target system, this method cannot be resolved. For example:
```bash
nm -D /lib/x86_64-linux-gnu/libc.so.6  | grep __stack_chk_fail
0000000000137ec0 T __stack_chk_fail@@GLIBC_2.4
(__stack_chk_fail is not yet available in GLIBC 2.2.5)
```
You can refer to my other project, which involves **partial static linking** of libc.a/libstdc++.a and partial dynamic linking to libc-2\.5\.so / libstdc++.so.6.0.8 .

