// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <set>
#include <map>
#include <compiler/elements/element.h>

namespace basecode::compiler {

    class code_dom_formatter {
    public:
        code_dom_formatter(
            compiler::program* program_element,
            FILE* file);

        void format(const std::string& title);

    private:
        void add_primary_edge(
            element* parent,
            element* child,
            const std::string& label = "");

        void add_secondary_edge(
            element* parent,
            element* child,
            const std::string& label = "");

        std::string format_node(element* node);

        std::string get_vertex_name(element* node) const;

    private:
        FILE* _file = nullptr;
        std::set<std::string> _edges {};
        std::set<std::string> _nodes {};
        compiler::program* _program = nullptr;
    };

};
