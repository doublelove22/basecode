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
#include <common/string_support.h>
#include "environment.h"

namespace basecode::debugger {

    environment::environment(compiler::session& session) :_session(session) {
    }

    environment::~environment() {
        delete _stack_window;
        delete _output_window;
        delete _header_window;
        delete _footer_window;
        delete _memory_window;
        delete _command_window;
        delete _assembly_window;
        delete _registers_window;
    }

    int environment::ch() const {
        return _ch;
    }

    void environment::draw_all() {
        _header_window->draw(*this);
        _footer_window->draw(*this);
        _assembly_window->draw(*this);
        _registers_window->draw(*this);
        _memory_window->draw(*this);
        _output_window->draw(*this);
        _stack_window->draw(*this);
        _command_window->draw(*this);
        refresh();
    }

    bool environment::run(common::result& r) {
        auto& terp = _session.terp();

        _output_window->start_redirect();

        while (true) {
            auto pc = terp.register_file().r[vm::register_pc].qw;
            auto bp = breakpoint(pc);
            if (bp != nullptr && bp->enabled) {
                _state = debugger_state_t::break_s;
                _header_window->mark_dirty();
                _assembly_window->mark_dirty();
            }

            auto user_step = false;
            _ch = getch();
            switch (_ch) {
                case KEY_F(2): {
                    terp.reset();
                    pc = terp.register_file().r[vm::register_pc].qw;

                    _state = debugger_state_t::stopped;

                    _output_window->clear();
                    _stack_window->mark_dirty();
                    _header_window->mark_dirty();
                    _memory_window->mark_dirty();
                    _registers_window->mark_dirty();
                    _assembly_window->move_to_address(pc);
                    break;
                }
                case KEY_F(3): {
                    goto _exit;
                }
                case KEY_F(8): {
                    if (_state == debugger_state_t::break_s)
                        _state = debugger_state_t::single_step;

                    if (_state == debugger_state_t::single_step) {
                        user_step = true;
                    } else {
                        // XXX: state error
                    }
                    break;
                }
                case KEY_F(9): {
                    switch (_state) {
                        case debugger_state_t::running: {
                            break;
                        }
                        case debugger_state_t::stopped: {
                            _state = debugger_state_t::single_step;
                            _assembly_window->move_to_address(pc);
                            break;
                        }
                        case debugger_state_t::break_s:
                        case debugger_state_t::single_step: {
                            _state = debugger_state_t::running;
                            break;
                        }
                    }
                    _header_window->mark_dirty();
                    break;
                }
                case CTRL('c'): {
                    if (_state == debugger_state_t::running)
                        _state = debugger_state_t::stopped;
                    _stack_window->mark_dirty();
                    _header_window->mark_dirty();
                    _output_window->mark_dirty();
                    _memory_window->mark_dirty();
                    _assembly_window->mark_dirty();
                    _registers_window->mark_dirty();
                    break;
                }
                case CTRL('r'): {
                    _registers_window->update(*this);
                    break;
                }
                case ERR: {
                    break;
                }
                default: {
                    _assembly_window->update(*this);
                    break;
                }
            }

            bool execute_next_step;
            if (user_step) {
                execute_next_step = _state == debugger_state_t::single_step;
            } else {
                execute_next_step = _state == debugger_state_t::running;
            };

            if (execute_next_step) {
                auto success = terp.step(_session.result());
                if (!success) {
                    // XXX:
                    continue;
                }

                pc = terp.register_file().r[vm::register_pc].qw;
                _output_window->process_buffers();

                if (terp.has_exited()) {
                    _state = debugger_state_t::stopped;
                } else {
                    _assembly_window->move_to_address(pc);
                }

                _stack_window->mark_dirty();
                _header_window->mark_dirty();
                _memory_window->mark_dirty();
                _registers_window->mark_dirty();
            }

            draw_all();
            refresh();
        }

    _exit:
        _output_window->stop_redirect();

        return true;
    }

    breakpoint_t* environment::add_breakpoint(
            uint64_t address,
            breakpoint_type_t type) {
        auto bp = breakpoint(address);
        if (bp != nullptr)
            return bp;
        auto it = _breakpoints.insert(std::make_pair(
            address,
            breakpoint_t{true, address, type}));
        return &it.first->second;
    }

    compiler::session& environment::session() {
        return _session;
    }

    debugger_state_t environment::state() const {
        return _state;
    }

    bool environment::shutdown(common::result& r) {
        endwin();
        return true;
    }

    bool environment::initialize(common::result& r) {
        initscr();
        start_color();
        raw();
        keypad(stdscr, true);
        noecho();
        nodelay(stdscr, true);

        init_pair(1, COLOR_BLUE, COLOR_WHITE);
        init_pair(2, COLOR_BLUE, COLOR_YELLOW);
        init_pair(3, COLOR_RED, COLOR_WHITE);

        _main_window = new window(nullptr, stdscr);
        _header_window = new header_window(
            _main_window,
            0,
            0,
            _main_window->max_width(),
            1);
        _footer_window = new footer_window(
            _main_window,
            0,
            _main_window->max_height() - 1,
            _main_window->max_width(),
            1);
        _command_window = new command_window(
            _main_window,
            0,
            _main_window->max_height() - 2,
            _main_window->max_width(),
            1);
        _assembly_window = new assembly_window(
            _main_window,
            0,
            1,
            _main_window->max_width() - 23,
            _main_window->max_height() - 15);
        _registers_window = new registers_window(
            _main_window,
            _main_window->max_width() - 23,
            1,
            23,
            _main_window->max_height() - 15);
        _stack_window = new stack_window(
            _main_window,
            _main_window->max_width() - 38,
            _main_window->max_height() - 14,
            38,
            12);

        auto left_section = _main_window->max_width() - 38;
        _memory_window = new memory_window(
            _main_window,
            0,
            _main_window->max_height() - 14,
            left_section / 2,
            12);
        _output_window = new output_window(
            _main_window,
            _memory_window->x() + _memory_window->width(),
            _memory_window->y(),
            _memory_window->width(),
            _memory_window->height());

        // XXX: since we only have the one listing source file for now
        //      automatically select it.
        auto& listing = _session.assembler().listing();
        auto file_names = listing.file_names();
        if (!file_names.empty())
            _assembly_window->source_file(listing.source_file(file_names.front()));

        refresh();
        draw_all();

        return true;
    }

    void environment::remove_breakpoint(uint64_t address) {
        _breakpoints.erase(address);
    }

    breakpoint_t* environment::breakpoint(uint64_t address) {
        auto it = _breakpoints.find(address);
        if (it == _breakpoints.end())
            return nullptr;
        return &it->second;
    }

};