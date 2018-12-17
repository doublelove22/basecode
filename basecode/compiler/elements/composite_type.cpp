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
#include "field.h"
#include "block.h"
#include "identifier.h"
#include "symbol_element.h"
#include "composite_type.h"
#include "type_reference.h"

namespace basecode::compiler {

    composite_type::composite_type(
            compiler::module* module,
            block* parent_scope,
            composite_types_t type,
            compiler::block* scope,
            compiler::symbol_element* symbol,
            element_type_t element_type) : compiler::type(
                                                module,
                                                parent_scope,
                                                element_type,
                                                symbol),
                                           _type(type),
                                           _scope(scope) {
    }

    bool composite_type::is_enum() const {
        return _type == composite_types_t::enum_type;
    }

    field_map_t& composite_type::fields() {
        return _fields;
    }

    bool composite_type::on_emit_initializer(
            compiler::session& session,
            compiler::variable* var) {
        auto block = session.assembler().current_block();

        auto field_list = _fields.as_list();
        for (auto fld: field_list) {
            variable_handle_t field_var {};
            if (!var->field(fld->identifier()->symbol()->name(), field_var)) {
                // XXX: error
                return false;
            }
            block->comment("initializer", vm::comment_location_t::after_instruction);
            if (!field_var->initializer()) {
                // XXX: error
                return false;
            }
        }

        return true;
    }

    compiler::block* composite_type::scope() {
        return _scope;
    }

    bool composite_type::on_is_constant() const {
        return true;
    }

    type_map_t& composite_type::type_parameters() {
        return _type_parameters;
    }

    composite_types_t composite_type::type() const {
        return _type;
    }

    bool composite_type::is_composite_type() const {
        return true;
    }

    bool composite_type::on_type_check(compiler::type* other) {
        return other != nullptr
               && other->symbol()->name() == symbol()->name();
    }

    type_access_model_t composite_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

    void composite_type::on_owned_elements(element_list_t& list) {
        for (auto element : _fields.as_list())
            list.emplace_back(element);

        if (_scope != nullptr)
            list.emplace_back(_scope);
    }

    bool composite_type::on_initialize(compiler::session& session) {
        size_t size = 0;
        for (auto fld : _fields.as_list())
            size += fld->size_in_bytes();
        size_in_bytes(size);
        alignment(sizeof(uint64_t));
        return true;
    }

};