# BASIC Interpreter Test Suite

This directory contains the test suite for the BASIC interpreter. The tests are designed to be easy to run and extend.

## Test Structure

The test suite is organized into three main parts:

-   `tests/cases/`: This directory contains all the test cases, which are written as `.bas` files. Each file represents a specific feature or scenario to be tested.
-   `tests/expected/`: This directory contains the expected output for each test case. For each `test_name.bas` file in `tests/cases/`, there is a corresponding `test_name.bas.expected` file in this directory.
-   `tests/run_all_tests.sh`: This is the main test runner script. It executes all the test cases and compares their output to the expected output.

## Running the Tests

To run the entire test suite, simply execute the `run_all_tests.sh` script from the root of the repository:

```bash
./tests/run_all_tests.sh
```

The script will build the interpreter, run all the tests, and provide a summary of the results. If a test fails, the script will print a diff of the actual output versus the expected output.

## Adding New Tests

To add a new test, follow these steps:

1.  **Create a new `.bas` file** in the `tests/cases/` directory. The file should contain the BASIC code for your test case. Give it a descriptive name, like `my_new_feature.bas`.

2.  **Generate the expected output.** After creating your `.bas` file, you need to generate its corresponding `.expected` file. You can do this by temporarily modifying the `tests/run_all_tests.sh` script to not fail on a diff, and instead redirect the output. Or, you can run the test case manually and save its output:

    ```bash
    (echo "NEW"; cat tests/cases/my_new_feature.bas; echo "RUN") | ./altair_basic > tests/expected/my_new_feature.bas.expected 2>&1
    ```

    Make sure to manually inspect the generated `.expected` file to ensure it is correct.

3.  **Run the test suite.** Run `./tests/run_all_tests.sh` to confirm that your new test passes and that you haven't introduced any regressions.
