cmake_minimum_required(VERSION 3.20)

find_package(GTest REQUIRED)

add_executable(test_ecdsa unit/test_ecdsa.cpp)
target_link_libraries(test_ecdsa PRIVATE sovereign_engine GTest::GTest GTest::Main)
target_compile_features(test_ecdsa PRIVATE cxx_std_20)

add_executable(test_ml_dsa unit/test_ml_dsa.cpp)
target_link_libraries(test_ml_dsa PRIVATE sovereign_engine GTest::GTest GTest::Main)
target_compile_features(test_ml_dsa PRIVATE cxx_std_20)

add_executable(test_integration integration/test_integration.cpp)
target_link_libraries(test_integration PRIVATE sovereign_engine GTest::GTest GTest::Main)
target_compile_features(test_integration PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(test_ecdsa)
gtest_discover_tests(test_ml_dsa)
gtest_discover_tests(test_integration)
