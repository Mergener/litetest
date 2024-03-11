cmake_minimum_required(VERSION 3.10)

project(litetest)

add_library(litetest STATIC litetest/litetest.cpp litetest/litetest.h main.cpp suites/types.cpp)