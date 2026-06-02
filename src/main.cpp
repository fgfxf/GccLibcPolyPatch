#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "CElfImp.h"

/*
 * Copyright (c) fgfxf, qihoo360. All rights reserved.
 * 版权所有 (c) fgfxf, qihoo360。保留所有权利。
 *
 * File: main.cpp / 文件：main.cpp
 * Description: Main entry point of GccLibcPolyPatch.
 *              This tool patches libc.so.6 (or other ELF shared libraries)
 *              to change the default symbol version from the newest to the oldest
 *              for symbols that have multiple versions.
 *              GccLibcPolyPatch 的主入口。
 *              该工具用于将 libc.so.6（或其他 ELF 共享库）中具有多个版本的符号的
 *              默认链接版本从新版本降级到最旧的版本。
 */
std::string LOGO=R"(GccLibcPolyPatch 
https://github.com/fgfxf/GccLibcPolyPatch
This tool is used to downgrade the default link version of symbols with multiple versions in libc.so.6 from the new version to the oldest version, for compatibility with GCC compilation.
Please do not directly modify the system so.6 file, but instead modify the .so script file.
Copyright (c) fgfxf, qihoo360. All rights reserved.

该工具用于将 libc.so.6中具有多个版本的符号的默认链接版本从新版本降级到最旧的版本，用于GCC编译兼容。
请不要直接修改系统so.6文件，而是修改.so脚本文件。
版权所有 (c) fgfxf, qihoo360。保留所有权利。

This project uses ELFIO.
ELFIO:
  Project: https://github.com/serge1/ELFIO
  License: MIT License
  Copyright: Copyright (C) Serge Lamikhov-Center
)";
int main(int argc, char** argv) {
    std::cout<<LOGO<<std::endl;
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <libc.so.6> [output.so]" << std::endl;
        return -1;
    }

    std::string path_libc_so_6 = argv[1];
    std::string output_path = (argc >= 3) ? argv[2] : path_libc_so_6 + ".patched";

    std::cout << "Loading file: " << path_libc_so_6 << std::endl;
    ELFIO::elfio reader;
    if (!reader.load(path_libc_so_6)) {
        std::cerr << "load file error!" << std::endl;
        return -1;
    }

    auto ver_map = CElfImp::load_version_definitions(reader);
    ELFIO::section* dynsym = reader.sections[".dynsym"];
    ELFIO::section* gnuver = reader.sections[".gnu.version"];

    if (!dynsym || !gnuver) {
        std::cerr << "Missing .dynsym or .gnu.version section!" << std::endl;
        return -1;
    }

    ELFIO::symbol_section_accessor symbols(reader, dynsym);
    ELFIO::versym_section_accessor versym_acc(gnuver);

    // 第一步：按符号名分组，收集所有版本信息
    std::map<std::string, std::vector<SymbolVersionInfo>> symbol_groups;

    for (ELFIO::Elf_Xword i = 0; i < symbols.get_symbols_num(); ++i) {
        std::string name;
        ELFIO::Elf64_Addr value;
        ELFIO::Elf_Xword size;
        unsigned char bind, type, other;
        ELFIO::Elf_Half section_index;

        symbols.get_symbol(i, name, value, size, bind, type, section_index, other);
        if (name.empty()) continue;

        ELFIO::Elf_Half raw_ver = 0;
        versym_acc.get_entry(static_cast<ELFIO::Elf_Word>(i), raw_ver);
        ELFIO::Elf_Half ver_idx = raw_ver & 0x7fff;

        // ver_idx == 0 表示本地符号，不参与版本切换
        if (ver_idx == 0) continue;

        auto it = ver_map.find(ver_idx);
        if (it == ver_map.end()) continue;

        SymbolVersionInfo info;
        info.ver_idx = ver_idx;
        info.raw_ver = raw_ver;
        info.version_name = it->second;
        info.is_default = CElfImp::default_symbol_flag_check(raw_ver);

        symbol_groups[name].push_back(info);
    }

    // 第二步：找出需要修改的符号（有多版本且当前默认不是最旧版）
    struct PatchEntry {
        ELFIO::Elf_Xword sym_idx;
        ELFIO::Elf_Half new_raw_ver;
    };
    std::vector<PatchEntry> patches;

    for (ELFIO::Elf_Xword i = 0; i < symbols.get_symbols_num(); ++i) {
        std::string name;
        ELFIO::Elf64_Addr value;
        ELFIO::Elf_Xword size;
        unsigned char bind, type, other;
        ELFIO::Elf_Half section_index;

        symbols.get_symbol(i, name, value, size, bind, type, section_index, other);
        if (name.empty()) continue;

        auto it = symbol_groups.find(name);
        if (it == symbol_groups.end()) continue;
        const auto& versions = it->second;

        // 只有多个版本的符号才需要处理
        if (versions.size() < 2) continue;

        ELFIO::Elf_Half current_raw_ver = 0;
        versym_acc.get_entry(static_cast<ELFIO::Elf_Word>(i), current_raw_ver);
        ELFIO::Elf_Half current_ver_idx = current_raw_ver & 0x7fff;
        bool current_is_default = CElfImp::default_symbol_flag_check(current_raw_ver);

        // 找到最旧的版本
        size_t oldest_idx = CElfImp::find_oldest_version_index(versions);
        const SymbolVersionInfo& oldest = versions[oldest_idx];

        // 如果当前已经是默认的最旧版本，跳过
        if (current_is_default && current_ver_idx == oldest.ver_idx) {
            continue;
        }

        // 如果当前是默认的但不是最旧版，需要修改
        if (current_is_default) {
            // 把当前默认设为非默认
            ELFIO::Elf_Half new_raw = current_raw_ver | 0x8000;
            patches.push_back({i, new_raw});
        }

        // 找到最旧版本对应的符号索引，把它设为默认
        // 注意：versions 中可能不包含当前索引 i 本身（如果当前不是默认）
        // 我们需要遍历所有同名符号，找到最旧版本的那个索引
    }

    // 重新遍历，处理把最旧版本设为默认
    for (ELFIO::Elf_Xword i = 0; i < symbols.get_symbols_num(); ++i) {
        std::string name;
        ELFIO::Elf64_Addr value;
        ELFIO::Elf_Xword size;
        unsigned char bind, type, other;
        ELFIO::Elf_Half section_index;

        symbols.get_symbol(i, name, value, size, bind, type, section_index, other);
        if (name.empty()) continue;

        auto it = symbol_groups.find(name);
        if (it == symbol_groups.end()) continue;
        const auto& versions = it->second;
        if (versions.size() < 2) continue;

        ELFIO::Elf_Half current_raw_ver = 0;
        versym_acc.get_entry(static_cast<ELFIO::Elf_Word>(i), current_raw_ver);
        ELFIO::Elf_Half current_ver_idx = current_raw_ver & 0x7fff;

        size_t oldest_idx = CElfImp::find_oldest_version_index(versions);
        const SymbolVersionInfo& oldest = versions[oldest_idx];

        if (current_ver_idx == oldest.ver_idx) {
            // 这是最旧版本，设为默认
            ELFIO::Elf_Half new_raw = current_ver_idx; // 清除 0x8000 标志
            // 检查是否已经在 patches 中（避免重复）
            bool already_patched = false;
            for (const auto& p : patches) {
                if (p.sym_idx == i) {
                    already_patched = true;
                    break;
                }
            }
            if (!already_patched) {
                patches.push_back({i, new_raw});
            }
        }
    }

    // 应用修改
    for (const auto& p : patches) {
        versym_acc.modify_entry(static_cast<ELFIO::Elf_Word>(p.sym_idx), p.new_raw_ver);
    }

    std::cout << "Patched " << patches.size() << " symbol entries." << std::endl;

    // 保存文件
    if (!reader.save(output_path)) {
        std::cerr << "save file error!" << std::endl;
        return -1;
    }

    std::cout << "Saved to: " << output_path << std::endl;
    return 0;
}
