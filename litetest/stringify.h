#ifndef LITETEST_STRINGIFY_H
#define LITETEST_STRINGIFY_H

#include <sstream>
#include <string>

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

} // litetest

#endif // LITETEST_STRINGIFY_H
