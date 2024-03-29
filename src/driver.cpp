#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "allocator.hpp"
#include "assem.hpp"
#include "codegen.hpp"
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
    auto frames = as::As_Instructions(ast);
    for (const auto &f : frames) {
        std::cout << f.name << std::endl;
        for (const auto &ins : f.instructions) {
            std::visit([](const auto &i) {
                std::cout << i << std::endl;
            }, ins);
        }
    }
    std::cout << "-----------------" << std::endl;
    auto rewritten = allocator::rewrite(frames);
    for (const auto &f : rewritten) {
        std::cout << f.name << std::endl;
        for (const auto &ins : f.instructions) {
            std::visit([](const auto &i) {
                std::cout << i << std::endl;
            }, ins);
        }
    }
    auto code = codegen::Generate(rewritten);
    // write code to file
    std::ofstream outFile;
    outFile.open("test.asm");
    outFile << code;
    outFile.close();
    return 0;
}
