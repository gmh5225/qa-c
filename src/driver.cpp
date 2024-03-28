#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "lexer.hpp"
#include "parser.hpp"
#include "st.hpp"
#include "translate.hpp"

[[nodiscard]] const std::string readfile(const char *sourcefile) {
    std::ifstream inFile;
    inFile.open(sourcefile);
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    return strStream.str();
}

int runfile(const char *sourcefile) {
    const auto contents = readfile(sourcefile);
    const auto tokens = lex(contents);
    const auto st = parse(tokens);
    for (auto &v : st.nodes) {
        std::cout << v << std::endl;
    }
    const auto ast = translate(st);
    std::cout << "AST:" << std::endl;
    for (const auto &node : ast) {
        std::cout << *node << std::endl;
    }
    return 0;
}
