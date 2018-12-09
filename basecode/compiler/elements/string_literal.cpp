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

#include <compiler/session.h>
#include <vm/instruction_block.h>
#include <common/string_support.h>
#include "program.h"
#include "string_literal.h"

namespace basecode::compiler {

    string_literal::string_literal(
            compiler::module* module,
            block* parent_scope,
            const std::string& value) : element(module, parent_scope, element_type_t::string_literal),
                                        _value(value) {
    }

    bool string_literal::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        result.operands.emplace_back(vm::instruction_operand_t(
            assembler.make_label_ref(session.intern_data_label(this))));
        return true;
    }

    bool string_literal::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = session
            .scope_manager()
            .find_type(qualified_symbol_t("string"));
        return true;
    }

    bool string_literal::on_is_constant() const {
        return true;
    }

    std::string string_literal::escaped_value() const {
        return common::escaped_string(_value);
    }

    bool string_literal::on_as_string(std::string& value) const {
        value = _value;
        return true;
    }

}
