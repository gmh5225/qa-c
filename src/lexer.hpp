#pragma once

#include <string>
#include <vector>

#include "token.hpp"

[[nodiscard]] std::vector<Token> lex(const std::string &source);