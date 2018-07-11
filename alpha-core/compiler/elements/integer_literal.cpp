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

#include <vm/instruction_block.h>
#include "program.h"
#include "integer_literal.h"

namespace basecode::compiler {

    integer_literal::integer_literal(
            element* parent,
            uint64_t value) : element(parent, element_type_t::integer_literal),
                              _value(value) {
    }

    bool integer_literal::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->current_block();
        auto target_reg = instruction_block->current_target_register();
        instruction_block->move_u32_to_ireg(
            target_reg->reg.i,
            static_cast<uint32_t>(_value));
        return true;
    }

    uint64_t integer_literal::value() const {
        return _value;
    }

    bool integer_literal::on_is_constant() const {
        return true;
    }

    bool integer_literal::on_as_integer(uint64_t& value) const {
        value = _value;
        return true;
    }

    compiler::type* integer_literal::on_infer_type(const compiler::program* program) {
        // XXX: i'm a bad person, i should do type narrowing here
        return program->find_type_up("u32");
    }

};