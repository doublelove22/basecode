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

#include "namespace_type.h"

namespace basecode::compiler {

    namespace_type::namespace_type(
        element* parent) : compiler::type(
                                parent,
                                element_type_t::namespace_type,
                                "namespace") {
    }

    bool namespace_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        return true;
    }

};