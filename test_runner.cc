#include <gtest/gtest.h>

#include <cstdlib>
#include <expected>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>

constexpr std::string compiler_path = "./build/bin/qac";
constexpr std::string temp_dir = "./tmp/";
constexpr std::string test_dir = "./tests/sources";
constexpr std::string asm_path = "test.asm";
const std::string compiler_gen_asm_path = temp_dir + asm_path;
const std::string compiler_gen_object_path = temp_dir + "test.o";
const std::string compiler_gen_binary_path = temp_dir + "test.out";

[[nodiscard]] auto invoke_qac(const std::string& sourcePath)
    -> std::expected<int, std::string> {
    const auto command = compiler_path.data() + std::string(" ") + sourcePath +
                         " -o " + compiler_gen_asm_path;
    const auto result = system(command.c_str());
    if (result != 0) {
        return std::unexpected("Failed to compile the source file");
    }
    return result;
}

[[nodiscard]] auto nasm_assemble_elf64(const std::string& asmPath,
                                       const std::string& objectPath)
    -> std::expected<int, std::string> {
    const auto command = "nasm -f elf64 -o " + objectPath + " " + asmPath;
    const auto result = system(command.c_str());
    if (result != 0) {
        return std::unexpected("Failed to compile the asm file");
    }
    return result;
}

[[nodiscard]] auto gcc_link_standalone_binary(const std::string& objectPath,
                                              const std::string& binaryPath)
    -> std::expected<int, std::string> {
    const auto command =
        "gcc -o " + binaryPath + " " + objectPath + " -nostartfiles -lc";
    const auto result = system(command.c_str());
    if (result != 0) {
        return std::unexpected("Failed to link the object file");
    }
    return result;
}

[[nodiscard]] auto compile(const std::string& sourcePath)
    -> std::expected<int, std::string> {
    const auto compileResult = invoke_qac(sourcePath);
    if (!compileResult) {
        return std::unexpected(compileResult.error());
    }

    const auto assembleResult =
        nasm_assemble_elf64(compiler_gen_asm_path, compiler_gen_object_path);
    if (!assembleResult) {
        return std::unexpected(assembleResult.error());
    }

    const auto linkResult = gcc_link_standalone_binary(
        compiler_gen_object_path, compiler_gen_binary_path);
    if (!linkResult) {
        return std::unexpected(linkResult.error());
    }

    return 0;
}

[[nodiscard]] auto parse_expected_return_from_source(
    const std::string& sourcePath) -> std::expected<int, std::string> {
    std::ifstream file(sourcePath);
    if (!file.is_open()) {
        return std::unexpected("Failed to open the source file");
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.starts_with("// EXPECTED_RETURN: ")) {
            return std::stoi(line.substr(19));
        }
    }
    return std::unexpected("Expected return value not found");
}

[[nodiscard]] auto run_test_for_status_code(const std::string& sourcePath)
    -> std::expected<bool, std::string> {
    const auto expectedReturn = parse_expected_return_from_source(sourcePath);

    if (!expectedReturn) {
        return std::unexpected(expectedReturn.error());
    }
    const auto expected_return_code = expectedReturn.value();

    const auto compileResult = compile(sourcePath);
    if (!compileResult) {
        return std::unexpected(compileResult.error());
    }
    const auto result = system(compiler_gen_binary_path.data());
    if (WIFEXITED(result)) {
        int normal_exit_status = WEXITSTATUS(result);
        if (normal_exit_status != expected_return_code) {
            const auto message = "Expected return value: " +
                                 std::to_string(expected_return_code) +
                                 " but got: " + std::to_string(result);
            return std::unexpected(message);
        }
    } else {
        return std::unexpected("Compiled binary crashed during execution");
    }

    return true;
}

#define RUN_TEST_CASE(NAME, PATH)                                           \
    TEST(CompilerIntegrationTest, NAME) {                                   \
        auto result =                                                       \
            run_test_for_status_code("tests/sources/" + std::string(PATH)); \
        if (!result) {                                                      \
            std::cerr << result.error() << std::endl;                       \
            FAIL();                                                         \
        }                                                                   \
        SUCCEED();                                                          \
    }

/** Basic **/
RUN_TEST_CASE(IntReturnValue, "int_return.c");
RUN_TEST_CASE(IntAssignment, "int_assignment.c");

/** Addition **/
RUN_TEST_CASE(IntVariableAddition, "int_variable_addition.c");

/** For loop test cases  **/
RUN_TEST_CASE(ForLoopIncrement, "for_loop_increment.c");
RUN_TEST_CASE(ForLoopDecrement, "for_loop_decrement.c");

/** Int* pointers  **/
RUN_TEST_CASE(IntSwap, "int_swap.c");
RUN_TEST_CASE(IntDerefAddition, "int_deref_addition.c");

/** Comparision operators  **/
RUN_TEST_CASE(IntEqualityEq_NegativeCase, "int_equality_eq_negative.c");
RUN_TEST_CASE(IntEqualityEq_PositiveCase, "int_equality_eq_positive.c");

RUN_TEST_CASE(IntEqualityNeq_NegativeCase, "int_equality_neq_negative.c");
RUN_TEST_CASE(IntEqualityNeq_PositiveCase, "int_equality_neq_positive.c");

RUN_TEST_CASE(IntGreaterThan_And_IfStatements, "int_gt_if_statements.c");

RUN_TEST_CASE(IntSubtractionInvolvingNegatives,
              "int_subtraction_involving_negative.c");

/** Parameter Passing  **/
RUN_TEST_CASE(PassVariablesOnStack, "pass_vars_on_stack.c");
RUN_TEST_CASE(PassVariablesOnStackMoreInvolved,
              "pass_vars_on_stack_more_involved.c");

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
