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

#include "field.h"

namespace basecode::compiler {

    field::field(
        element* parent,
        compiler::identifier* identifier): element(parent, element_type_t::field),
                                           _identifier(identifier) {
    }

    compiler::identifier* field::identifier() {
        return _identifier;
    }

};