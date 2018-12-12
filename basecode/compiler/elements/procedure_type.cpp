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

#include <common/bytes.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "block.h"
#include "field.h"
#include "element.h"
#include "program.h"
#include "identifier.h"
#include "declaration.h"
#include "initializer.h"
#include "procedure_type.h"
#include "symbol_element.h"
#include "type_reference.h"

//    int32_t offset = -8;
//    for (auto param : _parameters.as_list()) {
//    stack_frame->add(
//        vm::stack_frame_entry_type_t::parameter,
//        param->identifier()->symbol()->name(),
//        offset);
//    offset -= 8;
//}
//
//offset = 8;
//if (_return_type != nullptr) {
//stack_frame->add(
//    vm::stack_frame_entry_type_t::return_slot,
//"return_value",
//offset);
//offset += 8;
//}
//
//offset = 16;
//size_t local_count = 0;
//session.scope_manager().visit_blocks(
//    session.result(),
//[&](compiler::block* scope) {
//if (scope->is_parent_element(element_type_t::proc_type))
//return true;
//for (auto var : scope->identifiers().as_list()) {
//if (var->type_ref()->is_proc_type())
//continue;
//stack_frame->add(
//    vm::stack_frame_entry_type_t::local,
//    var->symbol()->name(),
//offset);
//var->usage(identifier_usage_t::stack);
//offset += 8;
//local_count++;
//}
//return true;
//},
//_scope);

namespace basecode::compiler {

    procedure_type::procedure_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::symbol_element* symbol) : compiler::type(
                                                      module,
                                                      parent_scope,
                                                      element_type_t::proc_type,
                                                      symbol),
                                                _scope(scope) {
    }

    bool procedure_type::emit_epilogue(
            session& session,
            emit_context_t& context,
            emit_result_t& result) {
        if (is_foreign())
            return true;

        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        block->move(
            vm::instruction_operand_t::sp(),
            vm::instruction_operand_t::fp(),
            vm::instruction_operand_t::offset(-16, vm::op_sizes::byte));
        block->pop(vm::instruction_operand_t::fp());
        block->rts();

        return false;
    }

    bool procedure_type::emit_prologue(
            session& session,
            emit_context_t& context,
            emit_result_t& result) {
        if (is_foreign())
            return true;

        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto procedure_label = symbol()->name();
        auto parent_init = parent_element_as<compiler::initializer>();
        if (parent_init != nullptr) {
            auto parent_var = parent_init->parent_element_as<compiler::identifier>();
            if (parent_var != nullptr) {
                procedure_label = parent_var->symbol()->name();
            }
        }

        block->blank_line();
        block->align(vm::instruction_t::alignment);
        block->label(assembler.make_label(procedure_label));

        block->push(vm::instruction_operand_t::fp());
        block->move(
            vm::instruction_operand_t::fp(),
            vm::instruction_operand_t::sp(),
            vm::instruction_operand_t::offset(16, vm::op_sizes::byte));

        uint64_t size = 0;
        for (auto var : _scope->identifiers().as_list()) {
            auto field = _parameters.find_by_name(var->symbol()->name());
            if (field != nullptr)
                continue;
            auto type = var->type_ref()->type();
            size += type->size_in_bytes();
        }

        size = common::align(size, 8);

        if (_return_type != nullptr)
            size += 8;

        if (size > 0) {
            block->sub(
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t(size, vm::op_sizes::dword));
        }

        return true;
    }

    bool procedure_type::is_foreign() const {
        return _is_foreign;
    }

    compiler::block* procedure_type::scope() {
        return _scope;
    }

    field_map_t& procedure_type::parameters() {
        return _parameters;
    }

    bool procedure_type::is_proc_type() const {
        return true;
    }

    void procedure_type::is_foreign(bool value) {
        _is_foreign = value;
    }

    bool procedure_type::on_is_constant() const {
        return true;
    }

    type_map_t& procedure_type::type_parameters() {
        return _type_parameters;
    }

    compiler::field* procedure_type::return_type() {
        return _return_type;
    }

    void procedure_type::return_type(field* value) {
        _return_type = value;
    }

    uint64_t procedure_type::foreign_address() const {
        return _foreign_address;
    }

    void procedure_type::foreign_address(uint64_t value) {
        _foreign_address = value;
    }

    procedure_instance_list_t& procedure_type::instances() {
        return _instances;
    }

    compiler::procedure_instance* procedure_type::instance_for(
            compiler::session& session,
            compiler::procedure_call* call) {
        // XXX: this is not complete.  for testing only
        if (_instances.empty())
            return nullptr;
        return _instances.back();
    }

    bool procedure_type::on_type_check(compiler::type* other) {
        if (other == nullptr)
            return false;

        return symbol()->name() == other->symbol()->name();
    }

    type_access_model_t procedure_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

    void procedure_type::on_owned_elements(element_list_t& list) {
        if (_scope != nullptr)
            list.emplace_back(_scope);

        if (_return_type != nullptr)
            list.emplace_back(_return_type);

        for (auto element : _parameters.as_list())
            list.emplace_back(element);
    }

    bool procedure_type::on_initialize(compiler::session& session) {
        return true;
    }

};