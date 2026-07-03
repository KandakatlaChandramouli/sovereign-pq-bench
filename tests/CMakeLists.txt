cmake_minimum_required(VERSION 3.20)
find_package(GTest REQUIRED)

add_executable(test_all unit/test_all.cpp)
target_link_libraries(test_all PRIVATE sovereign_engine GTest::GTest GTest::Main)
target_compile_features(test_all PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(test_all)
