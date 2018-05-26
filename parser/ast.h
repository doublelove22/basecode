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

#pragma once

#include <map>
#include <stack>
#include <memory>
#include <vector>
#include <string>
#include <cerrno>
#include <variant>
#include <functional>
#include "token.h"

namespace basecode::syntax {

    struct ast_node_t;

    using ast_node_shared_ptr = std::shared_ptr<ast_node_t>;
    using ast_node_list = std::vector<ast_node_shared_ptr>;

    enum class ast_node_types_t {
        program,
        fn_call,
        statement,
        attribute,
        directive,
        assignment,
        expression,
        basic_block,
        line_comment,
        none_literal,
        null_literal,
        empty_literal,
        block_comment,
        argument_list,
        fn_expression,
        if_expression,
        number_literal,
        string_literal,
        unary_operator,
        binary_operator,
        boolean_literal,
        map_constructor,
        else_expression,
        while_statement,
        alias_statement,
        break_statement,
        with_expression,
        type_identifier,
        enum_expression,
        symbol_reference,
        return_statement,
        extend_statement,
        for_in_statement,
        union_expression,
        character_literal,
        array_constructor,
        elseif_expression,
        switch_expression,
        struct_expression,
        continue_statement,
        constant_statement,
        namespace_statement,
        subscript_expression,
        qualified_symbol_reference,
    };

    static inline std::unordered_map<ast_node_types_t, std::string> s_node_type_names = {
        {ast_node_types_t::program, "program"},
        {ast_node_types_t::fn_call, "fn_call"},
        {ast_node_types_t::statement, "statement"},
        {ast_node_types_t::attribute, "attribute"},
        {ast_node_types_t::directive, "directive"},
        {ast_node_types_t::assignment, "assignment"},
        {ast_node_types_t::expression, "expression"},
        {ast_node_types_t::basic_block, "basic_block"},
        {ast_node_types_t::line_comment, "line_comment"},
        {ast_node_types_t::none_literal, "none_literal"},
        {ast_node_types_t::null_literal, "null_literal"},
        {ast_node_types_t::empty_literal, "empty_literal"},
        {ast_node_types_t::block_comment, "block_comment"},
        {ast_node_types_t::argument_list, "argument_list"},
        {ast_node_types_t::fn_expression, "fn_expression"},
        {ast_node_types_t::if_expression, "if_expression"},
        {ast_node_types_t::number_literal, "number_literal"},
        {ast_node_types_t::string_literal, "string_literal"},
        {ast_node_types_t::unary_operator, "unary_operator"},
        {ast_node_types_t::enum_expression, "enum_expression"},
        {ast_node_types_t::binary_operator, "binary_operator"},
        {ast_node_types_t::boolean_literal, "boolean_literal"},
        {ast_node_types_t::map_constructor, "map_constructor"},
        {ast_node_types_t::else_expression, "else_expression"},
        {ast_node_types_t::while_statement, "while_statement"},
        {ast_node_types_t::alias_statement, "alias_statement"},
        {ast_node_types_t::break_statement, "break_statement"},
        {ast_node_types_t::with_expression, "with_expression"},
        {ast_node_types_t::type_identifier, "type_identifier"},
        {ast_node_types_t::union_expression, "union_expression"},
        {ast_node_types_t::return_statement, "return_statement"},
        {ast_node_types_t::symbol_reference, "symbol_reference"},
        {ast_node_types_t::extend_statement, "extend_statement"},
        {ast_node_types_t::for_in_statement, "for_in_statement"},
        {ast_node_types_t::switch_expression, "switch_statement"},
        {ast_node_types_t::struct_expression, "struct_expression"},
        {ast_node_types_t::character_literal, "character_literal"},
        {ast_node_types_t::array_constructor, "array_constructor"},
        {ast_node_types_t::elseif_expression, "elseif_expression"},
        {ast_node_types_t::continue_statement, "continue_statement"},
        {ast_node_types_t::constant_statement, "constant_statement"},
        {ast_node_types_t::namespace_statement, "namespace_statement"},
        {ast_node_types_t::subscript_expression, "subscript_expression"},
        {ast_node_types_t::qualified_symbol_reference, "qualified_symbol_reference"},
    };
    
    struct ast_node_t {
        using flags_value_t = uint8_t;
        enum flags_t : uint8_t {
            none    = 0b00000000,
            pointer = 0b00000001,
            array   = 0b00000010,
            spread  = 0b00000100,
        };

        inline bool is_array() const {
            return ((flags & flags_t::array) != 0);
        }

        inline bool is_pointer() const {
            return ((flags & flags_t::pointer) != 0);
        }

        inline bool is_spread() const {
            return ((flags & flags_t::spread) != 0);
        }

        inline std::string name() const {
            auto it = s_node_type_names.find(type);
            if (it == s_node_type_names.end())
                return "unknown";
            return it->second;
        }

        inline bool operator != (const ast_node_t& other) const {
            return this->token.value != other.token.value;
        }

        inline bool operator == (const ast_node_t& other) const {
            return this->token.value == other.token.value;
        }

        uint32_t id;
        token_t token;
        ast_node_types_t type;
        ast_node_list children;
        ast_node_shared_ptr lhs = nullptr;
        ast_node_shared_ptr rhs = nullptr;
        flags_value_t flags = flags_t::none;
        ast_node_shared_ptr parent = nullptr;
    };

    class ast_builder {
    public:
        ast_builder();

        virtual ~ast_builder();

        ast_node_shared_ptr if_node();

        ast_node_shared_ptr else_node();

        ast_node_shared_ptr end_scope();

        ast_node_shared_ptr pop_scope();

        ast_node_shared_ptr begin_scope();

        ast_node_t* current_scope() const;

        ast_node_shared_ptr return_node();

        ast_node_shared_ptr for_in_node();

        ast_node_shared_ptr program_node();

        ast_node_shared_ptr fn_call_node();

        ast_node_shared_ptr fn_decl_node();

        ast_node_shared_ptr else_if_node();

        ast_node_shared_ptr statement_node();

        ast_node_shared_ptr subscript_node();

        ast_node_shared_ptr assignment_node();

        ast_node_shared_ptr expression_node();

        ast_node_shared_ptr basic_block_node();

        ast_node_shared_ptr binary_operator_node(
            const ast_node_shared_ptr& lhs,
            const token_t& token,
            const ast_node_shared_ptr& rhs);

        ast_node_shared_ptr argument_list_node();

        void push_scope(const ast_node_shared_ptr& node);

        ast_node_shared_ptr enum_node(const token_t& token);

        ast_node_shared_ptr break_node(const token_t& token);

        ast_node_shared_ptr union_node(const token_t& token);

        ast_node_shared_ptr qualified_symbol_reference_node();

        ast_node_shared_ptr struct_node(const token_t& token);

        ast_node_shared_ptr constant_node(const token_t& token);

        ast_node_shared_ptr continue_node(const token_t& token);

        ast_node_shared_ptr directive_node(const token_t& token);

        ast_node_shared_ptr attribute_node(const token_t& token);

        ast_node_shared_ptr namespace_node(const token_t& token);

        ast_node_shared_ptr none_literal_node(const token_t& token);

        ast_node_shared_ptr null_literal_node(const token_t& token);

        ast_node_shared_ptr line_comment_node(const token_t& token);

        ast_node_shared_ptr block_comment_node(const token_t& token);

        ast_node_shared_ptr empty_literal_node(const token_t& token);

        ast_node_shared_ptr number_literal_node(const token_t& token);

        ast_node_shared_ptr string_literal_node(const token_t& token);

        ast_node_shared_ptr unary_operator_node(const token_t& token);

        ast_node_shared_ptr type_identifier_node(const token_t& token);

        ast_node_shared_ptr boolean_literal_node(const token_t& token);

        ast_node_shared_ptr symbol_reference_node(const token_t& token);

        ast_node_shared_ptr character_literal_node(const token_t& token);

    private:
        void configure_node(
            const ast_node_shared_ptr& node,
            const token_t& token,
            ast_node_types_t type);

    private:
        uint32_t _id = 0;
        std::stack<ast_node_shared_ptr> _scope_stack {};
    };

}