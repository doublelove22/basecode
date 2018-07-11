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

#include <regex>
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include "parser.h"
#include "ast_formatter.h"

namespace basecode::syntax {

    ///////////////////////////////////////////////////////////////////////////

    static void pairs_to_list(
            const ast_node_shared_ptr& target,
            const ast_node_shared_ptr& root) {
        if (root->type != ast_node_types_t::pair) {
            target->children.push_back(root);
            return;
        }

        auto current_pair = root;
        while (true) {
            if (current_pair->lhs->type != ast_node_types_t::pair) {
                target->children.push_back(current_pair->lhs);
                target->children.push_back(current_pair->rhs);
                break;
            }
            target->children.push_back(current_pair->rhs);
            current_pair = current_pair->lhs;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    static ast_node_shared_ptr create_symbol_node(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto symbol_node = parser
            ->ast_builder()
            ->symbol_node();

        while (true) {
            auto symbol_part_node = parser
                ->ast_builder()
                ->symbol_part_node(token);
            symbol_node->children.push_back(symbol_part_node);
            if (!parser->peek(token_types_t::scope_operator))
                break;
            parser->consume();
            if (!parser->expect(r, token))
                return nullptr;
        }

        if (lhs != nullptr
        &&  (lhs->token.is_block_comment() || lhs->token.is_line_comment())) {
            symbol_node->children.push_back(lhs);
        }

        return symbol_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    static ast_node_shared_ptr create_expression_node(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto expression_node = parser->ast_builder()->expression_node();
        expression_node->lhs = parser->parse_expression(r, 0);

        token_t right_paren_token;
        right_paren_token.type = token_types_t::right_paren;
        if (!parser->expect(r, right_paren_token))
            return nullptr;

        if (lhs != nullptr
        &&  lhs->type == ast_node_types_t::block_comment) {
            expression_node->children.push_back(lhs);
        }

        return expression_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    static ast_node_shared_ptr create_cast_node(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto cast_node = parser
            ->ast_builder()
            ->cast_node(token);

        token_t less_than;
        less_than.type = token_types_t::less_than;
        if (!parser->expect(r, less_than))
            return nullptr;

        token_t identifier;
        identifier.type = token_types_t::identifier;
        if (!parser->expect(r, identifier))
            return nullptr;

        cast_node->lhs = parser
            ->ast_builder()
            ->type_identifier_node(identifier);

        token_t greater_than;
        greater_than.type = token_types_t::greater_than;
        if (!parser->expect(r, greater_than))
            return nullptr;

        token_t left_paren;
        left_paren.type = token_types_t::left_paren;
        if (!parser->expect(r, left_paren))
            return nullptr;

        cast_node->rhs = parser->parse_expression(r, 0);

        token_t right_paren;
        right_paren.type = token_types_t::right_paren;
        if (!parser->expect(r, right_paren))
            return nullptr;

        return cast_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    static ast_node_shared_ptr create_type_identifier_node(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto is_spread = false;
        ast_node_shared_ptr array_node = nullptr;

        if (parser->peek(token_types_t::left_square_bracket)) {
            array_node = parser->parse_expression(
                r,
                static_cast<uint8_t>(precedence_t::variable));
        }

        if (parser->peek(token_types_t::spread_operator)) {
            parser->consume();
            is_spread = true;
        }

        token_t type_identifier;
        type_identifier.type = token_types_t::identifier;

        if (!parser->expect(r, type_identifier)) {
            parser->error(
                r,
                "B027",
                "type expected.",
                token.line,
                token.column);
            return nullptr;
        }

        auto type_node = parser
            ->ast_builder()
            ->type_identifier_node(type_identifier);

        if (array_node != nullptr) {
            type_node->rhs = array_node;
            type_node->flags |= ast_node_t::flags_t::array;
        }

        if (is_spread)
            type_node->flags |= ast_node_t::flags_t::spread;

        return type_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr cast_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return create_cast_node(r, parser, token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr label_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->label_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr constant_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto symbol_node = parser->parse_expression(
            r,
            static_cast<uint8_t>(precedence_t::assignment));
        symbol_node->lhs = parser->ast_builder()->constant_node(token);
        return symbol_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr with_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto with_node = parser->ast_builder()->with_node(token);
        with_node->rhs = parser->parse_expression(r, 0);
        return with_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr defer_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto defer_node = parser->ast_builder()->defer_node(token);
        defer_node->rhs = parser->parse_expression(r, 0);
        return defer_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr union_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto union_node = parser->ast_builder()->union_node(token);
        union_node->rhs = parser->parse_expression(r, 0);
        return union_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr namespace_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto namespace_node = parser->ast_builder()->namespace_node(token);
        namespace_node->rhs = parser->parse_expression(r, 0);
        return namespace_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr struct_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto struct_node = parser->ast_builder()->struct_node(token);
        struct_node->rhs = parser->parse_expression(r, 0);
        return struct_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr enum_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto enum_node = parser->ast_builder()->enum_node(token);
        enum_node->rhs = parser->parse_expression(r, 0);
        return enum_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr for_in_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto for_node = parser->ast_builder()->for_in_node();
        for_node->lhs = parser->parse_expression(r, 0);

        token_t in_token;
        in_token.type = token_types_t::in_literal;
        if (!parser->expect(r, in_token))
            return nullptr;

        for_node->rhs = parser->parse_expression(r, 0);
        for_node->children.push_back(parser->parse_expression(r, 0));

        return for_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr return_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto return_node = parser->ast_builder()->return_node();
        pairs_to_list(return_node->rhs, parser->parse_expression(r, 0));
        return return_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr if_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto if_node = parser->ast_builder()->if_node();
        if_node->lhs = parser->parse_expression(r, 0);
        if_node->children.push_back(parser->parse_expression(r, 0));

        auto current_branch = if_node;
        while (true) {
            if (!parser->peek(token_types_t::else_if_literal))
                break;
            parser->consume();
            current_branch->rhs = parser->ast_builder()->else_if_node();
            current_branch->rhs->lhs = parser->parse_expression(r, 0);
            current_branch->rhs->children.push_back(parser->parse_expression(r, 0));
            current_branch = current_branch->rhs;
        }

        if (parser->peek(token_types_t::else_literal)) {
            parser->consume();
            current_branch->rhs = parser->ast_builder()->else_node();
            current_branch->rhs->children.push_back(parser->parse_expression(r, 0));
        }

        return if_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr type_identifier_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return create_type_identifier_node(r, parser, token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr basic_block_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->parse_scope(r);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr proc_expression_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto proc_node = parser->ast_builder()->proc_expression_node();

        token_t left_paren_token;
        left_paren_token.type = token_types_t::left_paren;
        if (!parser->expect(r, left_paren_token))
            return nullptr;

        if (!parser->peek(token_types_t::right_paren)) {
            pairs_to_list(proc_node->rhs, parser->parse_expression(r, 0));
        }

        token_t right_paren_token;
        right_paren_token.type = token_types_t::right_paren;
        if (!parser->expect(r, right_paren_token))
            return nullptr;

        if (parser->peek(token_types_t::colon)) {
            parser->consume();
            pairs_to_list(proc_node->lhs, parser->parse_expression(r, 0));
        }

        if (!parser->peek(token_types_t::semi_colon))
            proc_node->children.push_back(parser->parse_expression(r, 0));

        return proc_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr group_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return create_expression_node(r, parser, nullptr, token);
    }

    ///////////////////////////////////////////////////////////////////////////

    unary_operator_prefix_parser::unary_operator_prefix_parser(
        precedence_t precedence) : _precedence(precedence) {

    }

    ast_node_shared_ptr unary_operator_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto unary_operator_node = parser->ast_builder()->unary_operator_node(token);
        unary_operator_node->rhs = parser->parse_expression(
            r,
            static_cast<uint8_t>(_precedence));
        return unary_operator_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr keyword_literal_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        switch (token.type) {
            case token_types_t::alias_literal: {
                auto alias_node = parser->ast_builder()->alias_node(token);
                alias_node->lhs = parser->expect_expression(
                    r,
                    ast_node_types_t::assignment,
                    0);
                return alias_node;
            }
            case token_types_t::import_literal: {
                auto import_node = parser->ast_builder()->import_node(token);
                import_node->lhs = parser->parse_expression(r, 0);
                return import_node;
            }
            case token_types_t::break_literal: {
                return parser->ast_builder()->break_node(token);
            }
            case token_types_t::continue_literal: {
                return parser->ast_builder()->continue_node(token);
            }
            case token_types_t::null_literal: {
                return parser->ast_builder()->null_literal_node(token);
            }
            case token_types_t::true_literal:
            case token_types_t::false_literal: {
                return parser->ast_builder()->boolean_literal_node(token);
            }
            default:
                return nullptr;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr number_literal_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->number_literal_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr string_literal_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->string_literal_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr char_literal_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->character_literal_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr line_comment_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->line_comment_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr block_comment_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->block_comment_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr symbol_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return create_symbol_node(r, parser, nullptr, token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr comma_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto pair_node = parser->ast_builder()->pair_node();
        pair_node->lhs = lhs;
        pair_node->rhs = parser->parse_expression(
            r,
            static_cast<uint8_t>(precedence_t::comma));
        return pair_node;
    }

    precedence_t comma_infix_parser::precedence() const {
        return precedence_t::comma;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr block_comment_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto block_comment_node = parser
            ->ast_builder()
            ->block_comment_node(token);
        lhs->children.push_back(block_comment_node);
        return lhs;
    }

    precedence_t block_comment_infix_parser::precedence() const {
        return precedence_t::block_comment;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr cast_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        lhs->rhs = create_cast_node(r, parser, token);
        return lhs;
    }

    precedence_t cast_infix_parser::precedence() const {
        return precedence_t::cast;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr proc_call_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        if (lhs->type == ast_node_types_t::symbol) {
            auto proc_call_node = parser->ast_builder()->proc_call_node();
            proc_call_node->lhs = lhs;

            if (!parser->peek(token_types_t::right_paren)) {
                pairs_to_list(
                    proc_call_node->rhs,
                    parser->parse_expression(r, 0));
            }

            token_t right_paren_token;
            right_paren_token.type = token_types_t::right_paren;
            if (!parser->expect(r, right_paren_token))
                return nullptr;

            return proc_call_node;
        } else {
            return create_expression_node(r, parser, lhs, token);
        }
    }

    precedence_t proc_call_infix_parser::precedence() const {
        return precedence_t::call;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr symbol_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        return create_symbol_node(r, parser, lhs, token);
    }

    precedence_t symbol_infix_parser::precedence() const {
        return precedence_t::variable;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr type_identifier_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        lhs->rhs = create_type_identifier_node(r, parser, token);
        return lhs;
    }

    precedence_t type_identifier_infix_parser::precedence() const {
        return precedence_t::type;
    }

    ///////////////////////////////////////////////////////////////////////////

    binary_operator_infix_parser::binary_operator_infix_parser(
            precedence_t precedence,
            bool is_right_associative) : _precedence(precedence),
                                         _is_right_associative(is_right_associative) {
    }

    ast_node_shared_ptr binary_operator_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto associative_precedence = static_cast<uint8_t>(
            static_cast<uint8_t>(_precedence) - (_is_right_associative ? 1 : 0));
        return parser->ast_builder()->binary_operator_node(
            lhs,
            token,
            parser->parse_expression(r, associative_precedence));
    }

    precedence_t binary_operator_infix_parser::precedence() const {
        return _precedence;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr assignment_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto assignment_node = parser->ast_builder()->assignment_node();
        pairs_to_list(assignment_node->lhs, lhs);
        assignment_node->rhs = parser->parse_expression(
            r,
            static_cast<uint8_t>(precedence_t::assignment) - 1);
        return assignment_node;
    }

    precedence_t assignment_infix_parser::precedence() const {
        return precedence_t::assignment;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr directive_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto directive_node = parser->ast_builder()->directive_node(token);
        if (parser->peek(token_types_t::semi_colon)) {
            return directive_node;
        }
        directive_node->lhs = parser->parse_expression(r, 0);
        return directive_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr attribute_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto attribute_node = parser->ast_builder()->attribute_node(token);
        if (parser->peek(token_types_t::semi_colon)) {
            return attribute_node;
        }
        attribute_node->lhs = parser->parse_expression(r, 0);
        return attribute_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr array_subscript_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        ast_node_shared_ptr subscript_node = parser->ast_builder()->subscript_node();
        if (!parser->peek(token_types_t::right_square_bracket)) {
            subscript_node->lhs = parser->parse_expression(r, 0);
        }

        token_t right_bracket_token;
        right_bracket_token.type = token_types_t::right_square_bracket;
        if (!parser->expect(r, right_bracket_token))
            return nullptr;

        return subscript_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    parser::parser(std::istream& source) : _source(source),
                                           _lexer(source) {
    }

    parser::~parser() {
    }

    void parser::error(
            common::result& r,
            const std::string& code,
            const std::string& message,
            uint32_t line,
            uint32_t column) {
        _source.seekg(0, std::ios_base::beg);

        std::vector<std::string> source_lines {};
        std::string source_line;
        while (std::getline(_source, source_line)) {
            source_lines.push_back(source_line);
        }

        std::stringstream stream;
        stream << "\n";
        auto start_line = std::max<int32_t>(0, static_cast<int32_t>(line) - 4);
        auto stop_line = std::min<int32_t>(
            static_cast<int32_t>(source_lines.size()),
            line + 4);
        auto message_indicator = "^ " + message;
        for (int32_t i = start_line; i < stop_line; i++) {
            if (i == static_cast<int32_t>(line + 1)) {
                stream << fmt::format("{:04d}: ", i + 1)
                       << source_lines[i] << "\n"
                       << fmt::format("{}{}",
                                      std::string(column + 6, ' '),
                                      message_indicator);
            } else {
                stream << fmt::format("{:04d}: ", i + 1)
                       << source_lines[i];
            }

            if (i < static_cast<int32_t>(stop_line - 1))
                stream << "\n";
        }

        r.add_message(
            code,
            fmt::format("{} @ {}:{}", message, line, column),
            stream.str(),
            true);
    }

    bool parser::consume() {
        token_t token;
        return consume(token);
    }

    void parser::write_ast_graph(
            const std::filesystem::path& path,
            const ast_node_shared_ptr& program_node) {
        auto close_required = false;
        FILE* ast_output_file = stdout;
        if (!path.empty()) {
            ast_output_file = fopen(
                path.string().c_str(),
                "wt");
            close_required = true;
        }

        ast_formatter formatter(
            program_node,
            ast_output_file);
        formatter.format(fmt::format("AST Graph: {}", path.string()));

        if (close_required)
            fclose(ast_output_file);
    }

    bool parser::consume(token_t& token) {
        if (!look_ahead(0))
            return false;

        token = _tokens.front();
        _tokens.erase(_tokens.begin());

        return token.type != token_types_t::end_of_file;
    }

    bool parser::peek(token_types_t type) {
        if (!look_ahead(0))
            return false;
        auto& token = _tokens.front();
        return token.type == type;
    }

    bool parser::look_ahead(size_t count) {
        while (count >= _tokens.size() && _lexer.has_next()) {
            token_t token;
            if (_lexer.next(token))
                _tokens.push_back(token);
        }
        return !_tokens.empty();
    }

    uint8_t parser::current_infix_precedence() {
        if (!look_ahead(0))
            return 0;

        auto& token = _tokens.front();
        auto infix_parser = infix_parser_for(token.type);
        if (infix_parser != nullptr)
            return static_cast<uint8_t>(infix_parser->precedence());

        return 0;
    }

    syntax::ast_builder* parser::ast_builder() {
        return &_ast_builder;
    }

    ast_node_shared_ptr parser::parse_expression(
            common::result& r,
            uint8_t precedence) {
        token_t token;
        if (!consume(token))
            return nullptr;

        auto prefix_parser = prefix_parser_for(token.type);
        if (prefix_parser == nullptr) {
            error(
                r,
                "B021",
                fmt::format("prefix parser for token '{}' not found.", token.name()),
                token.line,
                token.column);
            return nullptr;
        }

        auto lhs = prefix_parser->parse(r, this, token);
        if (lhs == nullptr) {
            error(
                r,
                "B021",
                "unexpected empty ast node.",
                token.line,
                token.column);
            return nullptr;
        }

        if (token.is_line_comment()
        ||  token.is_label())
            return lhs;

        while (precedence < current_infix_precedence()) {
            if (!consume(token)) {
                break;
            }

            auto infix_parser = infix_parser_for(token.type);
            if (infix_parser == nullptr) {
                error(
                    r,
                    "B021",
                    fmt::format("infix parser for token '{}' not found.", token.name()),
                    token.line,
                    token.column);
                break;
            }
            lhs = infix_parser->parse(r, this, lhs, token);
            if (lhs == nullptr || r.is_failed())
                break;
        }

        return lhs;
    }

    ast_node_shared_ptr parser::expect_expression(
            common::result& r,
            ast_node_types_t expected_type,
            uint8_t precedence) {
        auto node = parse_expression(r, precedence);
        if (node == nullptr)
            return nullptr;

        if (node->type != expected_type) {
            error(
                r,
                "B031",
                fmt::format(
                    "unexpected '{}', wanted '{}'.",
                    node->name(),
                    ast_node_type_name(expected_type)),
                node->token.line,
                node->token.column);
            return nullptr;
        }

        return node;
    }

    ast_node_shared_ptr parser::parse(common::result& r) {
        return parse_scope(r);
    }

    bool parser::expect(common::result& r, token_t& token) {
        if (!look_ahead(0))
            return false;

        std::string expected_name = token.name();
        auto expected_type = token.type;
        token = _tokens.front();
        if (token.type != expected_type) {
            error(
                r,
                "B016",
                fmt::format(
                    "expected token '{}' but found '{}'.",
                    expected_name,
                    token.name()),
                token.line,
                token.column);
            return false;
        }

        _tokens.erase(_tokens.begin());

        return true;
    }

    ast_node_shared_ptr parser::parse_scope(common::result& r) {
        auto scope = _ast_builder.begin_scope();

        while (true) {
            if (peek(token_types_t::right_curly_brace)) {
                consume();
                break;
            }

            auto node = parse_statement(r);
            if (node == nullptr)
                break;
            scope->children.push_back(node);

            if (node->type == ast_node_types_t::statement) {
                token_t line_terminator_token;
                line_terminator_token.type = token_types_t::semi_colon;
                if (!expect(r, line_terminator_token))
                    break;
            }

            if (!scope->pending_attributes.empty()) {
                for (const auto& attr_node : scope->pending_attributes)
                    node->rhs->children.push_back(attr_node);
                scope->pending_attributes.clear();
            }
        }

        if (peek(token_types_t::attribute)) {
            auto attribute_node = parse_expression(r, 0);
            scope->children.push_back(attribute_node);
        }

        return _ast_builder.end_scope();
    }

    ast_node_shared_ptr parser::parse_statement(common::result& r) {
        ast_node_list pending_labels {};

        ast_node_shared_ptr expression;
        while (true) {
            expression = parse_expression(r, 0);
            if (expression == nullptr)
                return nullptr;

            if (expression->type == ast_node_types_t::attribute) {
                _ast_builder.current_scope()->pending_attributes.push_back(expression);
                token_t line_terminator_token;
                line_terminator_token.type = token_types_t::semi_colon;
                if (!expect(r, line_terminator_token))
                    break;
                continue;
            }

            if (expression->type == ast_node_types_t::label) {
                pending_labels.push_back(expression);
                continue;
            }

            if (expression->type == ast_node_types_t::line_comment
            ||  expression->type == ast_node_types_t::block_comment
            ||  expression->type == ast_node_types_t::basic_block) {
                return expression;
            }

            break;
        }

        auto statement_node = _ast_builder.statement_node();

        if (!pending_labels.empty()) {
            statement_node->lhs = _ast_builder.label_list_node();
            for (const auto& label_node : pending_labels)
                statement_node->lhs->children.push_back(label_node);
        }

        statement_node->rhs = expression;

        return statement_node;
    }

    infix_parser* parser::infix_parser_for(token_types_t type) {
        auto it = s_infix_parsers.find(type);
        if (it == s_infix_parsers.end())
            return nullptr;
        return it->second;
    }

    prefix_parser* parser::prefix_parser_for(token_types_t type) {
        auto it = s_prefix_parsers.find(type);
        if (it == s_prefix_parsers.end())
            return nullptr;
        return it->second;
    }

}