#ifndef ILLUMINA_INTERNAL_H
#define ILLUMINA_INTERNAL_H

#include <atomic>
#include <vector>
#include <sstream>
#include <string>
#include <functional>
#include <stdexcept>
#include <optional>
#include <thread>

namespace litetest::internal {

struct TestCase {
    std::string name;
    std::function<void()> function;
    std::string src_file;
    int line;
};

struct TestSuite {
    std::string name;
    std::string src_file;
    int line;
    std::function<void()> setup   = [](){};
    std::function<void()> cleanup = [](){};
    std::vector<TestCase*> cases;
};

struct TestFailure : public std::runtime_error {
    std::string test_case;
    std::string test_suite;
    int line;

    inline TestFailure(const std::string& message,
                       const std::string& test_case,
                       const std::string& test_suite,
                       int line = 0)
        : std::runtime_error(message), line(line),
          test_case(test_case), test_suite(test_suite) {}
};

TestCase* push_case(const std::string&, std::function<void()>, const std::string&, int);

TestSuite* push_suite(const std::string&, const std::string&, int);

std::function<void()> push_suite_setup(const std::string&, const std::string&, std::function<void()>);

std::function<void()> push_suite_cleanup(const std::string&, const std::string&, std::function<void()>);

std::vector<TestSuite*> process_suites();

const TestCase& current_case(std::thread::id = std::this_thread::get_id());

const TestSuite& current_suite(std::thread::id = std::this_thread::get_id());

extern std::atomic_int g_assert_count;

template<typename T>
class ExpectValue {
public:
    const ExpectValue& to_be(const T& other) const {
        g_assert_count++;
        if (m_val == other) {
            return *this;
        }

        std::stringstream ss;
        ss << "Expected " << other << ", got " << m_val;
        throw TestFailure(ss.str(), current_case().name, current_suite().name, m_line);
    }

    const ExpectValue& to_not_be(const T& other) const {
        g_assert_count++;
        if (m_val != other) {
            return *this;
        }

        std::stringstream ss;
        ss << "Expected " << m_val << " to be different";
        throw TestFailure(ss.str(), current_case().name, current_suite().name, m_line);

    }

    const ExpectValue& to_be_greater_than(const T& other) const {
        g_assert_count++;
        if (m_val > other) {
            return *this;
        }

        std::stringstream ss;
        ss << "Expected value to be greater than " << other << ", got " << m_val;
        throw TestFailure(ss.str(), current_case().name, current_suite().name, m_line);
    }

    const ExpectValue& to_be_less_than(const T& other) const {
        g_assert_count++;
        if (m_val < other) {
            return *this;
        }

        std::stringstream ss;
        ss << "Expected value to be less than " << other << ", got " << m_val;
        throw TestFailure(ss.str(), current_case().name, current_suite().name, m_line);
    }

    const ExpectValue& to_be_greater_than_or_equal_to(const T& other) const {
        g_assert_count++;
        if (m_val >= other) {
            return *this;
        }

        std::stringstream ss;
        ss << "Expected value to be greater than or equal to " << other << ", got " << m_val;
        throw TestFailure(ss.str(), current_case().name, current_suite().name, m_line);
    }

    const ExpectValue& to_be_less_than_or_equal_to(const T& other) const {
        g_assert_count++;
        if (m_val <= other) {
            return *this;
        }

        std::stringstream ss;
        ss << "Expected value to be less than or equal to " << other << ", got " << m_val;
        throw TestFailure(ss.str(), current_case().name, current_suite().name, m_line);
    }

    ExpectValue(const T& val, int line)
        : m_val(val), m_line(line) {}

    ExpectValue(T&& val, int line)
        : m_val(std::move(val)), m_line(line) {}

private:
    T m_val;
    int m_line;
};

}

#endif // ILLUMINA_INTERNAL_H
