// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <map>
#include <cstdint>
#include <filesystem>
#include "result.h"

namespace basecode::common {

    using source_file_range_t = std::pair<size_t, size_t>;

    struct source_file_range_compare_t {
        bool operator()(
                const source_file_range_t& lhs,
                const source_file_range_t& rhs) const {
            return lhs.second < rhs.first;
        }
    };

    struct source_file_line_t {
        size_t end {};
        size_t begin {};
        uint32_t line {};
        uint32_t columns {};
    };

    class source_file {
    public:
        explicit source_file(const std::filesystem::path& path);

        ~source_file();

        rune_t next();

        bool eof() const;

        bool empty() const;

        size_t length() const;

        bool load(common::result& r);

        size_t number_of_lines() const;

        uint8_t operator[](size_t index);

        const std::filesystem::path& path() const;

        std::string substring(size_t start, size_t end);

        const source_file_line_t* line_by_number(size_t line) const;

        const source_file_line_t* line_by_index(size_t index) const;

    private:
        void build_lines();

    private:
        size_t _index = 0;
        std::filesystem::path _path;
        std::vector<uint8_t> _buffer;
        std::map<size_t, source_file_line_t*> _lines_by_number {};
        std::map<
            source_file_range_t,
            source_file_line_t,
            source_file_range_compare_t> _lines_by_index_range {};
    };

};

