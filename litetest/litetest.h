#ifndef LITETEST_H
#define LITETEST_H

#include "internal.h"

namespace litetest {

/**
 * Defines a test suite.
 * A test suite consists in a named group of test cases.
 * Each test suite may contain a setup or a cleanup procedure, defined
 * respectively by SUITE_SETUP() and SUITE_CLEANUP(). These procedures are
 * guaranteed to be invoked before and after all test cases from the suite
 * are executed.
 *
 * Usage: TEST_SUITE(your_suite_name);
 */
#define TEST_SUITE(name) \
    static auto s_suite##name = []() { \
        return litetest::internal::push_suite(#name, __FILE__, __LINE__); \
    }()

/**
 * Defines a test case.
 * A test case must be preceded by a declaration of a test suite.
 *
 * Usage: TEST_CASE(your_case_name) {
 *      // Your test case code here...
 * }
 */
#define TEST_CASE(name) \
    static void case_##name(); \
    static auto s_case_##name = []() { \
        return litetest::internal::push_case(#name, case_##name, __FILE__, __LINE__); \
    }(); \
    static void case_##name()

/**
 * Defines setup code for a test suite.
 * The test suite setup code is guaranteed to be executed before
 * the suite's test cases are run.
 * Perform any initialization code required to run the tests here.
 *
 * Usage: SUITE_SETUP(your_suite_name) {
 *      // Your setup code here...
 * }
 */
#define SUITE_SETUP(suite_name) \
    static void setup_##suite_name(); \
    static auto s_suite_setup_##suite_name = []() { \
        return litetest::internal::push_suite_setup(#suite_name, __FILE__, setup_##suite_name);  \
    }();                                            \
    static void setup_##suite_name()

/**
 * Defines setup cleanup for a test suite.
 * The test suite cleanup code is guaranteed to be executed after
 * the suite's test cases are run.
 * Perform any resource cleanup required to run after the
 * suite's tests - independently of whether they fail, succeed or throw - here.
 *
 * Usage: SUITE_CLEANUP(your_suite_name) {
 *      // Your setup code here...
 * }
 */
#define SUITE_CLEANUP(suite_name) \
    static void cleanup_##suite_name(); \
    static auto s_suite_cleanup_##suite_name = []() {                     \
        return litetest::internal::push_suite_cleanup(#suite_name, __FILE__, cleanup_##suite_name);  \
    }(); \
    static void cleanup_##suite_name() \


/**
 * Main function for testing. Receives a value to be tested against other values
 * or by itself. Fails the current test case by throwing TestFailure if the requested
 * test fails.
 *
 * Usage examples:
 *      expect(foo()).to_be("some expected string");
 *      expect(sum(5,2)).to_be(7);
 *      expect(cost_of("something")).to_be_greater_than(200);
 *
 */
template <typename T>
internal::ExpectValue<T> expect(const T& val) {
    return internal::ExpectValue(val);
}

/**
 * Test arguments to be passed to run_tests().
 */
struct RunTestsArgs {};

struct RunTestsResults {

    /** Number of cases that were executed. */
    int n_cases_executed = 0;

    /** Number of test cases that finished without failing. */
    int n_cases_passed = 0;

    /** Number of test cases that threw an exception that wasn't TestFailure. */
    int n_cases_incomplete = 0;
};

/**
 * Executes all tests and returns an object with the test results.
 */
RunTestsResults run_tests(RunTestsArgs args = {});

/**
 * If desired, litetest_main automatically performs all tests and logs results to stdout.
 * Returns the number of failed + incomplete cases.
 */
int litetest_main(int argc, char* argv[]);

}

#endif // LITETEST_H
