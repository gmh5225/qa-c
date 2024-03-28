#pragma once

#include <vector>

#include "st.hpp"
#include "token.hpp"

[[nodiscard]] st::Program parse(const std::vector<Token> &tokens);