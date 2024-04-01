#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "allocator.hpp"
#include "assem.hpp"
#include "codegen.hpp"
#include "lexer.hpp"
#include "lower_ir.hpp"
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
  std::cout << "IR:" << std::endl;
  std::cout << "-----------------" << std::endl;
  std::vector<target::Frame> lowered_frames = target::LowerIR(frames);
  auto rewritten = allocator::rewrite(lowered_frames);
  auto code = codegen::Generate(lowered_frames);
  // write code to file
  std::ofstream outFile;
  outFile.open("test.asm");
  outFile << code;
  outFile.close();
  return 0;
}
