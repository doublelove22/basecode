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

#include "string_literal.h"

namespace basecode::compiler {

    string_literal::string_literal(
            element* parent,
            const std::string& value) : element(parent, element_type_t::string_literal),
                                        _value(value) {
    }

}