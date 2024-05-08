#include "litetest.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace litetest {
namespace internal {

static void initialize();

static std::vector<TestCase>* s_cases = []() {
    initialize();
    return s_cases;
}();

static std::vector<TestSuite>* s_suites = []() {
    initialize();
    return s_suites;
}();

static std::unordered_map<std::string, std::function<void()>>* s_setups = []() {
    initialize();
    return s_setups;
}();

static std::unordered_map<std::string, std::function<void()>>* s_cleanups = []() {
    initialize();
    return s_cleanups;
}();

static void initialize() {
    if (!s_suites) {
        s_suites = new std::vector<TestSuite>();
    }
    if (!s_cases) {
        s_cases = new std::vector<TestCase>();
    }
    if (!s_setups) {
        s_setups = new std::unordered_map<std::string, std::function<void()>>();
    }
    if (!s_cleanups) {
        s_cleanups = new std::unordered_map<std::string, std::function<void()>>();
    }
}

TestCase* push_case(const std::string& name,
                    std::function<void()> fn,
                    const std::string& src_file,
                    int line) {
    initialize();
    s_cases->push_back({name, std::move(fn), src_file, line,});
    return &*(s_cases->rbegin());
}

TestSuite* push_suite(const std::string& name,
                      const std::string& src_file,
                      int line) {
    initialize();
    s_suites->push_back({name, src_file, line});
    return &*(s_suites->rbegin());
}

std::function<void()> push_suite_setup(const std::string& suite_name,
                                       const std::string& suite_file,
                                       std::function<void()> setup) {
    initialize();
    (*s_setups)[suite_file + "/" + suite_name] = setup;
    return setup;
}

std::function<void()> push_suite_cleanup(const std::string& suite_name,
                                         const std::string& suite_file,
                                         std::function<void()> cleanup) {
    initialize();
    (*s_cleanups)[suite_file + "/" + suite_name] = cleanup;
    return cleanup;
}

using FileSuiteMap = std::unordered_map<std::string, std::vector<TestSuite*>>;

static FileSuiteMap generate_file_suite_map() {
    FileSuiteMap suites_by_file;
    for (auto& suite: *s_suites) {
        suites_by_file[suite.src_file].push_back(&suite);
    }

    for (auto& kv: suites_by_file) {
        std::sort(kv.second.begin(), kv.second.end(), [](auto a, auto b) {
            return a->line < b->line;
        });
    }
    return suites_by_file;
}

static TestSuite* find_matching_suite(const FileSuiteMap& suites_by_file,
                                      const std::string& file_name,
                                      int line) {
    auto it = suites_by_file.find(file_name);
    if (it != suites_by_file.end()) {
        auto& vec = it->second;
        if (vec.begin() == vec.end()) {
            // Empty vector, no suites.
            return nullptr;
        }
        TestSuite* matching_suite = nullptr;
        for (TestSuite* suite: vec) {
            if (suite->line > line) {
                // Suite was declared after the test case.
                // Since the vector is sorted, we can assume that all
                // further suites also fall into this condition, so we abort.
                break;
            }
            matching_suite = suite;
        }
        return matching_suite;
    }
    return nullptr;
}

static void arrange_suites_and_functions(const FileSuiteMap& suites_by_file) {
    for (auto& suite: *s_suites) {
        // Fallback to empty functions.
        suite.setup = []() {};
        suite.cleanup = []() {};

        std::string fn_identifier = suite.src_file + "/" + suite.name;

        auto setups_it = s_setups->find(fn_identifier);
        if (setups_it != s_setups->end()) {
            suite.setup = setups_it->second;
        }

        auto cleanups_it = s_cleanups->find(fn_identifier);
        if (cleanups_it != s_cleanups->end()) {
            suite.cleanup = cleanups_it->second;
        }
    }
}

static void arrange_cases_and_suites(FileSuiteMap& suites_by_file) {
    for (auto& test_case: *s_cases) {
        TestSuite* selected_suite = find_matching_suite(suites_by_file,
                                                        test_case.src_file,
                                                        test_case.line);

        if (selected_suite == nullptr) {
            throw std::runtime_error(std::string("Test case ") + test_case.name + " has no suite.");
        }

        selected_suite->cases.push_back(&test_case);
    }
}

std::vector<TestSuite*> process_suites() {
    FileSuiteMap suites_by_file = generate_file_suite_map();

    arrange_cases_and_suites(suites_by_file);
    arrange_suites_and_functions(suites_by_file);

    std::vector<TestSuite*> suites;
    for (auto& suite: *s_suites) {
        suites.push_back(&suite);
    }
    return suites;
}

static std::unordered_map<std::thread::id, TestCase*> s_current_case;
static std::unordered_map<std::thread::id, TestSuite*> s_current_suite;

const TestCase& current_case(std::thread::id thread_id) {
    return *s_current_case.at(thread_id);
}

const TestSuite& current_suite(std::thread::id thread_id) {
    return *s_current_suite.at(thread_id);
}

<<<<<<< HEAD
=======
std::atomic_int g_assert_count = 0;

>>>>>>> main
} // internal
using namespace internal;

static bool has_suite_in_args(const RunTestsArgs& args,
                              const std::string& suite_name) {
    return args.suites.empty() ||
           std::any_of(args.suites.begin(), args.suites.end(), [&suite_name](const std::string& name) {
               return name == suite_name;
           });
}

RunTestsResults run_tests(RunTestsArgs args) {
    RunTestsResults results;

    auto suites = process_suites();
    for (TestSuite* suite: suites) {
        // We might want to skip some suites if the user says so.
        if (!has_suite_in_args(args, suite->name)) {
            continue;
        }

        s_current_suite[std::this_thread::get_id()] = suite;
        suite->setup();

        for (TestCase* test_case: suite->cases) {
            s_current_case[std::this_thread::get_id()] = test_case;

            bool passed = false;
            try {
                results.n_cases_executed++;
                test_case->function();
                passed = true;
            }
            catch (const TestFailure& test_failure) {
                std::cout << "Test case '" << test_case->name << "' (assertion at line " << test_failure.line << ") failed:\n\t" << test_failure.what() << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Test case '" << test_case->name << "' threw an unexpected exception:\n" << e.what() << std::endl;
                results.n_cases_incomplete++;
            }

            if (passed) {
                results.n_cases_passed++;
            }
        }

        suite->cleanup();
    }

    return results;
}

enum class ExecutionMode {
    NORMAL,
    LIST_SUITES,
    UNKNOWN
};

struct Argument {
    std::string name;
    std::vector<std::string> parameters;
};

class ProgramArgs {
public:
    ProgramArgs(int argc, char* argv[]);

    ExecutionMode exec_mode() const;
    const std::string& exec_path() const;
    bool has_arg(const std::string& arg_name) const;
    std::optional<Argument> get_arg(const std::string& arg_name) const;

private:
    ExecutionMode m_mode;
    std::string m_exec_path;
    std::unordered_map<std::string, Argument> m_args;
};

ProgramArgs::ProgramArgs(int argc, char** argv) {
    m_mode = ExecutionMode::NORMAL;
    m_exec_path = argv[0];

    if (argc <= 1) {
        return;
    }

    argc--; argv++;

    // Check for alternative execution modes.
    std::string first_arg = argv[0];
    if (first_arg[0] != '-') {
        // User has specified an execution mode.
        std::unordered_map<std::string, ExecutionMode> exec_modes {
            { "suites", ExecutionMode::LIST_SUITES }
        };
        auto it = exec_modes.find(first_arg);
        if (it == exec_modes.end()) {
            m_mode = ExecutionMode::UNKNOWN;
        }
        else {
            m_mode = it->second;
        }

        // We'll parse arguments afterwards, so ignore this first one when
        // we do it.
        argc--; argv++;
    }

    // Parse specified arguments.
    std::string current_arg;
    for (int i = 0; i < argc; ++i) {
        std::string s = std::string(argv[i]);
        if (s[0] == '-') {
            // We're dealing with an argument.
            // Check if it's a parameterized arg.
            if (s.size() > 1 && s[1] == '-') {
                // Not parameterized (--arg).
                current_arg = s.substr(2);
            }
            else {
                // Parameterized (-arg).
                current_arg = s.substr(1);
            }
            m_args[current_arg].name = current_arg;
        }
        else {
            // We're dealing with an argument's parameter.
            m_args[current_arg].parameters.push_back(s);
        }
    }
}

const std::string& ProgramArgs::exec_path() const {
    return m_exec_path;
}

std::optional<Argument> ProgramArgs::get_arg(const std::string& arg_name) const {
    auto it = m_args.find(arg_name);
    if (it == m_args.end()) {
        return std::nullopt;
    }
    return it->second;
}

ExecutionMode ProgramArgs::exec_mode() const {
    return m_mode;
}

bool ProgramArgs::has_arg(const std::string& arg_name) const {
    return get_arg(arg_name).has_value();
}

static int run_mode_normal(const ProgramArgs& args) {
    RunTestsArgs test_args;

    if (args.has_arg("only")) {
        test_args.suites = args.get_arg("only")->parameters;
    }

    RunTestsResults results = run_tests(test_args);

    std::cout.flush();
    std::cerr.flush();

    std::cout << "Testing finished." << std::endl;
    std::cout << results.n_cases_passed << " of " << results.n_cases_executed << " passed (" << internal::g_assert_count << " total assertions made)." << std::endl;
    if (results.n_cases_incomplete) {
        std::cout << results.n_cases_incomplete << " threw an unexpected exception." << std::endl;
    }

    return results.n_cases_executed - results.n_cases_passed;
}

static int run_mode_list_suites(const ProgramArgs& args) {
    auto suites = process_suites();
    for (const TestSuite* suite: suites) {
        std::cout << suite->name << std::endl;
    }
    return 0;
}

int litetest_main(int argc, char* argv[]) {
    ProgramArgs args(argc, argv);

    try {
        switch (args.exec_mode()) {
            case ExecutionMode::LIST_SUITES:
                return run_mode_list_suites(args);
            case ExecutionMode::NORMAL:
                return run_mode_normal(args);
            default:
                throw std::invalid_argument("Unknown execution mode.");
        }
    }
    catch (const std::exception& err) {
        std::cerr << "Fatal:\n" << err.what() << std::endl;
        return EXIT_FAILURE;
    }
}

} // litetest