#ifndef LITETEST_STRINGIFY_H
#define LITETEST_STRINGIFY_H

#include <sstream>
#include <string>
#include <cstdint>

namespace litetest {

template <typename T>
inline std::string stringify(const T& val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
}

template <>
inline std::string stringify(const unsigned char& val) {
    std::stringstream ss;

    if (val != '\0') {
        ss << val << " (" << int(val) << ")";
    }
    else {
        ss << " (" << int(val) << ")";
    }
    return ss.str();
}

template <>
inline std::string stringify(const char& val) {
    return stringify<unsigned char>(val);
}

template <>
inline std::string stringify(const uint64_t & val) {
    std::stringstream ss;

    ss << "0x" << std::hex << val;
    return ss.str();
}

template <>
inline std::string stringify(const int64_t& val) {
    std::stringstream ss;

    if (val > 0) {
        ss << "0x" << std::hex << val;
    }
    else {
        ss << "-0x" << std::hex << std::abs(val);
    }
    return ss.str();
}

} // litetest

#endif // LITETEST_STRINGIFY_H
