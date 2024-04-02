#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "allocator.hpp"
#include "assem.hpp"
#include "codegen.hpp"
#include "lexer.hpp"
#include "lower_ir.hpp"
#include "optimizer.hpp"
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
    auto frames = qa_ir::Produce_IR(ast);
    std::cout << "-----------------" << std::endl;
    std::cout << "IR:" << std::endl;
    for (const auto &frame : frames) {
        for (const auto &ins : frame.instructions) {
            std::cout << ins << std::endl;
        }
    }
    std::cout << "-----------------" << std::endl;
    std::vector<target::Frame> lowered_frames = target::LowerIR(frames);
    std::cout << "LOWER_IR:" << std::endl;
    for (const auto &frame : lowered_frames) {
        for (const auto &ins : frame.instructions) {
            std::cout << ins << std::endl;
        }
    }
    std::cout << "-----------------" << std::endl;
    std::cout << "OPTIMIZED LOWER_IR:" << std::endl;
    auto optimized_frames = optimizer::optimize_lowered(lowered_frames);
    for (const auto &frame : optimized_frames) {
        for (const auto &ins : frame.instructions) {
            std::cout << ins << std::endl;
        }
    }
    std::cout << "REGISTER_ALLOCATION: :" << std::endl;
    auto rewritten = allocator::rewrite(lowered_frames);
    for (const auto &frame : rewritten) {
        for (const auto &ins : frame.instructions) {
            std::cout << ins << std::endl;
        }
    }
    std::cout << "-----------------" << std::endl;
    auto code = codegen::Generate(rewritten);
    // write code to file
    std::ofstream outFile;
    outFile.open("test.asm");
    outFile << code;
    outFile.close();
    return 0;
}
