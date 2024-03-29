#!/bin/bash

# Path to your compiler
COMPILER=./ac

# Directory containing test cases
TEST_DIR=./tests

# Temporary output binary name
OUTPUT_ASM=test.asm
OUTPUT_OBJECT_FILE=test.o
OUTPUT_BINARY=test.out

HAD_ANY_FAILURES=0

# Loop through all .c files in the test directory
for test_file in $TEST_DIR/*.c; do
    echo "Compiling $test_file..."

    expected_status=$(awk -F'//' 'NR==1 {print $2 + 0}' $test_file)

    # Use your compiler to compile the current test case into a binary named `test.out`
    $COMPILER $test_file
    compile_status=$?

    # Check if compilation was successful
    if [ $compile_status -eq 0 ]; then
        nasm -f elf64 -g $OUTPUT_ASM -o $OUTPUT_OBJECT_FILE
        gcc -o $OUTPUT_BINARY $OUTPUT_OBJECT_FILE -nostartfiles -lc
        
        # Run the compiled binary
        ./$OUTPUT_BINARY
        run_status=$?

        # Check the exit status of the program against the expected status
        if [ $run_status -eq $expected_status ]; then
            echo "PASS: $test_file"
        else
            echo "FAIL: $test_file (Expected exit code: $expected_status, Actual exit code: $run_status)"
            HAD_ANY_FAILURES=1
        fi
    else
        echo "Compilation failed for $test_file (Compiler exit code: $compile_status)"
        HAD_ANY_FAILURES=1
    fi

    # Optional: Remove the output binary to clean up before the next test
    rm -f $OUTPUT_BINARY
    rm -f $OUTPUT_ASM
    rm -f $OUTPUT_OBJECT_FILE

    # if had any failures
    if [ $HAD_ANY_FAILURES -eq 1 ]; then
        echo "Some tests failed"
        exit 1
    fi
done