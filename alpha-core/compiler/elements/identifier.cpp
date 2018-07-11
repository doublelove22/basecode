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
#include "type.h"
#include "identifier.h"
#include "initializer.h"

namespace basecode::compiler {

    identifier::identifier(
            element* parent,
            const std::string& name,
            compiler::initializer* initializer) : element(parent, element_type_t::identifier),
                                                  _name(name),
                                                  _initializer(initializer) {
    }

    bool identifier::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->current_block();

        if (_type->element_type() == element_type_t::namespace_type) {
            instruction_block->comment(fmt::format("namespace: {}", _name));
            return true;
        }

        auto target_reg = instruction_block->current_target_register();
        if (target_reg == nullptr)
            return true;

        if (context.current_access() == emit_access_type_t::write) {
            if (context.assembler->in_procedure_scope()
            &&  _usage == identifier_usage_t::stack) {
                emit_stack_based_load(instruction_block);
            } else {
                instruction_block->move_label_to_ireg(
                    target_reg->reg.i,
                    _name);
            }
        } else {
            switch (_type->element_type()) {
                case element_type_t::bool_type:
                case element_type_t::numeric_type: {
                    if (context.assembler->in_procedure_scope()
                    &&  _usage == identifier_usage_t::stack) {
                        emit_stack_based_load(instruction_block);
                    } else {
                        vm::i_registers_t ptr_reg;
                        if (!instruction_block->allocate_reg(ptr_reg)) {
                            // XXX: error!
                        }

                        instruction_block->move_label_to_ireg(ptr_reg, _name);
                        instruction_block->load_to_ireg_u64(
                            target_reg->reg.i,
                            ptr_reg);
                        instruction_block->free_reg(ptr_reg);
                    }
                    break;
                }
                default: {
                    instruction_block->move_label_to_ireg(
                        target_reg->reg.i,
                        _name);
                    break;
                }
            }
        }

        return true;
    }

    bool identifier::constant() const {
        return _constant;
    }

    compiler::type* identifier::type() {
        return _type;
    }

    std::string identifier::name() const {
        return _name;
    }

    void identifier::constant(bool value) {
        _constant = value;
    }

    void identifier::emit_stack_based_load(
            vm::instruction_block* instruction_block) {
        auto target_reg = instruction_block->current_target_register();
        auto entry = instruction_block->stack_frame()->find_up(name());
        if (entry == nullptr) {
            // XXX: error
            return;
        }
        instruction_block->comment(fmt::format(
            "{} identifier: {}",
            stack_frame_entry_type_name(entry->type),
            name()));
        instruction_block->load_to_ireg_u64(
            target_reg->reg.i,
            vm::i_registers_t::fp,
            entry->offset);
    }

    bool identifier::inferred_type() const {
        return _inferred_type;
    }

    bool identifier::on_is_constant() const {
        return _constant;
    }

    void identifier::type(compiler::type* t) {
        _type = t;
    }

    void identifier::inferred_type(bool value) {
        _inferred_type = value;
    }

    identifier_usage_t identifier::usage() const {
        return _usage;
    }

    bool identifier::on_as_bool(bool& value) const {
        if (_initializer == nullptr)
            return false;
        return _initializer->as_bool(value);
    }

    void identifier::usage(identifier_usage_t value) {
        _usage = value;
    }

    compiler::initializer* identifier::initializer() {
        return _initializer;
    }

    bool identifier::on_as_float(double& value) const {
        if (_initializer == nullptr)
            return false;
        return _initializer->as_float(value);
    }

    bool identifier::on_as_integer(uint64_t& value) const {
        if (_initializer == nullptr)
            return false;
        return _initializer->as_integer(value);
    }

    bool identifier::on_as_string(std::string& value) const {
        if (_initializer == nullptr)
            return false;
        return _initializer->as_string(value);
    }

};