#include "CElfImp.h"
#include <algorithm>
#include <cctype>
#include <sstream>

/*
 * Copyright (c) fgfxf, qihoo360. All rights reserved.
 * 版权所有 (c) fgfxf, qihoo360。保留所有权利。
 *
 * File: CElfImp.cpp / 文件：CElfImp.cpp
 * Description: Implementation of ELF symbol version manipulation helpers,
 *              including version string parsing and default flag operations.
 *              ELF 符号版本操作辅助函数的实现，包括版本字符串解析和默认标志位操作。
 */

void CElfImp::default_symbol_flag_set(ELFIO::Elf_Half *vers) {
    *vers &= ~0x8000;  // 设为默认 @@
}

void CElfImp::default_symbol_flag_clear(ELFIO::Elf_Half *vers) {
    *vers |= 0x8000;   // 设为非默认 @
}

bool CElfImp::default_symbol_flag_check(const ELFIO::Elf_Half vers) {
    return (vers & 0x8000) == 0;
}

std::vector<int> CElfImp::parse_version(const std::string& ver_str) {
    std::vector<int> result;
    // 跳过前缀（如 GLIBC_、GLIBCXX_、GCC_ 等）
    size_t pos = 0;
    while (pos < ver_str.size() && !std::isdigit(ver_str[pos])) {
        ++pos;
    }
    // 解析数字部分，如 "2.2.5"
    std::string num_part = ver_str.substr(pos);
    std::stringstream ss(num_part);
    std::string token;
    while (std::getline(ss, token, '.')) {
        if (!token.empty()) {
            result.push_back(std::stoi(token));
        }
    }
    return result;
}

bool CElfImp::is_older_version(const std::string& a, const std::string& b) {
    std::vector<int> va = parse_version(a);
    std::vector<int> vb = parse_version(b);
    size_t len = std::max(va.size(), vb.size());
    for (size_t i = 0; i < len; ++i) {
        int av = (i < va.size()) ? va[i] : 0;
        int bv = (i < vb.size()) ? vb[i] : 0;
        if (av < bv) return true;
        if (av > bv) return false;
    }
    return false; // 相等不算旧
}

size_t CElfImp::find_oldest_version_index(const std::vector<SymbolVersionInfo>& versions) {
    if (versions.empty()) return 0;
    size_t oldest_idx = 0;
    for (size_t i = 1; i < versions.size(); ++i) {
        if (is_older_version(versions[i].version_name, versions[oldest_idx].version_name)) {
            oldest_idx = i;
        }
    }
    return oldest_idx;
}

std::map<ELFIO::Elf_Half, std::string>
CElfImp::load_version_definitions(ELFIO::elfio& reader)
{
    std::map<ELFIO::Elf_Half, std::string> ver_map;

    const ELFIO::section* verdef_sec = reader.sections[".gnu.version_d"];
    const ELFIO::section* dynstr_sec = reader.sections[".dynstr"];

    if (!verdef_sec || !dynstr_sec) {
        return ver_map;
    }

    const char* verdef_data = verdef_sec->get_data();
    const char* dynstr_data = dynstr_sec->get_data();

    ELFIO::Elf_Xword offset = 0;

    while (offset < verdef_sec->get_size()) {
        auto* verdef = reinterpret_cast<const ELFIO::Elfxx_Verdef*>(
            verdef_data + offset
        );

        ELFIO::Elf_Half version_index = verdef->vd_ndx & 0x7fff;

        ELFIO::Elf_Word aux_offset = offset + verdef->vd_aux;

        auto* verdaux = reinterpret_cast<const ELFIO::Elfxx_Verdaux*>(
            verdef_data + aux_offset
        );

        std::string version_name = dynstr_data + verdaux->vda_name;

        ver_map[version_index] = version_name;

        if (verdef->vd_next == 0) {
            break;
        }

        offset += verdef->vd_next;
    }

    return ver_map;
}