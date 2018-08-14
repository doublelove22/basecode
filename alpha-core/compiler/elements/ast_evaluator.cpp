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
#include "type.h"
#include "cast.h"
#include "label.h"
#include "alias.h"
#include "import.h"
#include "module.h"
#include "comment.h"
#include "program.h"
#include "any_type.h"
#include "bool_type.h"
#include "attribute.h"
#include "directive.h"
#include "statement.h"
#include "type_info.h"
#include "transmute.h"
#include "expression.h"
#include "identifier.h"
#include "if_element.h"
#include "array_type.h"
#include "tuple_type.h"
#include "initializer.h"
#include "module_type.h"
#include "string_type.h"
#include "numeric_type.h"
#include "unknown_type.h"
#include "pointer_type.h"
#include "argument_list.h"
#include "float_literal.h"
#include "ast_evaluator.h"
#include "string_literal.h"
#include "unary_operator.h"
#include "composite_type.h"
#include "procedure_type.h"
#include "return_element.h"
#include "procedure_call.h"
#include "namespace_type.h"
#include "symbol_element.h"
#include "element_builder.h"
#include "boolean_literal.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "element_builder.h"
#include "module_reference.h"
#include "namespace_element.h"
#include "procedure_instance.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    std::unordered_map<syntax::ast_node_types_t, node_evaluator_callable> ast_evaluator::s_node_evaluators = {
        {syntax::ast_node_types_t::pair,                    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::label,                   std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::module,                  std::bind(&ast_evaluator::module, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::symbol,                  std::bind(&ast_evaluator::symbol, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::type_list,               std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::proc_call,               std::bind(&ast_evaluator::proc_call, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::statement,               std::bind(&ast_evaluator::statement, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::attribute,               std::bind(&ast_evaluator::attribute, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::directive,               std::bind(&ast_evaluator::directive, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::assignment,              std::bind(&ast_evaluator::assignment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::expression,              std::bind(&ast_evaluator::expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::label_list,              std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::basic_block,             std::bind(&ast_evaluator::basic_block, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::symbol_part,             std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::line_comment,            std::bind(&ast_evaluator::line_comment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::null_literal,            std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::block_comment,           std::bind(&ast_evaluator::block_comment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::argument_list,           std::bind(&ast_evaluator::argument_list, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::if_expression,           std::bind(&ast_evaluator::if_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::parameter_list,          std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::number_literal,          std::bind(&ast_evaluator::number_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::string_literal,          std::bind(&ast_evaluator::string_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::unary_operator,          std::bind(&ast_evaluator::unary_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::cast_expression,         std::bind(&ast_evaluator::cast_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::from_expression,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::proc_expression,         std::bind(&ast_evaluator::proc_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::enum_expression,         std::bind(&ast_evaluator::enum_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::binary_operator,         std::bind(&ast_evaluator::binary_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::boolean_literal,         std::bind(&ast_evaluator::boolean_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::map_constructor,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::else_expression,         std::bind(&ast_evaluator::else_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::while_statement,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::break_statement,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::with_expression,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::type_identifier,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::alias_expression,        std::bind(&ast_evaluator::alias_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::defer_expression,        std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::union_expression,        std::bind(&ast_evaluator::union_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::return_statement,        std::bind(&ast_evaluator::return_statement, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::symbol_reference,        std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::extend_statement,        std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::for_in_statement,        std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::switch_expression,       std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::import_expression,       std::bind(&ast_evaluator::import_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::struct_expression,       std::bind(&ast_evaluator::struct_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::character_literal,       std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::array_constructor,       std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::module_expression,       std::bind(&ast_evaluator::module_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::elseif_expression,       std::bind(&ast_evaluator::if_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::continue_statement,      std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::constant_expression,     std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::transmute_expression,    std::bind(&ast_evaluator::transmute_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::namespace_expression,    std::bind(&ast_evaluator::namespace_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::subscript_expression,    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::return_argument_list,    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::array_subscript_list,    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::assignment_source_list,  std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::assignment_target_list,  std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
    };

    ///////////////////////////////////////////////////////////////////////////

    ast_evaluator::ast_evaluator(
            element_builder* builder,
            compiler::program* program) : _builder(builder),
                                          _program(program) {
    }

    element* ast_evaluator::evaluate(
            compiler::session& session,
            const syntax::ast_node_t* node,
            element_type_t default_block_type) {
        if (node == nullptr)
            return nullptr;

        evaluator_context_t context(session);
        context.node = node;
        context.scope = _program->current_scope();
        context.default_block_type = default_block_type;

        auto it = s_node_evaluators.find(node->type);
        if (it != s_node_evaluators.end()) {
            evaluator_result_t result {};
            if (it->second(this, context, result)) {
                return result.element;
            } else {
                _program->error(
                    session,
                    "P071",
                    fmt::format(
                        "ast node evaluation failed: id = {}, type = {}",
                        node->id,
                        syntax::ast_node_type_name(node->type)),
                    node->location);
            }
        }

        return nullptr;
    }

    void ast_evaluator::apply_attributes(
            const evaluator_context_t& context,
            compiler::element* element,
            const syntax::ast_node_t* node) {
        if (node == nullptr)
            return;

        for (auto it = node->children.begin();
             it != node->children.end();
             ++it) {
            const auto& child_node = *it;
            if (child_node->type == syntax::ast_node_types_t::attribute) {
                auto attribute = dynamic_cast<compiler::attribute*>(evaluate(
                    context.session,
                    child_node.get()));
                attribute->parent_element(element);

                auto& attributes = element->attributes();
                attributes.add(attribute);
            }
        }
    }

    element* ast_evaluator::evaluate_in_scope(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node,
            compiler::block* scope,
            element_type_t default_block_type) {
        _program->push_scope(scope);
        auto result = evaluate(
            context.session,
            node,
            default_block_type);
        _program->pop_scope();
        return result;
    }

    void ast_evaluator::add_procedure_instance(
            const evaluator_context_t& context,
            compiler::procedure_type* proc_type,
            const syntax::ast_node_t* node) {
        if (node->children.empty())
            return;

        for (const auto& child_node : node->children) {
            switch (child_node->type) {
                case syntax::ast_node_types_t::attribute: {
                    auto attribute = _builder->make_attribute(
                        proc_type->scope(),
                        child_node->token.value,
                        evaluate(context.session, child_node->lhs.get()));
                    attribute->parent_element(proc_type);
                    proc_type->attributes().add(attribute);
                    break;
                }
                case syntax::ast_node_types_t::basic_block: {
                    auto basic_block = dynamic_cast<compiler::block*>(evaluate_in_scope(
                        context,
                        child_node.get(),
                        proc_type->scope(),
                        element_type_t::proc_instance_block));
                    auto instance = _builder->make_procedure_instance(
                        proc_type->scope(),
                        proc_type,
                        basic_block);
                    instance->parent_element(proc_type);
                    proc_type->instances().push_back(instance);
                }
                default:
                    break;
            }
        }
    }

    void ast_evaluator::add_expression_to_scope(
            compiler::block* scope,
            compiler::element* expr) {
        switch (expr->element_type()) {
            case element_type_t::comment: {
                auto comment = dynamic_cast<compiler::comment*>(expr);
                scope->comments().emplace_back(comment);
                break;
            }
            case element_type_t::import_e: {
                auto import = dynamic_cast<compiler::import*>(expr);
                scope->imports().emplace_back(import);
                break;
            }
            case element_type_t::attribute: {
                auto attribute = dynamic_cast<compiler::attribute*>(expr);
                scope->attributes().add(attribute);
                break;
            }
            case element_type_t::statement: {
                auto statement = dynamic_cast<compiler::statement*>(expr);
                scope->statements().emplace_back(statement);
                break;
            }
            default:
                break;
        }
    }

    compiler::element* ast_evaluator::resolve_symbol_or_evaluate(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node) {
        compiler::element* element = nullptr;
        if (node != nullptr
        &&  node->type == syntax::ast_node_types_t::symbol) {
            qualified_symbol_t qualified_symbol {};
            _builder->make_qualified_symbol(qualified_symbol, context.node);
            element = _builder->make_identifier_reference(
                _program->current_scope(),
                qualified_symbol,
                _program->find_identifier(qualified_symbol));
        } else {
            element = evaluate(context.session, node);
        }
        return element;
    }

    compiler::block* ast_evaluator::add_namespaces_to_scope(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node,
            compiler::symbol_element* symbol,
            compiler::block* parent_scope) {
        auto namespace_type = _program->find_type(qualified_symbol_t {
            .name = "namespace"
        });

        auto namespaces = symbol->namespaces();
        auto scope = parent_scope;
        string_list_t temp_list {};
        std::string namespace_name {};
        for (size_t i = 0; i < namespaces.size(); i++) {
            if (!namespace_name.empty())
                temp_list.push_back(namespace_name);
            namespace_name = namespaces[i];
            auto var = scope->identifiers().find(namespace_name);
            if (var == nullptr) {
                auto new_scope = _builder->make_block(scope, element_type_t::block);
                auto ns = _builder->make_namespace(scope, new_scope);
                auto ns_identifier = _builder->make_identifier(
                    scope,
                    _builder->make_symbol(scope, namespace_name, temp_list),
                    _builder->make_initializer(scope, ns));
                ns_identifier->type(namespace_type);
                ns_identifier->inferred_type(true);
                ns_identifier->parent_element(scope->parent_element());
                scope->blocks().push_back(new_scope);
                scope->identifiers().add(ns_identifier);
                scope = new_scope;
            } else {
                auto expr = var->initializer()->expression();
                if (expr->element_type() == element_type_t::namespace_e) {
                    auto ns = dynamic_cast<namespace_element*>(expr);
                    scope = dynamic_cast<compiler::block*>(ns->expression());
                } else {
                    _program->error(
                        context.session,
                        "P018",
                        "only a namespace is valid within a qualified name.",
                        node->lhs->location);
                    return nullptr;
                }
            }
        }
        return scope;
    }

    void ast_evaluator::add_composite_type_fields(
            const evaluator_context_t& context,
            compiler::composite_type* type,
            const syntax::ast_node_t* block) {
        auto u32_type = _program->find_type(qualified_symbol_t {.name = "u32"});

        for (const auto& child : block->children) {
            if (child->type != syntax::ast_node_types_t::statement) {
                // XXX: this is an error!
                break;
            }
            auto expr_node = child->rhs;
            switch (expr_node->type) {
                case syntax::ast_node_types_t::assignment: {
                    const auto& target_list = expr_node->lhs;
                    const auto& source_list = expr_node->rhs;
                    for (size_t i = 0; i < target_list->children.size(); i++) {
                        const auto& symbol_node = target_list->children[i];
                        auto symbol = dynamic_cast<compiler::symbol_element*>(evaluate_in_scope(
                            context,
                            symbol_node.get(),
                            type->scope()));
                        type_find_result_t type_find_result{};
                        _program->find_identifier_type(
                            context.session,
                            type_find_result,
                            source_list->children[i],
                            type->scope());
                        auto init = _builder->make_initializer(
                            type->scope(),
                            evaluate_in_scope(
                                context,
                                source_list->children[i].get(),
                                type->scope()));
                        auto field_identifier = _builder->make_identifier(
                            type->scope(),
                            symbol,
                            init);
                        if (type_find_result.type == nullptr) {
                            type_find_result.type = init->expression()->infer_type(_program);
                            field_identifier->inferred_type(type_find_result.type != nullptr);
                        }
                        field_identifier->type(type_find_result.type);
                        auto new_field = _builder->make_field(
                            type->scope(),
                            field_identifier);
                        new_field->parent_element(type);
                        type->fields().add(new_field);
                    }
                    break;
                }
                case syntax::ast_node_types_t::symbol: {
                    auto symbol = dynamic_cast<compiler::symbol_element*>(evaluate_in_scope(
                        context,
                        expr_node.get(),
                        type->scope()));
                    type_find_result_t type_find_result {};
                    _program->find_identifier_type(
                        context.session,
                        type_find_result,
                        expr_node->rhs,
                        type->scope());
                    auto field_identifier = _builder->make_identifier(
                        type->scope(),
                        symbol,
                        nullptr);
                    if (type_find_result.type == nullptr) {
                        if (type->type() == composite_types_t::enum_type) {
                            field_identifier->type(u32_type);
                        } else {
                            field_identifier->type(_builder->make_unknown_type_from_find_result(
                                context.session,
                                type->scope(),
                                field_identifier,
                                type_find_result));
                        }
                    } else {
                        field_identifier->type(type_find_result.type);
                    }
                    auto new_field = _builder->make_field(
                        type->scope(),
                        field_identifier);
                    new_field->parent_element(type);
                    type->fields().add(new_field);
                    break;
                }
                default:
                    break;
            }
        }
    }

    compiler::identifier* ast_evaluator::add_identifier_to_scope(
            const evaluator_context_t& context,
            compiler::symbol_element* symbol,
            type_find_result_t& type_find_result,
            const syntax::ast_node_t* node,
            size_t source_index,
            compiler::block* parent_scope) {
        auto scope = symbol->is_qualified()
                     ? _program->current_top_level()
                     : parent_scope != nullptr ? parent_scope : _program->current_scope();

        scope = add_namespaces_to_scope(context, node, symbol, scope);

        syntax::ast_node_shared_ptr source_node = nullptr;
        if (node != nullptr) {
            source_node = node->rhs->children[source_index];
        }

        auto init_expr = (compiler::element*) nullptr;
        auto init = (compiler::initializer*) nullptr;
        if (node != nullptr) {
            init_expr = evaluate_in_scope(context, source_node.get(), scope);
            if (init_expr != nullptr) {
                if (init_expr->element_type() == element_type_t::symbol) {
                    auto init_symbol = dynamic_cast<compiler::symbol_element*>(init_expr);
                    init_expr = _builder->make_identifier_reference(
                        scope,
                        init_symbol->qualified_symbol(),
                        nullptr);
                }
                if (init_expr->is_constant()) {
                    init = _builder->make_initializer(scope, init_expr);
                }
            }
        }

        auto new_identifier = _builder->make_identifier(scope, symbol, init);
        apply_attributes(context, new_identifier, node);
        if (init_expr != nullptr) {
            if (init == nullptr)
                init_expr->parent_element(new_identifier);
            else {
                auto folded_expr = init_expr->fold(context.session);
                if (folded_expr != nullptr) {
                    init_expr = folded_expr;
                    auto old_expr = init->expression();
                    init->expression(init_expr);
                    _program->elements().remove(old_expr->id());
                }
            }
        }

        if (type_find_result.type == nullptr) {
            if (init_expr != nullptr) {
                type_find_result.type = init_expr->infer_type(_program);
                new_identifier->type(type_find_result.type);
                new_identifier->inferred_type(type_find_result.type != nullptr);
            }

            if (type_find_result.type == nullptr) {
                new_identifier->type(_builder->make_unknown_type_from_find_result(
                    context.session,
                    scope,
                    new_identifier,
                    type_find_result));
            }
        } else {
            new_identifier->type(type_find_result.type);
        }

        scope->identifiers().add(new_identifier);

        if (init != nullptr
            &&  init->expression()->element_type() == element_type_t::proc_type) {
            add_procedure_instance(
                context,
                dynamic_cast<procedure_type*>(init->expression()),
                source_node.get());
        }

        if (init == nullptr
            &&  init_expr == nullptr
            &&  new_identifier->type() == nullptr) {
            _program->error(
                context.session,
                "P019",
                fmt::format("unable to infer type: {}", new_identifier->symbol()->name()),
                new_identifier->symbol()->location());
            return nullptr;
        } else {
            if (init == nullptr && init_expr != nullptr) {
                auto assign_bin_op = _builder->make_binary_operator(
                    scope,
                    operator_type_t::assignment,
                    new_identifier,
                    init_expr);
                auto statement = _builder->make_statement(
                    scope,
                    label_list_t{},
                    assign_bin_op);
                add_expression_to_scope(scope, statement);
                statement->parent_element(scope);
            }
        }

        return new_identifier;
    }

    bool ast_evaluator::noop(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        return false;
    }

    bool ast_evaluator::symbol(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _builder->make_symbol_from_node(
            context.session.result(),
            context.node);
        return true;
    }

    bool ast_evaluator::attribute(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _builder->make_attribute(
            _program->current_scope(),
            context.node->token.value,
            evaluate(context.session, context.node->lhs.get()));
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::directive(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto expression = evaluate(context.session, context.node->lhs.get());
        auto directive_element = _builder->make_directive(
            _program->current_scope(),
            context.node->token.value,
            expression);
        directive_element->location(context.node->location);
        apply_attributes(context, directive_element, context.node);
        directive_element->evaluate(context.session, _program);
        result.element = directive_element;
        return true;
    }

    bool ast_evaluator::module(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto module_block = _builder->make_block(
            _program->_block,
            element_type_t::module_block);
        auto module = _builder->make_module(_program->_block, module_block);
        module->source_file(context.session.current_source_file());
        _program->_block->blocks().push_back(module_block);

        _program->push_scope(module_block);
        _program->_top_level_stack.push(module_block);

        for (auto it = context.node->children.begin();
             it != context.node->children.end();
             ++it) {
            auto expr = evaluate(
                context.session,
                (*it).get(),
                context.default_block_type);
            if (expr == nullptr)
                return false;
            add_expression_to_scope(module_block, expr);
            expr->parent_element(module);
        }

        _program->_top_level_stack.pop();
        result.element = module;

        return true;
    }

    bool ast_evaluator::module_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto expr = resolve_symbol_or_evaluate(context, context.node->rhs.get());
        auto reference = _builder->make_module_reference(
            _program->current_scope(),
            expr);

        std::string path;
        if (expr->is_constant() && expr->as_string(path)) {
            boost::filesystem::path source_path(path);
            auto current_source_file = context.session.current_source_file();
            if (current_source_file != nullptr
            &&  source_path.is_relative()) {
                source_path = boost::filesystem::absolute(
                    source_path,
                    current_source_file->path().parent_path());
            }
            auto source_file = context.session.add_source_file(source_path);
            auto module = _program->compile_module(context.session, source_file);
            if (module == nullptr) {
                _program->error(
                    context.session,
                    "C021",
                    "unable to load module.",
                    context.node->rhs->location);
                return false;
            }
            reference->module(module);
            result.element = reference;
        } else {
            _program->error(
                context.session,
                "C021",
                "expected string literal or constant string variable.",
                context.node->rhs->location);
            return false;
        }

        return true;
    }

    bool ast_evaluator::line_comment(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _builder->make_comment(
            _program->current_scope(),
            comment_type_t::line,
            context.node->token.value);
        return true;
    }

    bool ast_evaluator::block_comment(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _builder->make_comment(
            _program->current_scope(),
            comment_type_t::block,
            context.node->token.value);
        return true;
    }

    bool ast_evaluator::string_literal(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _builder->make_string(
            _program->current_scope(),
            context.node->token.value);
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::number_literal(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        switch (context.node->token.number_type) {
            case syntax::number_types_t::integer: {
                uint64_t value;
                if (context.node->token.parse(value) == syntax::conversion_result_t::success) {
                    if (context.node->token.is_signed()) {
                        result.element = _builder->make_integer(
                            _program->current_scope(),
                            common::twos_complement(value));
                    } else {
                        result.element = _builder->make_integer(_program->current_scope(), value);
                    }
                    result.element->location(context.node->location);
                    return true;
                } else {
                    _program->error(
                        context.session,
                        "P041",
                        "invalid integer literal",
                        context.node->location);
                }
                break;
            }
            case syntax::number_types_t::floating_point: {
                double value;
                if (context.node->token.parse(value) == syntax::conversion_result_t::success) {
                    result.element = _builder->make_float(
                        _program->current_scope(),
                        value);
                    result.element->location(context.node->location);
                    return true;
                } else {
                    _program->error(
                        context.session,
                        "P041",
                        "invalid float literal",
                        context.node->location);
                }
                break;
            }
            default:
                break;
        }

        return false;
    }

    bool ast_evaluator::boolean_literal(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _builder->make_bool(
            _program->current_scope(),
            context.node->token.as_bool());
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::namespace_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _builder->make_namespace(
            _program->current_scope(),
            evaluate(context.session, context.node->rhs.get(), context.default_block_type));
        return true;
    }

    bool ast_evaluator::expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _builder->make_expression(
            _program->current_scope(),
            evaluate(context.session, context.node->lhs.get()));
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::argument_list(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto args = _builder->make_argument_list(_program->current_scope());
        for (const auto& arg_node : context.node->children) {
            auto arg = resolve_symbol_or_evaluate(context, arg_node.get());
            args->add(arg);
            arg->parent_element(args);
        }
        result.element = args;
        return true;
    }

    bool ast_evaluator::unary_operator(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto it = s_unary_operators.find(context.node->token.type);
        if (it == s_unary_operators.end())
            return false;
        result.element = _builder->make_unary_operator(
            _program->current_scope(),
            it->second,
            resolve_symbol_or_evaluate(context, context.node->rhs.get()));
        return true;
    }

    bool ast_evaluator::binary_operator(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto it = s_binary_operators.find(context.node->token.type);
        if (it == s_binary_operators.end())
            return false;
        auto lhs = resolve_symbol_or_evaluate(context, context.node->lhs.get());
        auto rhs = resolve_symbol_or_evaluate(context, context.node->rhs.get());
        result.element = _builder->make_binary_operator(
            _program->current_scope(),
            it->second,
            lhs,
            rhs);
        return true;
    }

    bool ast_evaluator::cast_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto type_name = context.node->lhs->lhs->children[0]->token.value;
        auto type = _program->find_type(qualified_symbol_t {.name = type_name});
        if (type == nullptr) {
            _program->error(
                context.session,
                "P002",
                fmt::format("unknown type '{}'.", type_name),
                context.node->lhs->lhs->location);
            return false;
        }
        result.element = _builder->make_cast(
            _program->current_scope(),
            type,
            resolve_symbol_or_evaluate(context, context.node->rhs.get()));
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::alias_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _builder->make_alias(
            _program->current_scope(),
            resolve_symbol_or_evaluate(context, context.node->lhs.get()));
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::return_statement(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto return_element = _builder->make_return(_program->current_scope());
        auto& expressions = return_element->expressions();
        for (const auto& arg_node : context.node->rhs->children) {
            auto arg = resolve_symbol_or_evaluate(context, arg_node.get());
            expressions.push_back(arg);
            arg->parent_element(return_element);
        }
        result.element = return_element;
        return true;
    }

    bool ast_evaluator::import_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        qualified_symbol_t qualified_symbol {};
        _builder->make_qualified_symbol(qualified_symbol, context.node->lhs.get());

        compiler::identifier_reference* from_reference = nullptr;
        if (context.node->rhs != nullptr) {
            from_reference = dynamic_cast<compiler::identifier_reference*>(
                resolve_symbol_or_evaluate(context, context.node->rhs.get()));
            qualified_symbol.namespaces.insert(
                qualified_symbol.namespaces.begin(),
                from_reference->symbol().name);
        }

        auto identifier_reference = _builder->make_identifier_reference(
            _program->current_scope(),
            qualified_symbol,
            _program->find_identifier(qualified_symbol));
        auto import = _builder->make_import(
            _program->current_scope(),
            identifier_reference,
            from_reference,
            dynamic_cast<compiler::module*>(_program->current_top_level()->parent_element()));
        add_expression_to_scope(_program->current_scope(), import);

        result.element = import;

        return true;
    }

    bool ast_evaluator::basic_block(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto active_scope = _program->push_new_block(context.default_block_type);

        for (auto it = context.node->children.begin();
             it != context.node->children.end();
             ++it) {
            auto current_node = *it;
            auto expr = evaluate(
                context.session,
                current_node.get(),
                context.default_block_type);
            if (expr == nullptr) {
                _program->error(
                    context.session,
                    "C024",
                    "invalid statement",
                    current_node->location);
                return false;
            }
            add_expression_to_scope(active_scope, expr);
            expr->parent_element(active_scope);
        }

        result.element = _program->pop_scope();
        return true;
    }

    bool ast_evaluator::proc_call(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        qualified_symbol_t qualified_symbol {};
        _builder->make_qualified_symbol(qualified_symbol, context.node->lhs.get());
        auto proc_identifier = _program->find_identifier(qualified_symbol);

        compiler::argument_list* args = nullptr;
        auto expr = evaluate(context.session, context.node->rhs.get());
        if (expr != nullptr) {
            args = dynamic_cast<compiler::argument_list*>(expr);
        }
        result.element = _builder->make_procedure_call(
            _program->current_scope(),
            _builder->make_identifier_reference(
                _program->current_scope(),
                qualified_symbol,
                proc_identifier),
            args);
        result.element->location(context.node->location);

        return true;
    }

    bool ast_evaluator::statement(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        label_list_t labels {};

        if (context.node->lhs != nullptr) {
            for (const auto& label : context.node->lhs->children) {
                labels.push_back(_builder->make_label(
                    _program->current_scope(),
                    label->token.value));
            }
        }

        auto expr = evaluate(context.session, context.node->rhs.get());
        if (expr == nullptr)
            return false;

        if (expr->element_type() == element_type_t::symbol) {
            type_find_result_t find_type_result {};
            _program->find_identifier_type(
                context.session,
                find_type_result,
                context.node->rhs->rhs);
            expr = add_identifier_to_scope(
                context,
                dynamic_cast<compiler::symbol_element*>(expr),
                find_type_result,
                nullptr,
                0);
        }

        result.element = _builder->make_statement(
            _program->current_scope(),
            labels,
            expr);

        return true;
    }

    bool ast_evaluator::enum_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto active_scope = _program->current_scope();
        auto enum_type = _builder->make_enum_type(
            context.session.result(),
            active_scope,
            _builder->make_block(active_scope, element_type_t::block));
        active_scope->types().add(enum_type);
        add_composite_type_fields(
            context,
            enum_type,
            context.node->rhs.get());
        if (!enum_type->initialize(context.session))
            return false;
        result.element = enum_type;

        return true;
    }

    bool ast_evaluator::struct_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto active_scope = _program->current_scope();
        auto struct_type = _builder->make_struct_type(
            context.session.result(),
            active_scope,
            _builder->make_block(active_scope, element_type_t::block));
        active_scope->types().add(struct_type);
        add_composite_type_fields(
            context,
            struct_type,
            context.node->rhs.get());
        if (!struct_type->initialize(context.session))
            return false;
        result.element = struct_type;
        return true;
    }

    bool ast_evaluator::union_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto active_scope = _program->current_scope();
        auto union_type = _builder->make_union_type(
            context.session.result(),
            active_scope,
            _builder->make_block(active_scope, element_type_t::block));
        active_scope->types().add(union_type);
        add_composite_type_fields(
            context,
            union_type,
            context.node->rhs.get());
        if (!union_type->initialize(context.session))
            return false;
        result.element = union_type;
        return true;
    }

    bool ast_evaluator::else_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = evaluate(context.session, context.node->children[0].get());
        return true;
    }

    bool ast_evaluator::if_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto predicate = evaluate(context.session, context.node->lhs.get());
        auto true_branch = evaluate(context.session, context.node->children[0].get());
        auto false_branch = evaluate(context.session, context.node->rhs.get());
        result.element = _builder->make_if(
            _program->current_scope(),
            predicate,
            true_branch,
            false_branch);
        return true;
    }

    bool ast_evaluator::proc_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto active_scope = _program->current_scope();
        auto block_scope = _builder->make_block(
            active_scope,
            element_type_t::proc_type_block);
        auto proc_type = _builder->make_procedure_type(active_scope, block_scope);
        active_scope->types().add(proc_type);

        auto count = 0;
        for (const auto& type_node : context.node->lhs->children) {
            switch (type_node->type) {
                case syntax::ast_node_types_t::symbol: {
                    auto return_identifier = _builder->make_identifier(
                        block_scope,
                        _builder->make_symbol(block_scope, fmt::format("_{}", count++)),
                        nullptr);
                    return_identifier->usage(identifier_usage_t::stack);
                    return_identifier->type(_program->find_type(qualified_symbol_t {
                        .name = type_node->children[0]->token.value
                    }));
                    auto new_field = _builder->make_field(block_scope, return_identifier);
                    proc_type->returns().add(new_field);
                    new_field->parent_element(proc_type);
                    break;
                }
                default: {
                    break;
                }
            }
        }

        for (const auto& param_node : context.node->rhs->children) {
            switch (param_node->type) {
                case syntax::ast_node_types_t::assignment: {
                    // XXX: in the parameter list, multiple targets is an error
                    const auto& first_target = param_node->lhs->children[0];
                    auto symbol = dynamic_cast<compiler::symbol_element*>(evaluate_in_scope(
                        context,
                        first_target.get(),
                        block_scope));
                    type_find_result_t find_type_result {};
                    _program->find_identifier_type(
                        context.session,
                        find_type_result,
                        first_target->rhs,
                        block_scope);
                    auto param_identifier = add_identifier_to_scope(
                        context,
                        symbol,
                        find_type_result,
                        param_node.get(),
                        0,
                        block_scope);
                    param_identifier->usage(identifier_usage_t::stack);
                    auto field = _builder->make_field(block_scope, param_identifier);
                    proc_type->parameters().add(field);
                    field->parent_element(proc_type);
                    break;
                }
                case syntax::ast_node_types_t::symbol: {
                    auto symbol = dynamic_cast<compiler::symbol_element*>(evaluate_in_scope(
                        context,
                        param_node.get(),
                        block_scope));
                    type_find_result_t find_type_result {};
                    _program->find_identifier_type(
                        context.session,
                        find_type_result,
                        param_node->rhs,
                        block_scope);
                    auto param_identifier = add_identifier_to_scope(
                        context,
                        symbol,
                        find_type_result,
                        nullptr,
                        0,
                        block_scope);
                    if (param_identifier != nullptr) {
                        param_identifier->usage(identifier_usage_t::stack);
                        auto field = _builder->make_field(block_scope, param_identifier);
                        proc_type->parameters().add(field);
                        field->parent_element(proc_type);
                    } else {
                        _program->error(
                            context.session,
                            "P014",
                            fmt::format("invalid parameter declaration: {}", symbol->name()),
                            symbol->location());
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        result.element = proc_type;

        return true;
    }

    bool ast_evaluator::assignment(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        const auto& target_list = context.node->lhs;
        const auto& source_list = context.node->rhs;

        if (target_list->children.size() != source_list->children.size()) {
            _program->error(
                context.session,
                "P027",
                "the number of left-hand-side targets must match"
                " the number of right-hand-side expressions.",
                source_list->location);
            return false;
        }

        element_list_t list {};

        for (size_t i = 0; i < target_list->children.size(); i++) {
            const auto& symbol_node = target_list->children[i];

            qualified_symbol_t qualified_symbol {};
            _builder->make_qualified_symbol(qualified_symbol, symbol_node.get());
            auto existing_identifier = _program->find_identifier(qualified_symbol);
            if (existing_identifier != nullptr) {
                auto rhs = evaluate(
                    context.session,
                    source_list->children[i].get());
                if (rhs == nullptr)
                    return false;
                auto binary_op = _builder->make_binary_operator(
                    _program->current_scope(),
                    operator_type_t::assignment,
                    existing_identifier,
                    rhs);
                apply_attributes(context, binary_op, context.node);
                list.emplace_back(binary_op);
            } else {
                auto symbol = dynamic_cast<compiler::symbol_element*>(evaluate(
                    context.session,
                    symbol_node.get()));
                type_find_result_t find_type_result {};
                _program->find_identifier_type(
                    context.session,
                    find_type_result,
                    symbol_node->rhs);
                auto new_identifier = add_identifier_to_scope(
                    context,
                    symbol,
                    find_type_result,
                    context.node,
                    i);
                if (new_identifier == nullptr)
                    return false;
                list.emplace_back(new_identifier);
            }
        }

        result.element = list.front();

        return true;
    }

    bool ast_evaluator::transmute_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto type_name = context.node->lhs->lhs->children[0]->token.value;
        auto type = _program->find_type(qualified_symbol_t {.name = type_name});
        if (type == nullptr) {
            _program->error(
                context.session,
                "P002",
                fmt::format("unknown type '{}'.", type_name),
                context.node->lhs->lhs->location);
            return false;
        }
        result.element = _builder->make_transmute(
            _program->current_scope(),
            type,
            resolve_symbol_or_evaluate(context, context.node->rhs.get()));
        result.element->location(context.node->location);
        return true;
    }

};