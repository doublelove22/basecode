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
#include "terp.h"
#include "assembler.h"
#include "instruction_block.h"

namespace basecode::vm {

    assembler::assembler(vm::terp* terp) : _terp(terp) {
    }

    assembler::~assembler() {
        for (auto block : _blocks)
            delete block;
        _blocks.clear();
    }

    bool assembler::assemble(
            common::result& r,
            vm::instruction_block* block) {
        // write out bootstrap jmp

        // write out data segments

        // write out instruction blocks

        return false;
    }

    vm::segment* assembler::segment(
            const std::string& name,
            segment_type_t type) {
        _segments.insert(std::make_pair(
            name,
            vm::segment(name, type)));
        return segment(name);
    }

    bool assembler::assemble_from_source(
            common::result& r,
            std::istream& source) {
        return false;
    }

    instruction_block* assembler::pop_block() {
        if (_block_stack.empty())
            return nullptr;
        auto top = _block_stack.top();
        if (top->type() == instruction_block_type_t::procedure && _procedure_block_count > 0)
            _procedure_block_count--;
        _block_stack.pop();
        return top;
    }

    vm::assembly_listing& assembler::listing() {
        return _listing;
    }

    bool assembler::in_procedure_scope() const {
        return _procedure_block_count > 0;
    }

    segment_list_t assembler::segments() const {
        segment_list_t list {};
        for (const auto& it : _segments) {
            list.push_back(const_cast<vm::segment*>(&it.second));
        }
        return list;
    }

    instruction_block* assembler::root_block() {
        return _blocks.front();
    }

    bool assembler::initialize(common::result& r) {
        _location_counter = _terp->heap_vector(heap_vectors_t::program_start);
        return true;
    }

    instruction_block* assembler::current_block() {
        if (_block_stack.empty())
            return nullptr;
        return _block_stack.top();
    }

    bool assembler::resolve_labels(common::result& r) {
        auto root_block = current_block();
        root_block->walk_blocks([&](instruction_block* block) -> bool {
            auto label_refs = block->label_references();
            for (auto label_ref : label_refs) {
                label_ref->resolved = root_block->find_label(label_ref->name);
                if (label_ref->resolved == nullptr) {
                    r.add_message(
                        "A001",
                        fmt::format("unable to resolve label: {}", label_ref->name),
                        true);
                    return false;
                }
            }
            return true;
        });
        return !r.is_failed();
    }

    bool assembler::apply_addresses(common::result& r) {
        size_t offset = 0;
        current_block()->walk_blocks([&](instruction_block* block) -> bool {
            for (auto& entry : block->entries()) {
                entry.address(_location_counter + offset);
                switch (entry.type()) {
                    case block_entry_type_t::memo: {
                        break;
                    }
                    case block_entry_type_t::align: {
                        auto alignment = entry.data<align_t>();
                        offset = common::align(offset, alignment->size);
                        entry.address(_location_counter + offset);
                        break;
                    }
                    case block_entry_type_t::section: {
                        auto section = entry.data<section_t>();
                        switch (*section) {
                            case section_t::bss:
                                break;
                            case section_t::text:
                                break;
                            case section_t::data:
                                break;
                            case section_t::ro_data:
                                break;
                        }
                        break;
                    }
                    case block_entry_type_t::instruction: {
                        auto inst = entry.data<instruction_t>();
                        offset += inst->encoding_size();
                        break;
                    }
                    case block_entry_type_t::data_definition:
                        auto data_def = entry.data<data_definition_t>();
                        offset += op_size_in_bytes(data_def->size);
                        break;
                }
            }
            return true;
        });
        return !r.is_failed();
    }

    void assembler::push_block(instruction_block* block) {
        _block_stack.push(block);
        if (block->type() == instruction_block_type_t::procedure)
            _procedure_block_count++;
    }

    void assembler::add_new_block(instruction_block* block) {
        auto source_file = _listing.current_source_file();
        if (source_file != nullptr)
            block->source_file(source_file);
        _blocks.push_back(block);
        auto top_block = current_block();
        if (top_block != nullptr)
            top_block->add_block(block);
    }

    vm::segment* assembler::segment(const std::string& name) {
        auto it = _segments.find(name);
        if (it == _segments.end())
            return nullptr;
        return &it->second;
    }

    instruction_block* assembler::make_basic_block(instruction_block* parent_block) {
        auto block = new instruction_block(
            parent_block != nullptr ? parent_block : current_block(),
            instruction_block_type_t::basic);
        add_new_block(block);
        return block;
    }

    instruction_block* assembler::make_procedure_block(instruction_block* parent_block) {
        auto block = new instruction_block(
            parent_block != nullptr ? parent_block : current_block(),
            instruction_block_type_t::procedure);
        add_new_block(block);
        return block;
    }

};