#pragma once

#include "elfio/elfio.hpp"
#include <map>
#include <string>
#include <vector>

/*
 * Copyright (c) fgfxf, qihoo360. All rights reserved.
 * 版权所有 (c) fgfxf, qihoo360。保留所有权利。
 *
 * File: CElfImp.h / 文件：CElfImp.h
 * Description: ELF symbol version manipulation helper class and data structures.
 *              ELF 符号版本操作辅助类及数据结构定义。
 */

struct SymbolVersionInfo {
    ELFIO::Elf_Half ver_idx;
    ELFIO::Elf_Half raw_ver;
    std::string version_name;
    bool is_default;
};

class CElfImp {
public:
    /**
     * @brief 把符号版本数字转为字符串 / uint16_t -> string (GLIBC_2.2.5)
     * @author fgfxf / chatgpt
     * @date 2026-06-01
     */
    static std::map<ELFIO::Elf_Half, std::string> load_version_definitions(ELFIO::elfio& reader);

    /**
     * @brief default symbol flag operator
     * @author fgfxf / chatgpt
     * @date 2026-06-01
     */
    static void default_symbol_flag_set(ELFIO::Elf_Half *vers);
    static void default_symbol_flag_clear(ELFIO::Elf_Half *vers);
    static bool default_symbol_flag_check(const ELFIO::Elf_Half vers);

    /**
     * @brief 解析版本字符串，返回版本号数组用于比较
     * @param ver_str 版本字符串，如 "GLIBC_2.2.5"
     * @return 版本号数组，如 {2, 2, 5}
     */
    static std::vector<int> parse_version(const std::string& ver_str);

    /**
     * @brief 比较两个版本字符串，返回 true 如果 a 比 b 旧（版本号更小）
     */
    static bool is_older_version(const std::string& a, const std::string& b);

    /**
     * @brief 从版本定义映射中找出最旧的版本索引
     * @param versions 该符号的所有版本信息列表
     * @return 最旧版本的索引在 versions 中的位置
     */
    static size_t find_oldest_version_index(const std::vector<SymbolVersionInfo>& versions);
};


