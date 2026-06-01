# GccLibcPolyPatch


<div align="center">

![CodeQL](https://github.com/serge1/ELFIO/workflows/CodeQL/badge.svg)
![Viewer](https://komarev.com/ghpvc/?username=fgfxf-gcclibpolypatch)
![Static Badge](https://img.shields.io/badge/License-MPL2.0-orange)

</div>

[English version](README.md "readme  for english")

对 libc.so.6 进行补丁处理，使 gcc 默认链接到较低版本的符号。

本项目仅解决 libc.so.6 的符号缺失问题。对于 C++（libstdc++\.so）中的符号缺失问题，请参考我的其他项目。

**本项目高度实验性，在 Ubuntu 24.04 x86_64 上进行了实验性测试。**

本项目由"奇虎360"和"河南科技大学"赞助。

版权所有 (c) fgfxf, qihoo360。保留所有权利。

<div align="center">
    <img src="./sponsor/360.png" width="128" height="128" />
    <img src="./sponsor/haust.png" width="128" height="128" />
</div>

例如：
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

两个 @@ 表示 gcc/g++ 链接时的**默认**符号。
这意味着当 gcc 链接 elf 文件时，默认会链接到 __libc_start_main@@GLIBC_2.34 和 memcpy@@GLIBC_2.14。
```bash
/lib64/libc.so.6: version `GLIBC_2.14' not found
/lib64/libc.so.6: version `GLIBC_2.38' not found

objdump  -T ./tool  | grep GLIBC_2.34
0000000000000000      DF *UND*	0000000000000000  GLIBC_2.34  __libc_start_main

objdump  -T ./tool  | grep GLIBC_2.14
0000000000000000      DF *UND*	0000000000000000  GLIBC_2.14  memcpy
```

在没有这些符号的旧系统上，程序无法运行。

复制系统的libc.so.6然后打补丁。将 **/lib/x86_64-linux-gnu/libc.so** 中 **/lib/x86_64-linux-gnu/libc.so.6** 的路径字符串修改为补丁后的路径，**不要**修改系统的 libc.so.6（/lib/x86_64-linux-gnu/libc.so）。否则会导致 CPU 软故障。

example文件夹里给出了一个案例，在ubuntu24.04上编译一个c代码，并复制到了centos5.11中运行。

如果 libc.so.6 中符号的最低版本高于目标系统，则此方法无法解决。例如：
```bash
nm -D /lib/x86_64-linux-gnu/libc.so.6  | grep getentropy
0000000000048710 T getentropy@@GLIBC_2.25
（getentropy 在 GLIBC 2.2.5 中尚不可用）
```

但是只有代码中使用了这种高版本符号，才会引入符号。你可以参考我的另一个项目，该项目涉及 libc.a/libstdc++.a 的**部分静态链接**以及到 libc-2.5\.so / libstdc++.so.6.0.8 的部分动态链接。
