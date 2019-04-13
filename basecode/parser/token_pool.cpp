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

#include "token_pool.h"

namespace basecode::syntax {

    token_pool* token_pool::instance() {
        static token_pool s_instance {};
        return &s_instance;
    }

    ///////////////////////////////////////////////////////////////////////////

    token_t* token_pool::find(common::id_t id) {
        auto it = _tokens.find(id);
        if (it == std::end(_tokens))
            return nullptr;
        return &it->second;
    }

    token_t* token_pool::add(token_type_t type) {
        auto id = common::id_pool::instance()->allocate();
        auto result = _tokens.insert(std::make_pair(
            id,
            token_t{.id = id, .type = type}));
        return &result.first->second;
    }

    token_t* token_pool::add(token_type_t type, const std::string_view& value) {
        auto id = common::id_pool::instance()->allocate();
        auto result = _tokens.insert(std::make_pair(
            id,
            token_t{.id = id, .type = type, .value = value}));
        return &result.first->second;
    }

}