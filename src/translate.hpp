#pragma once

#include <vector>

#include "ast.hpp"
#include "st.hpp"

[[nodiscard]] std::vector<std::unique_ptr<ast::Node>>
        translate(const st::Program &program);